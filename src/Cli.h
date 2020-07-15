/****************************************************************************
 *                                                                          *
 *  Author : lukasz.iwaszkiewicz@gmail.com                                  *
 *  ~~~~~~~~                                                                *
 *  License : see COPYING file for details.                                 *
 *  ~~~~~~~~~                                                               *
 ****************************************************************************/

#pragma once
#include <array>
#include <etl/queue_spsc_locked.h>
#include <gsl/gsl>
#include <optional>
#include <tuple>
#include <utility>

namespace cl {

enum class LineEnd { cr, lf, crlf };

template <typename Tok> struct Traits {
        static constexpr LineEnd outputLineEnd{LineEnd::crlf};
        static constexpr size_t maxTokenSize = 16;
        static constexpr bool echo = false;
};

/****************************************************************************/

/// Default output implementation (does nothing) (like printf, or std::cout)
template <typename Tok> void output (Tok const &tok) {}

template <typename Tok> void outputLineEnd ()
{
        if constexpr (Traits<Tok>::outputLineEnd == LineEnd::cr) {
                output<char> ('\r');
                return;
        }

        if constexpr (Traits<Tok>::outputLineEnd == LineEnd::lf) {
                output<char> ('\n');
                return;
        }

        if constexpr (Traits<Tok>::outputLineEnd == LineEnd::crlf) {
                output<const char *> ("\r\n");
                return;
        }
}

enum class Error { unrecognizedCommand };

/// Error handler
template <typename Tok> void errorHandler (Tok const & /* tok */, Error error)
{
        switch (error) {
        case Error::unrecognizedCommand:
                output<const char *> ("Unrecognized command");
                outputLineEnd<Tok> ();
                break;

        default:
                break;
        }
}

/****************************************************************************/

template <typename Str> struct Token {
        Str command;
        Str argument;
};

template <typename Str> class Tokenizer {
public:
        using Tok = Token<Str>;

        std::optional<Tok> operator() (char ch) const
        {
                if (clear) {
                        token.command.clear ();
                        token.argument.clear ();

                        clear = false;
                        argParse = false;
                }

                if (ch == '\n' || ch == '\r') {
                        if constexpr (Traits<Str>::echo) {
                                outputLineEnd<Str> ();
                        }

                        clear = true;
                        argParse = false;
                        return {token};
                }

                bool out{};

                if (ch == ' ') {
                        argParse = true;
                        out = true;
                }
                else { // Don't add space to the arg
                        if (!argParse && (token.command.size () < Traits<Str>::maxTokenSize)) {
                                token.command += ch;
                                out = true;
                        }
                        else if (argParse && (token.argument.size () < Traits<Str>::maxTokenSize)) {
                                out = true;
                                token.argument += ch;
                        }
                }
                if constexpr (Traits<Str>::echo) {
                        if (out) {
                                output<char> (ch);
                        }
                }

                return {};
        }

private:
        mutable Tok token;
        mutable bool clear{};
        mutable bool argParse{};
}; // namespace cl

/****************************************************************************/

/**
 * Commands have form of : name -> function.
 */
template <typename Str, typename Fn> class Cmd {
public:
        Cmd (Str tok, Fn fn) : command{std::move (tok)}, function{std::move (fn)} {}

        bool check (Str const &tok) const { return tok == command; }

        template <typename... Par> auto operator() (Par &&... parm)
        {
                if constexpr (std::is_invocable_v<Fn, Par...>) {
                        return function (std::forward<Par> (parm)...);
                }
                else {
                        return function ();
                }
        }

private:
        Str command;
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
                if (callback.check (token.command)) {
                        callback (token.argument);
                        return true;
                }

                if constexpr (sizeof...(rest)) {
                        return cliRunImpl (token, rest...);
                }

                return false;
        }

#ifdef PROTECTED_QUEUE
        const etl::function_fv<__disable_irq> lock{};
        const etl::function_fv<__enable_irq> unlock{};
#else
        inline void f () {}
        const etl::function_fv<f> lock{};
        const etl::function_fv<f> unlock{};
#endif
} // namespace detail

/**
 * Main command line intarface "engine".
 */
template <typename Str, typename CbksT> class Cli {
public:
        explicit Cli (CbksT c) : callbacks{std::move (c)} {}

        // Can be called from ISR
        template <typename Inp> void input (Inp const &input)
        {
                auto token = tokenizer (input);

                if (token) {
                        tokenQueue.push_from_unlocked (std::move (*token));
                }
        }

        void run ()
        {
                while (!tokenQueue.empty ()) {
                        bool commandWasRun{};
                        Token<Str> token;
                        tokenQueue.pop (token);

                        commandWasRun
                                = std::apply ([&token] (auto &... callback) { return detail::cliRunImpl (token, callback...); }, callbacks);

                        if (!token.command.empty () && !commandWasRun) {
                                errorHandler (token, Error::unrecognizedCommand);
                        }
                }
        }

private:
        CbksT callbacks;
        Tokenizer<Str> tokenizer;
        etl::queue_spsc_locked<Token<Str>, 2, etl::memory_model::MEMORY_MODEL_SMALL> tokenQueue{cl::detail::lock, cl::detail::unlock};
};

/**
 * A helper for creating a Cli instance.
 */
template <typename Tok, typename... Cbk> constexpr auto cli (Cbk &&... cbks)
{
        return Cli<Tok, decltype (std::make_tuple (std::forward<Cbk> (cbks)...))> (std::make_tuple (std::forward<Cbk> (cbks)...));
}

} // namespace cl
