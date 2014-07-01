#ifndef MEMORY_H
#define MEMORY_H

// shared_ptr / dynamic_pointer_cast
#if EXPRESSIONS_USE_BOOST_PTR
#include <boost/shared_ptr.hpp>
#define SHARED_PTR boost::shared_ptr
#define STATIC_POINTER_CAST boost::static_pointer_cast
#elif defined(_LIBCPP_VERSION)
#include <memory>
#define SHARED_PTR std::shared_ptr
#define STATIC_POINTER_CAST std::static_pointer_cast
#elif __GNUC__ >= 4
#include <tr1/memory>
#define SHARED_PTR std::tr1::shared_ptr
#define STATIC_POINTER_CAST std::tr1::static_pointer_cast
#else
#error Expressions needs gcc 4 or later to get access to <tr1/memory> (or specify EXPRESSIONS_USE_BOOST_PTR instead)
#endif


#endif
