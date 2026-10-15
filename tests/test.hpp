#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "tc/misc/macros.hpp"

#define TCT_REGISTER(method) namespace { Tc::Test::TestRegistrator JOIN(TestRegistrator_, method) (Tc::Test::TestInfo { #method, &method }); }
#define TCT_ASSERT(condition) if (!(condition)) { throw std::exception("Line " STRINGIFY(__LINE__) ": '" #condition "'"); }

namespace Tc::Test
{
    typedef void (*TestMethodPtr)();

    struct TestInfo
    {
        std::string Name;
        TestMethodPtr Method;
    };

    inline std::vector<TestInfo> Tests;

    struct TestRegistrator
    {
        TestRegistrator(TestInfo info)
        {
            Tests.push_back(info);
        }
    };
}
