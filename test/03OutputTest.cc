/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Cli.h"
#include "catch.hpp"
#include <cstring>
#include <deque>
#include <gsl/gsl>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

using namespace std::string_literals;
using namespace cl;

/**
 * This is a hack only for purpose of those tests. It lets me
 * define supporting classes like Tokenizer many times and make them
 * distinguishable by the linker.
 */
class Tok3 : public std::string {
        using std::string::string;
};

namespace cl {
template <> void output<Tok3> (Tok3 const &tok) { std::cerr << tok << std::endl; }
} // namespace cl

/**
 * Checks if custom tokenizers work at all.
 */
TEST_CASE ("Cerr output", "[output]")
{
        // Cli<std::string, int> cli (1);

        auto c = cli<Tok3> (cmd (Tok3 ("test"), [] { std::cout << "Test command2" << std::endl; }),
                            cmd (Tok3 ("help"), [] { std::cout << "Help command2" << std::endl; }));

        c.run (Tok3 ("test"));
        c.run (Tok3 ("aaa"));
}