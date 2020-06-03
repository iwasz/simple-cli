# What this is
Very simple interactive command line interface library suited both for microcontrollers and host applications. Header only.

# A quick example
``` cpp
#include "Cli.h"

auto c = cli<std::string> (cmd ("test"s, [] { std::cout << "Test command" << std::endl; }),
                           cmd ("help"s, [] { std::cout << "Help command" << std::endl; }));

c.run ("test"s);
```

# Dependencies
Library:
* C++17 compiler.

Tests
* GSL

# How to use
1. Pick your token type (like std::string or some statically allocated string like [etl::string](https://www.etlcpp.com/string.html)).
2. Implement a specialization of the `Tokenizer` template class, or use default one which returns its argument as is.
3. Define your cli calbacks.

Requirements for a token type:
* Has to have operator==

# Tokenizer
Is for... 
Interface is...
Can be statefull, if a token is not fully assembled yet, it can return an empty `std::optional`.

# TODO
* [ ] Commad parameters.
* [ ] Figure out how to (and where) implement echo. 