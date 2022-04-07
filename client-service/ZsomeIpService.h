#ifndef ZSOMEIP_ZSOMEIPSERVICE_H
#define ZSOMEIP_ZSOMEIPSERVICE_H

#include <mutex>
#include <condition_variable>

#include "zserio/IService.h"
#include "vsomeip/vsomeip.hpp"
#include "zsomeip-defs/MethodDefinition.h"
#include "zsomeip-defs/ZsomeIpApp.h"


namespace zsomeip {

/* Wrapper for a zserio service to be registered and offered via SOME/IP. */
class ZsomeIpService : public ZsomeIpApp {
public:
    ZsomeIpService(
        const std::string& appName,
        std::shared_ptr<MethodDefinition> methodDefinition,
        zserio::IService& internalService);
    void clear() override;

protected:
    void onState() override;

private:
    void internalCallback(const std::shared_ptr<vsomeip::message> &message);

    std::shared_ptr<MethodDefinition> def_;
    zserio::IService& zService_;
    std::mutex services_m_;
    bool offered_ = false;
};

/* Sends requests to zserio services via SOME/IP.  */
class ZsomeIpClient
#ifdef ZSERIO_2_4_2_SERVICE_INTERFACE
    : public ZsomeIpApp, public zserio::IService {
    public:
        ZsomeIpClient(
                const std::string& appName,
                std::shared_ptr<MethodDefinition> methodDefinition);
        void callMethod(
                zserio::StringView methodName,
                zserio::Span<const uint8_t> requestData,
                zserio::IBlobBuffer& responseData,
                void* context) override;
#else
    : public ZsomeIpApp, public zserio::IServiceClient {
    public:
        ZsomeIpClient(
            const std::string& appName,
            std::shared_ptr<MethodDefinition> methodDefinition);
        std::vector<uint8_t> callMethod(
            zserio::StringView methodName,
            const zserio::IServiceData& requestData,
            void* context) override;
#endif

protected:
    void clear() override;
    void onState() override;

private:
    void onResponse(const std::shared_ptr<vsomeip::message> &response);
    void onAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool available);

    std::shared_ptr<MethodDefinition> def_;
    std::mutex running_mutex_;
    std::condition_variable response_arrived_;
    vsomeip::return_code_e response_code_;
    std::shared_ptr<vsomeip::payload> response_payload_;
    std::mutex clients_m_;
    bool available_ = false;
};


struct ZsomeIpRuntimeError : public std::exception {
    std::string msg_;
public:
    ZsomeIpRuntimeError(vsomeip::return_code_e code)
    : msg_(std::string("zsomeip client received return code ") + std::to_string(static_cast<uint8_t>(code))) {}

    ZsomeIpRuntimeError(const std::string& description)
    : msg_(std::string("zsomeip request call was unsuccessful: ") + description) {}

    const char* what() const throw() {
        return msg_.c_str();
    }
};

} // namespace zsomeip

#endif //ZSOMEIP_ZSOMEIPSERVICE_H
