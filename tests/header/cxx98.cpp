/*
 * The C++98 gate. A C++ host must get the extern "C" guard: without it every
 * declaration is name-mangled and the link against libstarkbank.a fails at the
 * far end of a release, not here. Compiled as C++98 because that is the oldest
 * dialect a Windows consumer is likely to bring.
 */

#include "starkbank.h"

namespace {
    starkbank_entity *gateEntity = 0;
    const char *gateName = 0;
}

int main()
{
    gateName = starkbank_version();
    return (gateEntity != 0 || gateName == 0) ? 1 : 0;
}
