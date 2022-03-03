//
// Created by laura on 28.02.22.
//

#include "MethodDefinition.h"

namespace zsomeip {

MethodDefinition::MethodDefinition(
        const zserio::StringView zserioMethod,
        const zsomeip::AgentDefinition &agent,
        const vsomeip::method_t someIpMethod)
    : zserioMethod(zserioMethod), agent(agent), someIpMethod(someIpMethod) {

    snprintf(description_, 255, "[%s] (%x.%x.%x)",
            zserio::stringViewToString(zserioMethod).c_str(), agent.serviceId, agent.instanceId, someIpMethod);
}


// TODO use strftime and delete format_string
std::ostream &operator<<(std::ostream &out, const MethodDefinition &def) {
    out << def.description_;
    return out;
}

}