/*
    The Capo Library
    Code covered by the MIT License

    Author: mutouyun (http://darkc.at)
*/

#ifndef CAPO_PROOFING_PRINTF_HPP___
#define CAPO_PROOFING_PRINTF_HPP___

#include "../type/type_name.hpp"

#include <iostream>     // std::cout, std::cerr, std::clog
#include <string>       // std::string
#include <stdexcept>    // std::invalid_argument, std::logic_error
#include <type_traits>  // std::is_convertible, std::decay
#include <utility>      // std::forward, std::move
#include <cstdint>      // intmax_t, uintmax_t
#include <cstddef>      // size_t, ptrdiff_t
#include <cwchar>       // wint_t
#include <cstdio>       // vsnprintf
#include <cstdarg>      // va_list, va_start, va_end

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || \
    defined(WIN64) || defined(_WIN64) || defined(__WIN64__)
#include <windows.h>    // ::OutputDebugStringA
#define CAPO_OS_WIN__
#endif

namespace capo {

////////////////////////////////////////////////////////////////
/// Define standard stream policies
////////////////////////////////////////////////////////////////

namespace use
{
    struct std_cout
    {
        static void out(const std::string& str)
        {
            std::cout << str;
        }
    };

    struct std_cerr
    {
        static void out(const std::string& str)
        {
#       if defined(CAPO_OS_WIN__)
            ::OutputDebugStringA(str.c_str());
#       undef CAPO_OS_WIN__
#       endif
            std::cerr << str;
        }
    };

    struct std_clog
    {
        static void out(const std::string& str)
        {
            std::clog << str;
        }
    };
}

////////////////////////////////////////////////////////////////
/// To perpare for type-safe printf
////////////////////////////////////////////////////////////////

namespace detail_printf
{
    inline void enforce(std::string&& what)
    {
        throw std::invalid_argument
        {
            "Invalid format: " + what + "."
        };
    }

    template <typename T, typename U>
    inline void enforce(void)
    {
        if (!std::is_convertible<T, U>::value)
        {
            enforce(type_name<T>() + " => " + type_name<U>());
        }
    }

    template <typename T>
    void enforce_argument(const char* fmt)
    {
        using t_t = typename std::decay<T>::type;
        enum class length_t
        {
            none, h, hh, l, ll, j, z, t, L
        }
        state = length_t::none;
        for (; *fmt; ++fmt)
        {
            switch(*fmt)
            {
            // check specifiers's length
            case 'h':
                 if (state == length_t::h) state = length_t::hh;
                 else state = length_t::h; break;
            case 'l':
                 if (state == length_t::l) state = length_t::ll;
                 else state = length_t::l; break;
            case 'j': state = length_t::j; break;
            case 'z': state = length_t::z; break;
            case 't': state = length_t::t; break;
            case 'L': state = length_t::L; break;

            // check specifiers
            case 'd': case 'i':
                switch(state)
                {
                default:
                case length_t::none: enforce<t_t, int>();       break;
                case length_t::h:    enforce<t_t, short>();     break;
                case length_t::hh:   enforce<t_t, char>();      break;
                case length_t::l:    enforce<t_t, long>();      break;
                case length_t::ll:   enforce<t_t, long long>(); break;
                case length_t::j:    enforce<t_t, intmax_t>();  break;
                case length_t::z:    enforce<t_t, size_t>();    break;
                case length_t::t:    enforce<t_t, ptrdiff_t>(); break;
                }
                return;
            case 'u': case 'o': case 'x': case 'X':
                switch(state)
                {
                default:
                case length_t::none: enforce<t_t, unsigned int>();       break;
                case length_t::h:    enforce<t_t, unsigned short>();     break;
                case length_t::hh:   enforce<t_t, unsigned char>();      break;
                case length_t::l:    enforce<t_t, unsigned long>();      break;
                case length_t::ll:   enforce<t_t, unsigned long long>(); break;
                case length_t::j:    enforce<t_t, uintmax_t>();          break;
                case length_t::z:    enforce<t_t, size_t>();             break;
                case length_t::t:    enforce<t_t, ptrdiff_t>();          break;
                }
                return;
            case 'f': case 'F': case 'e': case 'E': case 'g': case 'G': case 'a': case 'A':
                switch(state)
                {
                default:
                case length_t::none: enforce<t_t, double>();      break;
                case length_t::L:    enforce<t_t, long double>(); break;
                }
                return;
            case 'c':
                switch(state)
                {
                default:
                case length_t::none: enforce<t_t, char>();    break;
                case length_t::l:    enforce<t_t, wchar_t>(); break;
                }
                return;
            case 's':
                switch(state)
                {
                default:
                case length_t::none: enforce<t_t, const char*>();    break;
                case length_t::l:    enforce<t_t, const wchar_t*>(); break;
                }
                return;
            case 'p':
                enforce<t_t, void*>();
                return;
            case 'n':
                switch(state)
                {
                default:
                case length_t::none: enforce<t_t, int*>();       break;
                case length_t::h:    enforce<t_t, short*>();     break;
                case length_t::hh:   enforce<t_t, char*>();      break;
                case length_t::l:    enforce<t_t, long*>();      break;
                case length_t::ll:   enforce<t_t, long long*>(); break;
                case length_t::j:    enforce<t_t, intmax_t*>();  break;
                case length_t::z:    enforce<t_t, size_t*>();    break;
                case length_t::t:    enforce<t_t, ptrdiff_t*>(); break;
                }
                return;
            }
        }
        enforce("Has no specifier");
    }

    void check(const char* fmt)
    {
        for (; *fmt; ++fmt)
        {
            if (*fmt != '%' || *++fmt == '%') continue;
            enforce("Bad format");
        }
    }

    template <typename T1, typename... T>
    void check(const char* fmt, T1&& /*a1*/, T&&... args)
    {
        for (; *fmt; ++fmt)
        {
            if (*fmt != '%' || *++fmt == '%') continue;
            enforce_argument<T1>(fmt);
            return check(++fmt, args...);
        }
        enforce("Too few format specifiers");
    }

    template <class PolicyT>
    int output(const char* fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        std::string buf;
        int n = ::vsnprintf(NULL, 0, fmt, args);
        if (n <= 0) goto exit_printf;
        buf.resize(n);
        n = ::vsnprintf(const_cast<char*>(buf.data()), n + 1, fmt, args);
        if (n <= 0) goto exit_printf;
        PolicyT::out(buf);
    exit_printf:
        va_end(args);
        return n;
    }
}

////////////////////////////////////////////////////////////////
/// Print formatted data to standard stream
////////////////////////////////////////////////////////////////

template <class PolicyT = capo::use::std_cout, typename... T>
int printf(const char* fmt, T&&... args)
{
    if (!fmt) return 0;
    detail_printf::check(fmt, std::forward<T>(args)...);
    return detail_printf::output<PolicyT>(fmt, std::forward<T>(args)...);
}

} // namespace capo

#endif // CAPO_PROOFING_PRINTF_HPP___
