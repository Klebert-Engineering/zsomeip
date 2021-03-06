#ifndef ZSOMEIP_METHODDEFINITION_H
#define ZSOMEIP_METHODDEFINITION_H

#include <ostream>

#include "zserio/IService.h"
#include "AgentDefinition.h"

namespace zsomeip {

class MethodDefinition {
public:
    MethodDefinition(
            zserio::StringView zserioMethod,
            const AgentDefinition &agent,
            vsomeip::method_t someIpMethod,
            bool reliable);

    /* Zserio method identifier */
    const zserio::StringView zserioMethod;

    /* SOME/IP agent and method identifiers - must match configuration file! */
    const AgentDefinition agent;
    vsomeip::method_t someIpMethod;
    const bool reliable;

    friend std::ostream& operator<< (std::ostream &out, const MethodDefinition &def);

private:
    std::string description_;
};

} // namespace zsomeip

#endif //ZSOMEIP_METHODDEFINITION_H
