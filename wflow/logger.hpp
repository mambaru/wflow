//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2013-2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#ifndef WFLOW_DISABLE_LOG

#include <wlog/logging.hpp>

#define WFLOW_LOG_ERROR(X)    WLOG_LOG_ERROR( "WFLOW", X )
#define WFLOW_LOG_WARNING(X)  WLOG_LOG_WARNING( "WFLOW", X )
#define WFLOW_LOG_MESSAGE(X)  WLOG_LOG_MESSAGE( "WFLOW", X )
#define WFLOW_LOG_FATAL(X)    WLOG_LOG_FATAL( "WFLOW", X )
#define WFLOW_LOG_BEGIN(X)    WLOG_LOG_BEGIN( "WFLOW", X )
#define WFLOW_LOG_END(X)      WLOG_LOG_END( "WFLOW", X )
#define WFLOW_LOG_DEBUG(X)    WLOG_LOG_DEBUG( "WFLOW", X )
#define WFLOW_LOG_TRACE(X)    WLOG_LOG_TRACE( "WFLOW", X )
#define WFLOW_LOG_PROGRESS(X) WLOG_LOG_PROGRESS( "WFLOW", X )

#else

#define WFLOW_LOG_ERROR(X)    
#define WFLOW_LOG_WARNING(X)  
#define WFLOW_LOG_MESSAGE(X)  
#define WFLOW_LOG_FATAL(X)    
#define WFLOW_LOG_BEGIN(X)    
#define WFLOW_LOG_END(X)      
#define WFLOW_LOG_DEBUG(X)    
#define WFLOW_LOG_TRACE(X)    
#define WFLOW_LOG_PROGRESS(X) 

#endif
