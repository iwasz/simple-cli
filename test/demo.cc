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

        auto c = cli<std::string> (cmd ("test"s, [] (auto const &s) { std::cout << "Test command. Arg : [" << s << "]" << std::endl; }),
                                   cmd ("help"s, [] { std::cout << "Help command" << std::endl; }),
                                   cmd ("hello"s, [] { std::cout << "Hello world" << std::endl; }));

        c.input ('h');
        c.input ('e');
        c.input ('l');
        c.input ('p');
        c.input ('\n');
        c.run ();

        c.input ('t');
        c.input ('e');
        c.input ('s');
        c.input ('t');
        c.input ('\n');
        c.run ();

        c.input ('t');
        c.input ('e');
        c.input ('s');
        c.input ('t');
        c.input (' ');
        c.input ('1');
        c.input ('2');
        c.input ('3');
        c.input ('\n');
        c.run ();

        int ch{};
        while ((ch = std::getchar ()) != EOF) {
                c.input (char (ch));
                c.run ();
        }
}
