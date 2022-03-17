#include "MethodDefinition.h"

namespace zsomeip {

MethodDefinition::MethodDefinition(
        const zserio::StringView zserioMethod,
        const zsomeip::AgentDefinition &agent,
        const vsomeip::method_t someIpMethod)
    : zserioMethod(zserioMethod), agent(agent), someIpMethod(someIpMethod) {

    char *proto_description_ = new char[255];
    snprintf(proto_description_, 255, "[%s] (%x.%x.%x)",
            zserio::stringViewToString(zserioMethod).c_str(), agent.serviceId, agent.instanceId, someIpMethod);
    description_ = proto_description_;

}

std::ostream &operator<<(std::ostream &out, const MethodDefinition &def) {
    out << def.description_;
    return out;
}

}  // namespace zsomeip