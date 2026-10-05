#include <string>
#include <iostream>
#include "tc/misc/misc.hpp"

void Tc::Log(std::string msg)
{
#ifdef TC_DEBUG_OUTPUT
    std::cout << msg << std::endl;
#endif
}
