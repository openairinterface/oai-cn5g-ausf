/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this
 * file except in compliance with the License. You may obtain a copy of the
 * License at
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

/*! \file ausf_app.hpp
 \brief
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2021
 \email: Tien-Thinh.Nguyen@eurecom.fr
 */

#ifndef FILE_AUSF_APP_HPP_SEEN
#define FILE_AUSF_APP_HPP_SEEN

#include <string>
#include "AuthenticationInfo.h"
#include "UEAuthenticationCtx.h"
#include "ConfirmationData.h"
#include "ausf.h"

namespace oai {
namespace ausf {
namespace app {

using namespace oai::ausf_server::model;

// class ausf_config;
class ausf_app {
 public:
  explicit ausf_app(const std::string& config_file);
  ausf_app(ausf_app const&) = delete;
  void operator=(ausf_app const&) = delete;

  virtual ~ausf_app();

  void handle_ue_authentications(
      const AuthenticationInfo& authenticationInfo, nlohmann::json& json_data,
      std::string& location, uint16_t& http_response_code);

  void handle_ue_authentications_confirmation(
      const std::string& authCtxId, const ConfirmationData& confirmation_data,
      nlohmann::json& json_data, uint16_t& http_response_code);

 private:
  AUSF_AV_s ausf_av_s;
  // stored temporarily
  uint8_t XRES_STAR[16];   // store xres*
  std::string SUPI_AUSF;   // store supi
  std::string AUTH_TYPE;   // store authType
  std::string SERVING_NN;  // store serving network name
  std::string KAUSF_TMP;   // store Kausf(string)
};
}  // namespace app
}  // namespace ausf
}  // namespace oai
#include "ausf_config.hpp"

#endif /* FILE_AUSF_APP_HPP_SEEN */
