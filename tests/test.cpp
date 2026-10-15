#include <iostream>
#include <chrono>
#include "tc/misc.hpp"
#include "test.hpp"

namespace Tct = Tc::Test;

int main()
{
    Tc::Log("Running tests...");

    unsigned int passed = 0;
    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    for (Tct::TestInfo& test : Tct::Tests)
    {
        Tc::Log(std::format("Starting '{}'.", test.Name));

        std::chrono::time_point<std::chrono::steady_clock> testStart = std::chrono::steady_clock::now();
#ifdef TCT_CATCH_ON_FAIL
        try
#endif
        {
            test.Method();
            passed++;
            Tc::Log("Test has passed.");
        }
#ifdef TCT_CATCH_ON_FAIL
        catch (std::exception ex)
        {
            Tc::Log(std::format("Test has failed: '{}'.", ex.what()));
        }
#endif

        std::chrono::time_point<std::chrono::steady_clock> testFinish = std::chrono::steady_clock::now();

        Tc::Log(std::format("Finished in {}.", std::chrono::duration_cast<std::chrono::milliseconds>(testFinish - testStart)));
    }
    std::chrono::time_point<std::chrono::steady_clock> finish = std::chrono::steady_clock::now();
    Tc::Log(std::format("{}/{} tests passed in {}. Press 'enter' to exit the test runner.", passed, Tct::Tests.size(), std::chrono::duration_cast<std::chrono::milliseconds>(finish - start)));
    std::cin.get();

    return 0;
}