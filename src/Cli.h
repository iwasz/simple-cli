/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <array>
#include <gsl/gsl>
#include <optional>
#include <tuple>
#include <utility>

namespace cl {

class Completion {
};
class History {
};

/**
 * Default tokenizer. Implement a specialization to override.
 */
template <typename Tok> class Tokenizer {
public:
        template <typename Inp> std::optional<Tok> operator() (Inp const &input) const { return input; }
};

/// Default output implementation (does nothing) (like printf, or std::cout)
template <typename Tok> void output (Tok const &tok) {}

enum class Error { unrecognizedCommand };

/// Error handler
template <typename Tok> void errorHandler (Tok const & /* tok */, Error error)
{
        switch (error) {
        case Error::unrecognizedCommand:
                output<Tok> ("Unrecognized command\n");
                break;

        default:
                break;
        }
}

/**
 * Commands have form of : name -> function.
 */
template <typename Tok, typename Fn> class Cmd {
public:
        Cmd (Tok tok, Fn fn) : token{std::move (tok)}, function{std::move (fn)} {}

        bool check (Tok const &tok) const { return tok == token; }

        template <typename... Par> auto operator() (Par &&... parm) { return function (std::forward<Par> (parm)...); }

private:
        Tok token;
        Fn function;
};

/**
 * Helper function for consistency.
 */
template <typename Tok, typename Fn> constexpr auto cmd (Tok &&token, Fn &&function)
{
        return Cmd{std::forward<Tok> (token), std::forward<Fn> (function)};
}

namespace detail {
        template <typename Tok, typename Cbk, typename... Rst> bool cliRunImpl (Tok const &token, Cbk &callback, Rst &... rest)
        {
                if (callback.check (token)) {
                        callback ();
                        return true;
                }

                if constexpr (sizeof...(rest)) {
                        cliRunImpl (token, rest...);
                }

                return false;
        }
} // namespace detail

/**
 * Main command line intarface "engine".
 */
template <typename Tok, typename CbksT> class Cli {
public:
        explicit Cli (CbksT c) : callbacks{std::move (c)} {}

        template <typename Inp> void run (Inp const &input)
        {
                auto token = tokenizer (input);
                bool commandWasRun{};

                if (token) {
                        commandWasRun = std::apply ([token = *token] (auto &... callback) { return detail::cliRunImpl (token, callback...); },
                                                    callbacks);
                }

                if (token && !commandWasRun) {
                        errorHandler (*token, Error::unrecognizedCommand);
                }
        }

private:
        CbksT callbacks;
        Tokenizer<Tok> tokenizer;
};

/**
 * A helper for creating a Cli instance.
 */
template <typename Tok, typename... Cbk> constexpr auto cli (Cbk &&... cbks)
{
        return Cli<Tok, decltype (std::make_tuple (std::forward<Cbk> (cbks)...))> (std::make_tuple (std::forward<Cbk> (cbks)...));
}

} // namespace cl
