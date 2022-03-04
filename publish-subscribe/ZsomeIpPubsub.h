#ifndef ZSERIO_PUBSUB_SOMEIP_H
#define ZSERIO_PUBSUB_SOMEIP_H

#include <map>
#include <memory>
#include <utility>
#include <thread>

#include "vsomeip/vsomeip.hpp"
#include "zserio/IPubsub.h"
#include "zsomeip-defs/TopicDefinition.h"
#include "zsomeip-defs/ZsomeIpApp.h"

namespace zsomeip
{

/**
 * Sample Zserio IPubsub implementation using SOME/IP C++ implementation.
 */
class ZsomeIpPubsub : public zserio::IPubsub, public zsomeip::ZsomeIpApp
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

protected:
    void clear() override;

private:
    bool useTcp_; // TODO UDP/TCP switch
    std::map<SubscriptionId, std::unique_ptr<ZsomeIpSubscription>> subscriptions_;
    SubscriptionId idCounter_{};
    std::map<zserio::StringView, std::unique_ptr<ZsomeIpPublisher>> publishers_;
};

} // namespace zsomeip

#endif // ZSERIO_PUBSUB_SOMEIP_H
