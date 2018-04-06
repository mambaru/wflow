//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <memory>
#include <utility>

//#ifndef DEF_MAKE_UNIQUE
//#define DEF_MAKE_UNIQUE

// If user hasn't specified COMPILER_SUPPORTS_MAKE_UNIQUE then try to figure out
// based on compiler version if std::make_unique is provided.
#if !defined(COMPILER_SUPPORTS_MAKE_UNIQUE)
   #if defined(_MSC_VER)
      // std::make_unique was added in MSVC 12.0
      #if _MSC_VER >= 1800 // MSVC 12.0 (Visual Studio 2013)
         #define COMPILER_SUPPORTS_MAKE_UNIQUE
      #endif
      #elif defined(__clang__)
        // std::make_unique was added in clang 3.4, but not until Xcode 6.
        // Annoyingly, Apple makes the clang version defines match the version
        // of Xcode, not the version of clang.
        #define CLANG_VERSION (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
        #if defined(__apple_build_version__) && CLANG_VERSION >= 60000 && __cplusplus > 201103L
          #define COMPILER_SUPPORTS_MAKE_UNIQUE
        #elif !defined(__apple_build_version__) && CLANG_VERSION >= 30400 && __cplusplus > 201103L
          #define COMPILER_SUPPORTS_MAKE_UNIQUE
        #endif
      #elif defined(__GNUC__)
        // std::make_unique was added in gcc 4.9, for standards versions greater
        // than -std=c++11.
        #define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
        #if GCC_VERSION >= 40900 && __cplusplus > 201103L
          #define COMPILER_SUPPORTS_MAKE_UNIQUE
      #endif
   #endif
#endif

#ifndef COMPILER_SUPPORTS_MAKE_UNIQUE

namespace _dmu_{

    template<class T> struct _Unique_if {
        typedef ::std::unique_ptr<T> _Single_object;
    };

    template<class T> struct _Unique_if<T[]> {
        typedef ::std::unique_ptr<T[]> _Unknown_bound;
    };

    template<class T, size_t N> struct _Unique_if<T[N]> {
        typedef void _Known_bound;
    };

} 

namespace std{
    template<class T, class... Args>
        typename _dmu_::_Unique_if<T>::_Single_object
        make_unique(Args&&... args) {
            return unique_ptr<T>(new T(std::forward<Args>(args)...));
        }

    template<class T>
        typename _dmu_::_Unique_if<T>::_Unknown_bound
        make_unique(size_t n) {
            typedef typename remove_extent<T>::type U;
            return unique_ptr<T>(new U[n]());
        }

    template<class T, class... Args>
        typename _dmu_::_Unique_if<T>::_Known_bound
        make_unique(Args&&...) = delete;
}

#endif // ifndef COMPILER_SUPPORTS_MAKE_UNIQUE
//#endif // ifndef DEF_MAKE_UNIQUE

