#pragma once

#include <iow/io/socket/dgram/aspect.hpp>
#include <iow/io/descriptor/holder.hpp>
#include <fas/aop.hpp>

namespace iow{ namespace io{ namespace socket{ namespace dgram{

template<typename A = fas::aspect<> >
using socket_base = ::iow::io::descriptor::holder_base< typename fas::merge_aspect< A, aspect >::type >;

template<typename A = fas::aspect<> >
using socket = ::iow::io::descriptor::holder< typename fas::merge_aspect< A, aspect>::type >;

}}}}
