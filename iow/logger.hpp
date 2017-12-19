//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2015, 2017
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#ifndef IOW_DISABLE_LOG

#include <wlog/logging.hpp>

#define IOW_LOG_ERROR(X)    WLOG_LOG_ERROR( "IOW", X )
#define IOW_LOG_WARNING(X)  WLOG_LOG_WARNING( "IOW", X )
#define IOW_LOG_MESSAGE(X)  WLOG_LOG_MESSAGE( "IOW", X )
#define IOW_LOG_FATAL(X)    WLOG_LOG_FATAL( "IOW", X )
#define IOW_LOG_BEGIN(X)    WLOG_LOG_BEGIN( "IOW", X )
#define IOW_LOG_END(X)      WLOG_LOG_END( "IOW", X )
#define IOW_LOG_DEBUG(X)    WLOG_LOG_DEBUG( "IOW", X )
#define IOW_LOG_TRACE(X)    WLOG_LOG_TRACE( "IOW", X )
#define IOW_LOG_PROGRESS(X) WLOG_LOG_PROGRESS( "IOW", X )

#else

#define IOW_LOG_ERROR(X)    
#define IOW_LOG_WARNING(X)  
#define IOW_LOG_MESSAGE(X)  
#define IOW_LOG_FATAL(X)    
#define IOW_LOG_BEGIN(X)    
#define IOW_LOG_END(X)      
#define IOW_LOG_DEBUG(X)    
#define IOW_LOG_TRACE(X)    
#define IOW_LOG_PROGRESS(X) 

#endif
