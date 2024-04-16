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

#include "ausf_client.hpp"

#include <curl/curl.h>
#include <pistache/http.h>

#include <nlohmann/json.hpp>
#include <stdexcept>

#include "ausf.h"
#include "ausf_sbi_helper.hpp"
#include "logger.hpp"
#include "utils.hpp"

using namespace Pistache::Http;
using namespace oai::ausf::app;
using namespace oai::config;
using namespace oai::ausf::api;
using namespace oai::common::sbi;
using json = nlohmann::json;

extern ausf_config ausf_cfg;

//------------------------------------------------------------------------------
// To read content of the response from NF
static std::size_t callback(
    const char* in, std::size_t size, std::size_t num, std::string* out) {
  const std::size_t totalBytes(size * num);
  out->append(in, totalBytes);
  return totalBytes;
}

//------------------------------------------------------------------------------
ausf_client::ausf_client() {}

//------------------------------------------------------------------------------
ausf_client::~ausf_client() {
  Logger::ausf_app().debug("Delete AUSF Client instance...");
}

//------------------------------------------------------------------------------
bool ausf_client::send_request(
    std::string remote_uri, std::string method, std::string msg_body,
    std::string& response, long& response_code) {
  Logger::ausf_app().info("Send HTTP message with body %s", msg_body.c_str());

  bool result      = false;
  uint32_t str_len = msg_body.length();
  char* body_data  = (char*) malloc(str_len + 1);
  memset(body_data, 0, str_len + 1);
  memcpy((void*) body_data, (void*) msg_body.c_str(), str_len);

  curl_global_init(CURL_GLOBAL_ALL);
  CURL* curl = curl_easy_init();

  if (curl) {
    CURLcode res               = {};
    struct curl_slist* headers = nullptr;
    if ((method.compare("POST") == 0) or (method.compare("PUT") == 0) or
        (method.compare("PATCH") == 0)) {
      std::string content_type = "Content-Type: application/json";
      headers = curl_slist_append(headers, content_type.c_str());
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }

    curl_easy_setopt(curl, CURLOPT_URL, remote_uri.c_str());
    if (method.compare("POST") == 0)
      curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
    else if (method.compare("PUT") == 0)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    else if (method.compare("DELETE") == 0)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    else if (method.compare("PATCH") == 0)
      curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PATCH");
    else
      curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, kNfDefaultCurlTimeout);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1);
    curl_easy_setopt(curl, CURLOPT_INTERFACE, ausf_cfg.sbi.if_name.c_str());

    if (ausf_cfg.http_version == 2) {
      if (Logger::should_log(spdlog::level::debug))
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
      // we use a self-signed test server, skip verification during debugging
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
      curl_easy_setopt(
          curl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_PRIOR_KNOWLEDGE);
    }

    // Response information.
    std::unique_ptr<std::string> http_header_data(new std::string());

    // Hook up data handling function.
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, http_header_data.get());

    if ((method.compare("POST") == 0) or (method.compare("PUT") == 0) or
        (method.compare("PATCH") == 0)) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, msg_body.length());
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body_data);
    }

    int num_retries = 0;
    while (num_retries < kNumberOfCurlRetries) {
      num_retries++;
      res = curl_easy_perform(curl);
      if (res != CURLE_OK) {
        // Sleep between two consecutive retries
        usleep(kBaseTimeIntervalBetweenCurlRetries * pow(2, num_retries - 1));
        Logger::ausf_app().debug("Curl retry %d ...", num_retries);
        continue;
      } else {
        break;
      }
    }

    if (res != CURLE_OK) {
      Logger::ausf_app().debug(
          "Still could not reach the destination after %d retries",
          kNumberOfCurlRetries);
    } else {
      curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);
      Logger::ausf_app().debug(
          "Get response with HTTP code (%d)", response_code);
      Logger::ausf_app().debug("Response body: %s", response);

      if (response_code != 0) result = true;
    }
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
  } else {
    curl_easy_cleanup(curl);
  }

  curl_global_cleanup();
  utils::free_wrapper((void**) &body_data);

  return result;
}
