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

template <typename Tok> class Tokenizer {
public:
        std::optional<Tok> operator() (char ch) const
        {
                if (clear) {
                        current.clear ();
                        clear = false;
                }

                if (ch == '\n' || ch == '\r') {
                        if constexpr (Traits<Tok>::echo) {
                                outputLineEnd<Tok> ();
                        }

                        clear = true;
                        return {current};
                }

                if (current.size () < Traits<Tok>::maxTokenSize) {
                        current += ch;

                        if constexpr (Traits<Tok>::echo) {
                                output<char> (ch);
                        }
                }

                return {};
        }

private:
        mutable Tok current;
        mutable bool clear{};
};

/****************************************************************************/

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
                        return cliRunImpl (token, rest...);
                }

                return false;
        }

        class InterruptControl {
        public:
                void lock () { __disable_irq (); }
                void unlock () { __enable_irq (); }
        };

        InterruptControl interruptControl;

        // Create function wrappers with direct calls to the instance's member functions.
        etl::function_imv<InterruptControl, interruptControl, &InterruptControl::lock> lock;
        etl::function_imv<InterruptControl, interruptControl, &InterruptControl::unlock> unlock;

} // namespace detail

/**
 * Main command line intarface "engine".
 */
template <typename Tok, typename CbksT> class Cli {
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
                        Tok token;
                        tokenQueue.pop (token);

                        commandWasRun
                                = std::apply ([&token] (auto &... callback) { return detail::cliRunImpl (token, callback...); }, callbacks);

                        if (!token.empty () && !commandWasRun) {
                                errorHandler (token, Error::unrecognizedCommand);
                        }
                }
        }

private:
        CbksT callbacks;
        Tokenizer<Tok> tokenizer;
        etl::queue_spsc_locked<Tok, 2, etl::memory_model::MEMORY_MODEL_SMALL> tokenQueue{detail::lock, detail::unlock};
};

/**
 * A helper for creating a Cli instance.
 */
template <typename Tok, typename... Cbk> constexpr auto cli (Cbk &&... cbks)
{
        return Cli<Tok, decltype (std::make_tuple (std::forward<Cbk> (cbks)...))> (std::make_tuple (std::forward<Cbk> (cbks)...));
}

} // namespace cl
