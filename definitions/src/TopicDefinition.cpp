#include "TopicDefinition.h"

#include <iostream>
#include <utility>

#include <vsomeip/vsomeip.hpp>

namespace zsomeip
{

TopicDefinition::TopicDefinition(
        const zserio::StringView zserioTopic,
        const zsomeip::AgentDefinition &agent,
        const vsomeip::event_t eventId,
        std::set<vsomeip::eventgroup_t> eventGroups)
    : topic(zserioTopic), agent(agent), eventId(eventId), eventGroups(std::move(eventGroups)) {

    snprintf(description_, 255, "[%s] (%x.%x.%x)",
            zserio::stringViewToString(topic).c_str(), agent.serviceId, agent.instanceId, eventId);
}


std::ostream &operator<<(std::ostream &out, const TopicDefinition &def) {
    out << def.description_;
    return out;
}

}