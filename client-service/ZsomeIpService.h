#ifndef ZSOMEIP_ZSOMEIPSERVICE_H
#define ZSOMEIP_ZSOMEIPSERVICE_H

#include <mutex>
#include <condition_variable>

#include "zserio/IService.h"
#include "vsomeip/vsomeip.hpp"
#include "zsomeip-defs/MethodDefinition.h"

namespace zsomeip
{

/* Wrapper for a zserio service to be registered and offered via SOME/IP. */
class ZsomeIpService {
public:
    ZsomeIpService(
            std::shared_ptr<vsomeip::application> someIpApp,
            std::shared_ptr<MethodDefinition> methodDefinition,
            zserio::IService& internalService);

    void start();

private:
    void internalCallback(const std::shared_ptr<vsomeip::message> &message);

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<MethodDefinition> def_;
    zserio::IService& zService_;

};

/* Sends requests to zserio services via SOME/IP.  */
class ZsomeIpPubsub : public zserio::IServiceClient {
public:
    ZsomeIpPubsub(
        std::shared_ptr<vsomeip::application> someIpApp,
        std::shared_ptr<MethodDefinition> methodDefinition);
    std::vector<uint8_t> callMethod(
        zserio::StringView methodName,
        const zserio::IServiceData& requestData,
        void* context) override;

private:
    void onResponse(const std::shared_ptr<vsomeip::message> &response);

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<MethodDefinition> def_;

    std::mutex running_mutex_;
    std::condition_variable response_arrived_;
    bool response_ok_ = true;
    std::shared_ptr<vsomeip::payload> response_payload_;
};

}

#endif //ZSOMEIP_ZSOMEIPSERVICE_H
