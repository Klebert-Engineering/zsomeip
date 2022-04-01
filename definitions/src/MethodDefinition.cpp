#include "MethodDefinition.h"

namespace zsomeip {

MethodDefinition::MethodDefinition(
        const zserio::StringView zserioMethod,
        const zsomeip::AgentDefinition &agent,
        const vsomeip::method_t someIpMethod,
        const bool reliable)
    : zserioMethod(zserioMethod), agent(agent), someIpMethod(someIpMethod), reliable(reliable) {

    char proto_description[255] = {0};
    snprintf(proto_description, 255, "[%s] (%x.%x.%x)",
            zserio::stringViewToString(zserioMethod).c_str(), agent.serviceId, agent.instanceId, someIpMethod);
    description_ = std::string(proto_description);

}

std::ostream &operator<<(std::ostream &out, const MethodDefinition &def) {
    out << def.description_;
    return out;
}

}  // namespace zsomeip
