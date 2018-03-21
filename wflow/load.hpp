//
// Author: Vladimir Migashko <migashko@gmail.com>, (C) 2018
//
// Copyright: See COPYING file that comes with this distribution
//

#pragma once

#include <wflow/workflow_options.hpp>
#include <string>

namespace wflow{
  
bool load(const std::string& src, workflow_options* opt, std::string* err = nullptr);

workflow_options load(const std::string& src, std::string* err = nullptr); 

std::string dump(const workflow_options& opt);

}
