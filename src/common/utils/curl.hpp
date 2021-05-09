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

/*! \file curl.hpp
 \brief
 \date 2020
 \email: contact@openairinterface.org
 */

#ifndef _CURL_H_
#define _CURL_H_

// extern "C"{
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <unistd.h>
//}

#include <curl/curl.h>
#include <nlohmann/json.hpp>

#include "bstrlib.h"
#include "logger.hpp"
#include "ausf_config.hpp"

#define CURL_TIMEOUT_MS 100L

class Curl {
 public:
  /****** curl function ********/
  static void curl_http_client(
      std::string remoteUri, std::string Method, std::string msgBody,
      std::string& Response);

 private:
};

#endif
