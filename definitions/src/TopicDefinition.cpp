#include <iostream>
#include <utility>

#include "TopicDefinition.h"
#include <vsomeip/vsomeip.hpp>

namespace zsomeip {

TopicDefinition::TopicDefinition(
        const zserio::StringView zserioTopic,
        const zsomeip::AgentDefinition &agent,
        const vsomeip::event_t eventId,
        std::set<vsomeip::eventgroup_t> eventGroups)
    : topic(zserioTopic), agent(agent), eventId(eventId), eventGroups(std::move(eventGroups)) {

    char proto_description[255] = {0};
    snprintf(proto_description, 255, "[%s] (%x.%x.%x)",
            zserio::stringViewToString(topic).c_str(), agent.serviceId, agent.instanceId, eventId);
    description_ = std::string(proto_description);

}

std::ostream &operator<<(std::ostream &out, const TopicDefinition &def) {
    out << def.description_;
    return out;
}

}  // namespace zsomeip
