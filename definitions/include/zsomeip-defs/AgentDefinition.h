#ifndef ZSOMEIP_AGENTDEFINITION_H
#define ZSOMEIP_AGENTDEFINITION_H

#include "vsomeip/vsomeip.hpp"


namespace zsomeip {

struct AgentDefinition {
    /* SOME/IP agent identifier - must match configuration file! */
    const vsomeip::eventgroup_t serviceId{};
    const vsomeip::instance_t instanceId{};
};

}

#endif //ZSOMEIP_AGENTDEFINITION_H
