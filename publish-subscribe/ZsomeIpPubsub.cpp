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

    std::shared_ptr<TopicDefinition> def_;
    zserio::IPubsub::SubscriptionId subscriptionId_;

private:
    void internalCallback(const std::shared_ptr<vsomeip::message> &message);

    std::shared_ptr<vsomeip::application> app_;
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
    app_->request_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId, def_->eventGroups,
                        vsomeip::event_type_e::ET_EVENT, vsomeip::reliability_type_e::RT_UNKNOWN); // TODO support tcp
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

    std::shared_ptr<zsomeip::TopicDefinition> def_;

private:
    std::shared_ptr<vsomeip::application> app_;
};

ZsomeIpPubsub::ZsomeIpPublisher::ZsomeIpPublisher(
        std::shared_ptr<vsomeip::application> someIpApp, std::shared_ptr<TopicDefinition> topicDef)
    : app_(std::move(someIpApp)), def_(std::move(topicDef))
{
    app_->offer_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId, def_->eventGroups,
                      vsomeip::event_type_e::ET_EVENT, std::chrono::milliseconds::zero(),
                      false, true, nullptr, vsomeip::reliability_type_e::RT_UNKNOWN);
}

ZsomeIpPubsub::ZsomeIpPublisher::~ZsomeIpPublisher()
{
    app_->stop_offer_event(def_->agent.serviceId, def_->agent.instanceId, def_->eventId);
    app_->stop_offer_service(def_->agent.serviceId, def_->agent.instanceId);
}

void ZsomeIpPubsub::ZsomeIpPublisher::publish(const zserio::Span<const uint8_t>& data)
{
    auto payload_ = vsomeip::runtime::get()->create_payload({data.begin(), data.end()});
    app_->notify(def_->agent.serviceId, def_->agent.instanceId, def_->eventId, payload_);
}

ZsomeIpPubsub::ZsomeIpPubsub(const std::string& appName, bool useTcp)
        : ZsomeIpApp(appName), useTcp_(useTcp), idCounter_(0) {}

// no inline to because ZsomeIpSubscription uses unique_ptr
ZsomeIpPubsub::~ZsomeIpPubsub() = default;

bool ZsomeIpPubsub::addPublisher(std::shared_ptr<zsomeip::TopicDefinition> &def)
{
    std::lock_guard<std::mutex> p_lock(publishers_m_);
    if (publishers_.find(def->topic) == publishers_.end()) {

        app_->register_availability_handler(def->agent.serviceId, def->agent.instanceId,
            std::bind(&ZsomeIpPubsub::on_availability, this,
            std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
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
    if (!registered_) {
        std::cout << "Tried to publish before registering complete, skipping..." << std:: endl;
        return;
    }
    std::lock_guard<std::mutex> p_lock(publishers_m_);
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
    auto def = *(std::shared_ptr<TopicDefinition>*)topicDefinition;
    std::lock_guard<std::mutex> s_lock(subscriptions_m_);
    subscriptions_.emplace(idCounter_, std::make_unique<ZsomeIpSubscription>(
            app_, def, idCounter_, callback));
    app_->register_availability_handler(def->agent.serviceId, def->agent.instanceId,
                                        std::bind(&ZsomeIpPubsub::on_availability, this,
                                                  std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    return idCounter_++;
}

void ZsomeIpPubsub::unsubscribe(SubscriptionId id)
{
    std::lock_guard<std::mutex> s_lock(subscriptions_m_);
    subscriptions_.erase(id);
}

void ZsomeIpPubsub::clear()
{
    {
        std::lock_guard<std::mutex> s_lock(subscriptions_m_);
        // Destructors take care of unsubscribing, unregistering & co.
        subscriptions_.erase(subscriptions_.begin(), subscriptions_.end());
    }
    {
        std::lock_guard<std::mutex> p_lock(publishers_m_);
        publishers_.erase(publishers_.begin(), publishers_.end());
    }
}

void ZsomeIpPubsub::on_state(vsomeip::state_type_e _state)
{
    if (_state == vsomeip::state_type_e::ST_REGISTERED) {
        {
            std::lock_guard<std::mutex> s_lock(subscriptions_m_);
            for (auto &subscription: subscriptions_) {
                auto agent = subscription.second->def_->agent;
                app_->request_service(agent.serviceId, agent.instanceId);
            }
        }
        {
            std::lock_guard<std::mutex> p_lock(publishers_m_);
            for (auto &publisher: publishers_) {
                auto agent = publisher.second->def_->agent;
                app_->offer_service(agent.serviceId, agent.instanceId);
            }
        }
    }
}

} // namespace zsomeip
