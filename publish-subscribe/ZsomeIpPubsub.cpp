#include <memory>
#include <chrono>
#include <iostream>

#include <utility>
#include <vsomeip/vsomeip.hpp>

#include "ZsomeIpPubsub.h"
#include "zserio/BitBuffer.h"
#include "zserio/BitStreamWriter.h"

namespace zsomeip
{

class ZsomeIpPubsub::ZsomeIpSubscription {
public:
    ZsomeIpSubscription(std::shared_ptr<vsomeip::application> someIpApp,
                        std::shared_ptr<TopicDefinition> topicDefinition,
                        zserio::IPubsub::SubscriptionId id,
                        std::shared_ptr<zserio::IPubsub::OnTopicCallback> callback);

    ~ZsomeIpSubscription();

    ZsomeIpSubscription(const ZsomeIpSubscription& other) = delete;
    ZsomeIpSubscription& operator=(const ZsomeIpSubscription& other) = delete;

    ZsomeIpSubscription(ZsomeIpSubscription&& other) = delete;
    ZsomeIpSubscription& operator=(ZsomeIpSubscription&& other) = delete;

private:
    void internalCallback(const std::shared_ptr<vsomeip::message> &message);

    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<TopicDefinition> def_;
    zserio::IPubsub::SubscriptionId subscriptionId_;
    std::shared_ptr<zserio::IPubsub::OnTopicCallback> callback_;
};

ZsomeIpPubsub::ZsomeIpSubscription::ZsomeIpSubscription(
         std::shared_ptr<vsomeip::application> someIpApp,
         std::shared_ptr<TopicDefinition> topicDefinition,
         zserio::IPubsub::SubscriptionId id,
         std::shared_ptr<zserio::IPubsub::OnTopicCallback> callback)
    : app_(std::move(someIpApp)), def_(std::move(topicDefinition)),
      subscriptionId_(id), callback_(std::move(callback))
{
    app_->register_message_handler(vsomeip::ANY_SERVICE, vsomeip::ANY_INSTANCE, vsomeip::ANY_EVENT,
                                   std::bind(&ZsomeIpSubscription::internalCallback, this, std::placeholders::_1));
    app_->request_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId, def_->eventGroups); // TODO support tcp
    for (auto eventGroup : def_->eventGroups) {
        app_->subscribe(def_->agent.serviceId, def_->agent.instanceId, eventGroup);
    }

    std::cout << *def_ << " created subscriber with id " << subscriptionId_ << std::endl;
}

ZsomeIpPubsub::ZsomeIpSubscription::~ZsomeIpSubscription()
{
    for (auto eventGroup : def_->eventGroups) {
        app_->unsubscribe(def_->agent.serviceId, def_->agent.instanceId, eventGroup);
    }
    app_->release_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId);
}

void ZsomeIpPubsub::ZsomeIpSubscription::internalCallback(const std::shared_ptr<vsomeip::message> &message) {
    std::cout << *def_ << " callback triggered for subscriber " << subscriptionId_ << std::endl;
    uint8_t* payload = static_cast<uint8_t*>(message->get_payload()->get_data());
    uint32_t length = message->get_payload()->get_length();
    std::vector<uint8_t> data(payload, payload + length);
    (*callback_)(zserio::StringView(def_->topic), zserio::Span<const uint8_t>(data));
}

class ZsomeIpPubsub::ZsomeIpPublisher
{
public:
    ZsomeIpPublisher(std::shared_ptr<vsomeip::application> someIpApp, std::shared_ptr<TopicDefinition> topicDefinition);

    ~ZsomeIpPublisher();

    ZsomeIpPublisher(const ZsomeIpPublisher& other) = delete;
    ZsomeIpPublisher& operator=(const ZsomeIpPublisher& other) = delete;

    ZsomeIpPublisher(ZsomeIpPublisher&& other) = delete;
    ZsomeIpPublisher& operator=(ZsomeIpPublisher&& other) = delete;

    void publish(const zserio::Span<const uint8_t>& data);
    zserio::StringView topic() const;

private:
    std::shared_ptr<vsomeip::application> app_;
    std::shared_ptr<zsomeip::TopicDefinition> def_;
};

ZsomeIpPubsub::ZsomeIpPublisher::ZsomeIpPublisher(
        std::shared_ptr<vsomeip::application> someIpApp, std::shared_ptr<TopicDefinition> topicDef)
    : app_(std::move(someIpApp)), def_(std::move(topicDef))
{
    // TODO use state handler to assure app is running instead of sleep
    std::this_thread::sleep_for(std::chrono::seconds{1});

    app_->offer_service(def_->agent.serviceId, def_->agent.instanceId);
    app_->offer_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId, def_->eventGroups);

    // TODO await state handler confirmation before returning

    std::cout << *def_ << " created publisher" << std::endl;
}

ZsomeIpPubsub::ZsomeIpPublisher::~ZsomeIpPublisher()
{
    app_->stop_offer_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId);
    app_->stop_offer_service(def_->agent.serviceId, def_->agent.instanceId);

    // TODO await state handler confirmation before returning
}

void ZsomeIpPubsub::ZsomeIpPublisher::publish(const zserio::Span<const uint8_t>& data) {
    auto payload_ = vsomeip::runtime::get()->create_payload({data.begin(), data.end()});
    app_->notify(def_->agent.serviceId, def_->agent.instanceId, def_->eventId, payload_);
}

zserio::StringView ZsomeIpPubsub::ZsomeIpPublisher::topic() const {
    return def_->topic;
}

ZsomeIpPubsub::ZsomeIpPubsub(const std::string& appName, bool useTcp)
        : ZsomeIpApp(appName), useTcp_(useTcp), idCounter_(0) {}

// no inline to because ZsomeIpSubscription uses unique_ptr
ZsomeIpPubsub::~ZsomeIpPubsub() = default;

bool ZsomeIpPubsub::addPublisher(std::shared_ptr<zsomeip::TopicDefinition> &def) {
    // TODO synchronize publisher adding/removing
    if (publishers_.find(def->topic) == publishers_.end()) {
        publishers_.emplace(def->topic, std::make_unique<ZsomeIpPublisher>(app_, def));
        return true;
    }
    else {
        std::cout << "[ERROR] Publisher already registered for "
                  << std::string(def->topic.begin(), def->topic.end()) << std::endl;
        return false;
    }
}

void ZsomeIpPubsub::publish(zserio::StringView topic, zserio::Span<const uint8_t> data, void*)
{
    if (publishers_.find(topic) == publishers_.end()) {
        std::cout << "[ERROR] No publisher available for "
                  << std::string(topic.begin(), topic.end()) << std::endl;
        return;
    }
    publishers_[topic]->publish(data);
}

ZsomeIpPubsub::SubscriptionId ZsomeIpPubsub::subscribe(
        zserio::StringView topic,
        const std::shared_ptr<OnTopicCallback>& callback,
        void* topicDefinition)
{
    // TODO use state handler to assure app is running instead of sleep
    std::this_thread::sleep_for(std::chrono::seconds{1});

    // TODO synchronize subscription adding/removing
    auto def = *(std::shared_ptr<TopicDefinition>*)topicDefinition;
    subscriptions_.emplace(idCounter_, std::make_unique<ZsomeIpSubscription>(
            app_, def, idCounter_, callback));
    return idCounter_++;
}

void ZsomeIpPubsub::unsubscribe(SubscriptionId id)
{
    subscriptions_.erase(id);
}

void ZsomeIpPubsub::clear() {
    // Destructors take care of unsubscribing, unregistering & co.
    subscriptions_.erase(subscriptions_.begin(), subscriptions_.end());
    publishers_.erase(publishers_.begin(), publishers_.end());
}

} // namespace zsomeip
