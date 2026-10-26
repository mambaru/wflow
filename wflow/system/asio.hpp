#pragma once

#pragma GCC diagnostic push

#if defined(__GNUC__) && __GNUC__ >= 5
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#pragma GCC diagnostic ignored "-Wredundant-decls"
#pragma GCC diagnostic ignored "-Wcast-align"
#pragma GCC diagnostic ignored "-Woverloaded-virtual"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wundef"
#endif

#if defined(__GNUC__) && __GNUC__ >= 7
#pragma GCC diagnostic ignored "-Wsuggest-override"
#pragma GCC diagnostic ignored "-Wunused-const-variable"
#endif

#if __GNUC__ > 11 && __GNUC__ < 13
#pragma GCC diagnostic ignored "-Wnull-dereference"
#endif

#include <boost/asio.hpp>

#pragma GCC diagnostic pop

