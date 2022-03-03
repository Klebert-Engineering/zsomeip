#ifndef ZSOMEIP_TOPICDEFINITION_H
#define ZSOMEIP_TOPICDEFINITION_H

#include "zserio/IPubsub.h"
#include "vsomeip/primitive_types.hpp"
#include "vsomeip/vsomeip.hpp"
#include "AgentDefinition.h"

namespace zsomeip {

class TopicDefinition {
public:
    TopicDefinition(zserio::StringView zserioTopic,
                    const AgentDefinition &agent,
                    vsomeip::event_t eventId,
                    std::set<vsomeip::eventgroup_t> eventGroups);

    /* Zserio topic identifier */
    const zserio::StringView topic;

    /* SOME/IP topic and event identifiers - must match configuration file! */
    const AgentDefinition agent;
    const vsomeip::event_t eventId;
    const std::set<vsomeip::eventgroup_t> eventGroups;

    friend std::ostream& operator<< (std::ostream &out, const TopicDefinition &def);

private:
    char *description_ = new char[255];
};

} // namespace zsomeip

#endif //ZSOMEIP_TOPICDEFINITION_H
