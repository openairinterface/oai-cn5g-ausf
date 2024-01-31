/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 *file except in compliance with the License. You may obtain a copy of the
 *License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */

#include "ausf_config.hpp"

#include <arpa/inet.h>

#include "ausf.h"
#include "common_defs.h"
#include "if.hpp"
#include "logger.hpp"
#include "string.hpp"

namespace oai::config {

//------------------------------------------------------------------------------
ausf_config::ausf_config() : sbi(), ausf_name(), pid_dir(), instance() {
  udm_addr.ipv4_addr.s_addr = INADDR_ANY;
  udm_addr.port             = 8080;  // HTTP2 by default
  udm_addr.api_version      = "v1";
  use_http2                 = false;
  log_level                 = spdlog::level::debug;
  register_nrf              = false;
}

//------------------------------------------------------------------------------
ausf_config::~ausf_config() {}

//---------------------------------------------------------------------------------------------
void ausf_config::get_udm_ueau_api_root(std::string& api_root) {
  api_root = udm_addr.uri_root + NUDM_UEAU_BASE + udm_addr.api_version;
}

//---------------------------------------------------------------------------------------------
void ausf_config::get_nrf_api_root(std::string& api_root) {
  api_root = nrf_addr.uri_root + NNRF_NFM_BASE + nrf_addr.api_version;
}

}  // namespace oai::config
