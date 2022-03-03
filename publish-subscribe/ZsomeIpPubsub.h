#ifndef ZSERIO_PUBSUB_SOMEIP_H
#define ZSERIO_PUBSUB_SOMEIP_H

#include <map>
#include <memory>
#include <utility>
#include <thread>

#include "vsomeip/vsomeip.hpp"
#include "zserio/IPubsub.h"
#include "zsomeip-defs/TopicDefinition.h"

namespace zsomeip
{

/**
 * Sample Zserio IPubsub implementation using SOME/IP C++ implementation.
 */
class ZsomeIpPubsub : public zserio::IPubsub
{
public:
    class ZsomeIpSubscription;
    class ZsomeIpPublisher;

    ZsomeIpPubsub(const std::string& appName, bool useTcp);
    ~ZsomeIpPubsub();

    bool addPublisher(std::shared_ptr<TopicDefinition> &def);
    virtual void publish(zserio::StringView topic, zserio::Span<const uint8_t> data, void* context) override;
    virtual SubscriptionId subscribe(zserio::StringView topic, const std::shared_ptr<OnTopicCallback>& callback,
        void* topicDefinition) override;
    virtual void unsubscribe(SubscriptionId id) override;
    void shutdown();

private:
    std::shared_ptr<vsomeip::application> app_;
    // TODO UDP/TCP switch
    bool useTcp_;

    void runSomeIP();

    std::map<SubscriptionId, std::unique_ptr<ZsomeIpSubscription>> subscriptions_;
    SubscriptionId idCounter_{};
    std::map<zserio::StringView, std::unique_ptr<ZsomeIpPublisher>> publishers_;
    std::thread app_thread_;
};

} // namespace zsomeip

#endif // ZSERIO_PUBSUB_SOMEIP_H
