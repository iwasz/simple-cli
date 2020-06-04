/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#include "Cli.h"
#include <cstdio>
#include <gsl/gsl>
#include <iostream>
#include <string>

/*--------------------------------------------------------------------------*/

namespace cl {

template <> void output<std::string> (std::string const &tok) { std::cout << tok << std::flush; }

} // namespace cl

int main ()
{
        using namespace cl;
        using namespace std::string_literals;

        auto c = cli<std::string> (cmd ("test"s, [] { std::cout << "Test command" << std::endl; }),
                                   cmd ("help"s, [] { std::cout << "Help command" << std::endl; }),
                                   cmd ("hello"s, [] { std::cout << "Hello world" << std::endl; }));

        // c.run ('h');
        // c.run ('e');
        // c.run ('l');
        // c.run ('p');
        // c.run ('\n');

        int ch{};
        while ((ch = std::getchar ()) != EOF) {
                c.run (char (ch));
        }
}
