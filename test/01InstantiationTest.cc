// /****************************************************************************
//  *                                                                          *
//  *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
//  *  ~~~~~~~~                                                                *
//  *  License : see COPYING file for details.                                 *
//  *  ~~~~~~~~~                                                               *
//  ****************************************************************************/

#include "Cli.h"
#include "catch.hpp"
#include <cstring>
#include <deque>
#include <iostream>
#include <string>
#include <type_traits>

using namespace std::string_literals;
using namespace cl;

/**
 * This test only instantiates some bits of state machine and checks if it is even possible.
 * Does not do any REQUIRE checks.
 */
TEST_CASE ("First test", "[instantiation]")
{
        // Cli<std::string, int> cli (1);

        auto c = cli<std::string> (cmd ("test"s, [] { std::cout << "Test command" << std::endl; }),
                                   cmd ("help"s, [] { std::cout << "Help command" << std::endl; }));

        c.run ("test"s);
        c.run ("help"s);
}
