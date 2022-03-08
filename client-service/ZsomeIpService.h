#ifndef ZSOMEIP_ZSOMEIPSERVICE_H
#define ZSOMEIP_ZSOMEIPSERVICE_H

#include <mutex>
#include <condition_variable>

#include "zserio/IService.h"
#include "vsomeip/vsomeip.hpp"
#include "zsomeip-defs/MethodDefinition.h"
#include "zsomeip-defs/ZsomeIpApp.h"

namespace zsomeip
{

/* Wrapper for a zserio service to be registered and offered via SOME/IP. */
class ZsomeIpService : public ZsomeIpApp {
public:
    ZsomeIpService(
        const std::string& appName,
        std::shared_ptr<MethodDefinition> methodDefinition,
        zserio::IService& internalService);

    void offerService();
    void clear() override;
    void on_state(vsomeip::state_type_e _state) override;

private:
    void internalCallback(const std::shared_ptr<vsomeip::message> &message);

    std::shared_ptr<MethodDefinition> def_;
    zserio::IService& zService_;
    std::mutex services_m_;
};

/* Sends requests to zserio services via SOME/IP.  */
class ZsomeIpClient : public zserio::IServiceClient, public ZsomeIpApp {
public:
    ZsomeIpClient(
        const std::string& appName,
        std::shared_ptr<MethodDefinition> methodDefinition);
    std::vector<uint8_t> callMethod(
        zserio::StringView methodName,
        const zserio::IServiceData& requestData,
        void* context) override;

protected:
    void clear() override;
    void on_state(vsomeip::state_type_e _state) override;

private:
    void onResponse(const std::shared_ptr<vsomeip::message> &response);

    std::shared_ptr<MethodDefinition> def_;
    std::mutex running_mutex_;
    std::condition_variable response_arrived_;
    bool response_ok_ = true;
    std::shared_ptr<vsomeip::payload> response_payload_;
    std::mutex clients_m_;
};

}

#endif //ZSOMEIP_ZSOMEIPSERVICE_H
