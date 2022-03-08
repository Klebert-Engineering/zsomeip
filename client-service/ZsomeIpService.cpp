#include <iostream>
#include <utility>

#include "ZsomeIpService.h"
#include "zserio/BitStreamReader.h"


namespace zsomeip {

// TODO accept multiple MethodDefinitions per service/client (see pubsub)

ZsomeIpService::ZsomeIpService(
        const std::string& appName,
        std::shared_ptr<MethodDefinition> methodDefinition,
        zserio::IService& internalService)
    : ZsomeIpApp(appName), def_(std::move(methodDefinition)), zService_(internalService)
{
    std::lock_guard<std::mutex> s_lock(services_m_);
    app_->register_message_handler(
        def_->agent.serviceId,
        def_->agent.instanceId,
        def_->someIpMethod,
        std::bind(&ZsomeIpService::internalCallback, this, std::placeholders::_1));
}

void ZsomeIpService::offerService()
{
    std::lock_guard<std::mutex> s_lock(services_m_);
    app_->offer_service(def_->agent.serviceId, def_->agent.instanceId);
}

void ZsomeIpService::clear()
{
    std::lock_guard<std::mutex> s_lock(services_m_);
    app_->stop_offer_service(def_->agent.serviceId, def_->agent.instanceId);
}

void ZsomeIpService::on_state(vsomeip::state_type_e _state)
{
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
        std::lock_guard<std::mutex> s_lock(services_m_);
        app_->offer_service(def_->agent.serviceId, def_->agent.instanceId);
    }
}

void ZsomeIpService::internalCallback(const std::shared_ptr<vsomeip::message> &request)
{
    uint8_t* payload = static_cast<uint8_t*>(request->get_payload()->get_data());
    uint32_t length = request->get_payload()->get_length();
    std::vector<uint8_t> data(payload, payload + length);
    auto responseData = zService_.callMethod(zserio::StringView(def_->zserioMethod),
                          zserio::Span<const uint8_t>(data));

    std::shared_ptr<vsomeip::message> response = vsomeip::runtime::get()->create_response(request);
    std::shared_ptr<vsomeip::payload> responsePayload = vsomeip::runtime::get()->create_payload(
            {responseData->getData().begin(), responseData->getData().end()});
    response->set_payload(responsePayload);
    app_->send(response);
}

ZsomeIpClient::ZsomeIpClient(
        const std::string& appName,
        std::shared_ptr<MethodDefinition> methodDefinition)
    : ZsomeIpApp(appName), def_(std::move(methodDefinition))
{

    std::lock_guard<std::mutex> s_lock(clients_m_);
    app_->register_message_handler(
        def_->agent.serviceId, def_->agent.instanceId, def_->someIpMethod,
        std::bind(&ZsomeIpClient::onResponse, this, std::placeholders::_1));
    app_->request_service(def_->agent.serviceId, def_->agent.instanceId);
}

std::vector<uint8_t> ZsomeIpClient::callMethod(
        zserio::StringView methodName,
        const zserio::IServiceData& requestData,
        void* context)
{
    std::shared_ptr<vsomeip::message> request = vsomeip::runtime::get()->create_request();
    request->set_service(def_->agent.serviceId);
    request->set_instance(def_->agent.instanceId);
    request->set_method(def_->someIpMethod);
    std::shared_ptr<vsomeip::payload> requestPayload = vsomeip::runtime::get()->create_payload(
            {requestData.getData().begin(), requestData.getData().end()});
    request->set_payload(requestPayload);

    std::unique_lock<std::mutex> lock_until_response(running_mutex_);
    app_->send(request);
    response_arrived_.wait(lock_until_response);

    std::vector<uint8_t> responseData{};

    if (response_ok_) {
        auto* payload = static_cast<uint8_t*>(response_payload_->get_data());
        uint32_t length = response_payload_->get_length();
        responseData.resize(length);
        std::copy_n(payload, length, responseData.begin());
    }

    return responseData;
}

void ZsomeIpClient::onResponse(const std::shared_ptr<vsomeip::message> &response)
{
    if (response->get_return_code() == vsomeip::return_code_e::E_OK) {
        response_ok_ = true;
        response_payload_ = response->get_payload();
    } else {
        printf("Received return code %hhu \n", static_cast<uint8_t>(response->get_return_code()));
        response_ok_ = false;
    }
    response_arrived_.notify_one();
}

void ZsomeIpClient::clear()
{
    std::lock_guard<std::mutex> s_lock(clients_m_);
    app_->release_service(def_->agent.serviceId, def_->agent.instanceId);
}

void ZsomeIpClient::on_state(vsomeip::state_type_e _state)
{
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
        std::lock_guard<std::mutex> s_lock(clients_m_);
        app_->request_service(def_->agent.serviceId, def_->agent.instanceId);
    }
}

} // namespace zsomeip

