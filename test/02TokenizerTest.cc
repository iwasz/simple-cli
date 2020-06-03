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

/****************************************************************************/

/**
 * This is a hack only for purpose of those tests. It lets me
 * define supporting classes like Tokenizer many times and make them
 * distinguishable by the linker.
 */
class Tok1 : public std::string {
        using std::string::string;
};

std::vector<Tok1> tokens;

namespace cl {
template <> class Tokenizer<Tok1> {
public:
        template <typename Inp> std::optional<Tok1> operator() (Inp const &input) const
        {
                tokens.push_back (input);
                std::cout << "TTT : " << input << std::endl;
                return input;
        }
};
} // namespace cl

using namespace std::string_literals;
using namespace cl;

/**
 * Checks if custom tokenizers work at all.
 */
TEST_CASE ("Custom", "[tokenizer]")
{
        // Cli<std::string, int> cli (1);

        auto c = cli<Tok1> (cmd (Tok1 ("test"), [] { std::cout << "Test command2" << std::endl; }),
                            cmd (Tok1 ("help"), [] { std::cout << "Help command2" << std::endl; }));

        c.run (Tok1 ("test"));
        REQUIRE (tokens.size () == 1);
        REQUIRE (tokens.back () == "test");

        c.run (Tok1 ("help"));
        REQUIRE (tokens.size () == 2);
        REQUIRE (tokens.back () == "help");
}

/****************************************************************************/

class Tok2 : public std::string {
        using std::string::string;
};

namespace cl {
template <> class Tokenizer<Tok2> {
public:
        std::optional<Tok2> operator() (gsl::span<const char> data) const
        {
                std::copy (data.begin (), data.end (), std::back_inserter (current));

                if (data.back () == '\n') {
                        return {current};
                }

                return {};
        }

private:
        mutable Tok2 current;
};

/**
 * This test simulates a situation where new data arrives sequentially like
 * from UART or other character device.
 */
TEST_CASE ("Uart", "[tokenizer]")
{
        std::vector<std::string> result;

        auto c = cli<Tok2> (cmd (Tok2 ("test\n"), [&result] { result.emplace_back ("test"); }),
                            cmd (Tok2 ("help\n"), [&result] { result.emplace_back ("help"); }));

        gsl::czstring<> str = "test\n";
        c.run (gsl::span<const char> (str, 1)); // t
        REQUIRE (result.empty ());

        c.run (gsl::span<const char> (str + 1, 2)); // es
        REQUIRE (result.empty ());

        c.run (gsl::span<const char> (str + 3, 2)); // t\n
        REQUIRE (result.size () == 1);
        REQUIRE (result.back () == "test");

        // c.run (Tok1 ("help"));
        // REQUIRE (tokens.size () == 2);
        // REQUIRE (tokens.back () == "help");
}

} // namespace cl
