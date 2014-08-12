#pragma once

#define BOOST_UTF8_BEGIN_NAMESPACE namespace boost { namespace mylib {
#define BOOST_UTF8_END_NAMESPACE }}
#define BOOST_UTF8_DECL

#ifdef _WIN32
#pragma warning(push,3)
#pragma warning(disable : 4100)
#pragma warning(disable : 4511)
#endif

#include "utf8_codecvt_facet.hpp"

#ifdef _WIN32
#pragma warning(pop)
#endif

#undef BOOST_UTF8_DECL
#undef BOOST_UTF8_END_NAMESPACE
#undef BOOST_UTF8_BEGIN_NAMESPACE

typedef boost::mylib::utf8_codecvt_facet utf8_codecvt;

