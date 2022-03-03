#ifndef ZSOMEIP_METHODDEFINITION_H
#define ZSOMEIP_METHODDEFINITION_H

#include "zserio/IService.h"
#include "AgentDefinition.h"


// TODO rename namespace
namespace zsomeip {

class MethodDefinition {
public:
    MethodDefinition(
            zserio::StringView zserioMethod,
            const AgentDefinition &agent,
            vsomeip::method_t someIpMethod);

    /* Zserio method identifier */
    const zserio::StringView zserioMethod;

    /* SOME/IP agent and method identifiers - must match configuration file! */
    const AgentDefinition agent;
    vsomeip::method_t someIpMethod;

    friend std::ostream& operator<< (std::ostream &out, const MethodDefinition &def);

private:
    char *description_ = new char[255];
};

}

#endif //ZSOMEIP_METHODDEFINITION_H