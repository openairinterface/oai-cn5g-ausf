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

/*! \file ausf_app.cpp
 \brief
 \author  Tien-Thinh NGUYEN
 \company Eurecom
 \date 2021
 \email: Tien-Thinh.Nguyen@eurecom.fr
 */

#include "ausf_app.hpp"

#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/time_formatters.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <chrono>

#include "logger.hpp"
#include "ausf_client.hpp"
#include "ProblemDetails.h"

#include "conversions.hpp"
#include "sha256.hpp"
#include "UEAuthenticationCtx.h"
#include "ConfirmationDataResponse.h"
#include "AuthenticationInfo.h"

#include <pistache/endpoint.h>
#include <pistache/http.h>
#include <pistache/router.h>
#include <pistache/optional.h>

#include "authentication_algorithms_with_5gaka.hpp"
#include "comUt.hpp"

using namespace oai::ausf::app;
using namespace std::chrono;

extern ausf_app* ausf_app_inst;
ausf_client* ausf_client_inst = nullptr;
using namespace config;
extern ausf_config ausf_cfg;

//------------------------------------------------------------------------------
ausf_app::ausf_app(const std::string& config_file) {
  Logger::ausf_app().startup("Starting...");
  ausf_av_s             = {};
  uint8_t XRES_STAR[16] = {};
  Logger::ausf_app().startup("Started");
}

//------------------------------------------------------------------------------
ausf_app::~ausf_app() {
  Logger::ausf_app().debug("Delete AUSF_APP instance...");
}

//------------------------------------------------------------------------------
void ausf_app::handle_ue_authentications(
    const AuthenticationInfo& authenticationInfo, nlohmann::json& json_data,
    std::string& location, uint16_t& http_response_code) {
  // stored temporarily
  /* uint8_t XRES_STAR[16];   // store xres*
   std::string SUPI_AUSF;   // store supi
   std::string AUTH_TYPE;   // store authType
   std::string SERVING_NN;  // store serving network name
   std::string KAUSF_TMP;   // store Kausf(string)
   AUSF_AV_s ausf_av_s = {};
 */
  std::string snn =
      authenticationInfo.getServingNetworkName();  // serving network name
  std::string supi = authenticationInfo.getSupiOrSuci();  // supi

  Logger::ausf_server().info("servingNetworkName %s", snn.c_str());
  Logger::ausf_server().info("supiOrSuci %s", supi.c_str());

  // 5g he av from udm

  // UDM GET interface: get authentication related info
  std::string udmUri, Method, Response;

  // UDM POST interface: get authentication related info
  udmUri = "http://" +
           std::string(
               inet_ntoa(*((struct in_addr*) &ausf_cfg.udm_addr.ipv4_addr))) +
           ":" + std::to_string(ausf_cfg.udm_addr.port) + "/nudm-ueau/v1/" +
           supi + "/security-information/generate-auth-data";
  Logger::ausf_server().debug("POST Request:" + udmUri);
  Method = "POST";

  // form udm request body AuthInfo
  nlohmann::json AuthInfo =
      {};  // model AuthenticationInfo do not have ausfInstanceId field
  AuthInfo["servingNetworkName"] = snn;
  AuthInfo["ausfInstanceId"] =
      "400346f4-087e-40b1-a4cd-00566953999d";  // TODO: need to be generated
                                               // automatically

  if (authenticationInfo
          .resynchronizationInfoIsSet())  // set ResynchronizationInfo
  {
    ResynchronizationInfo resynInfo =
        authenticationInfo.getResynchronizationInfo();

    AuthInfo["resynchronizationInfo"]["rand"] = resynInfo.getRand();
    AuthInfo["resynchronizationInfo"]["auts"] = resynInfo.getAuts();
    Logger::ausf_server().info(
        "Received authInfo from amf with ResynchronizationInfo");
  } else {
    Logger::ausf_server().info("Received normal authInfo from amf");
  }

  ausf_client::curl_http_client(udmUri, Method, AuthInfo.dump(), Response);

  Logger::ausf_server().error("response: %s", Response.c_str());

  ProblemDetails problemDetails;
  nlohmann::json problemDetails_json = {};

  nlohmann::json response_data = {};
  try {
    response_data = nlohmann::json::parse(Response.c_str());
  } catch (nlohmann::json::exception& e) {
    Logger::ausf_server().info("Could not get Json content from UDM response");

    // TODO: error handling
    problemDetails.setCause("CONTEXT_NOT_FOUND");
    problemDetails.setStatus(404);
    problemDetails.setDetail(
        "Resource corresponding to User " + supi + " not found in UDM");
    to_json(problemDetails_json, problemDetails);

    Logger::ausf_server().error(
        "Resource corresponding to User " + supi + " not found in UDM");
    Logger::ausf_server().info("Send 404 Not_Found response to AUSF");
    // response.send(Pistache::Http::Code::Not_Found,
    // problemDetails_json.dump());
    http_response_code = 404;  // TODO: Pistache::Http::Code::Not_Found;
    json_data          = problemDetails_json;
    return;
  }

  std::string authType_udm = response_data.at("authType");  // AuthType
  Logger::ausf_server().debug("authType %s", authType_udm.c_str());
  std::string autn_udm =
      response_data["authenticationVector"].at("autn");  // autn
  Logger::ausf_server().debug("autn_udm %s", autn_udm.c_str());
  std::string avType_udm =
      response_data["authenticationVector"].at("avType");  // avType
  Logger::ausf_server().debug("avType_udm %s", avType_udm.c_str());
  std::string kausf_udm =
      response_data["authenticationVector"].at("kausf");  // kausf
  Logger::ausf_server().debug("kausf_udm %s", kausf_udm.c_str());
  std::string rand_udm =
      response_data["authenticationVector"].at("rand");  // rand
  Logger::ausf_server().debug("rand_udm %s", rand_udm.c_str());
  std::string xresStar_udm =
      response_data["authenticationVector"].at("xresStar");  // xres*
  Logger::ausf_server().debug("xres*_udm %s", xresStar_udm.c_str());

  //------------------5G HE
  // AV-----------------------------------------------------
  uint8_t autn[16]     = {0};
  uint8_t rand[16]     = {0};
  uint8_t xresStar[16] = {0};
  uint8_t kausf[32]    = {0};

  conv::hex_str_to_uint8(autn_udm.c_str(), autn);          // autn
  conv::hex_str_to_uint8(rand_udm.c_str(), rand);          // rand
  conv::hex_str_to_uint8(xresStar_udm.c_str(), xresStar);  // xres*
  conv::hex_str_to_uint8(kausf_udm.c_str(), kausf);        // kausf

  /*----------------------generating 5G AV from 5G HE
   * AV--------------------------*/
  /*  HXRES* <-- XRES*  */
  /*  KSEAF  <-- KAUSF  */
  /*  5G HE AV HXRES* XRES*，KSEAF KAUSF */
  /*  KSEAF，SEAF 5G SE AV（RAND, AUTN, HXRES*）*/
  /*  A.5, 3gpp ts33.501 */
  Logger::ausf_server().debug("==generating 5g av");

  //--------generating hxres*

  uint8_t rand_ausf[16]     = {0};
  uint8_t autn_ausf[16]     = {0};
  uint8_t xresStar_ausf[16] = {0};
  uint8_t kausf_ausf[32]    = {0};
  uint8_t hxresStar[16]     = {0};

  // getting params from udm 5G HE AV------may be simplified
  memcpy(xresStar_ausf, xresStar, 16);  // xres*
  memcpy(rand_ausf, rand, 16);          // rand
  memcpy(autn_ausf, autn, 16);          // autn
  memcpy(kausf_ausf, kausf, 32);        // kausf

  // generate_Hxres*
  Authentication_5gaka::generate_Hxres(rand_ausf, xresStar_ausf, hxresStar);
  Logger::ausf_server().debug(
      "hxresStar calculated:\n %s",
      (conv::uint8_to_hex_string(hxresStar, 16)).c_str());

  uint8_t kseaf[32] = {0};
  Authentication_5gaka::derive_kseaf(snn, kausf, kseaf);
  Logger::ausf_server().debug(
      "kseaf calculated:\n %s", (conv::uint8_to_hex_string(kseaf, 32)).c_str());

  memcpy(ausf_av_s.rand, rand_ausf, 16);  // store 5g av in ausf
  memcpy(ausf_av_s.autn, autn_ausf, 16);
  memcpy(ausf_av_s.hxresStar, hxresStar, 16);
  memcpy(ausf_av_s.kseaf, kseaf, 32);
  memcpy(XRES_STAR, xresStar, 16);                  // store xres* in ausf
  SUPI_AUSF  = authenticationInfo.getSupiOrSuci();  // store supi in ausf
  SERVING_NN = snn;                                 // store snn in ausf
  AUTH_TYPE  = authType_udm;                        // store authType in ausf
  KAUSF_TMP =
      conv::uint8_to_hex_string(kausf_ausf, 32);  // store kausf_tmp in ausf

  /*----------------ausf --> seaf-----------*/
  //---form UEAuthenticationCtx
  UEAuthenticationCtx UEAuthCtx;

  string rand_s      = conv::uint8_to_hex_string(rand_ausf, 16);
  string autn_s      = conv::uint8_to_hex_string(autn_ausf, 16);
  string hxresStar_s = conv::uint8_to_hex_string(hxresStar, 16);

  UEAuthCtx.setAuthType(authType_udm);  // authType(string)

  // links(std::map)
  std::map<std::string, LinksValueSchema> ausf_links;
  LinksValueSchema ausf_Href;
  std::string resourceURI;

  std::string authCtxId_s;
  authCtxId_s = autn_s;  // authCtxId = autn

  resourceURI =
      "http://" +
      std::string(inet_ntoa(*((struct in_addr*) &ausf_cfg.sbi.addr4))) + ":" +
      std::to_string(ausf_cfg.sbi.port) + "/nausf-auth/v1/ue-authentications/" +
      authCtxId_s + "/5g-aka-confirmation";
  ausf_Href.setHref(
      resourceURI);  //"/nausf-auth/v1/ue-authentications/640110987654321/5g-aka-confirmation"

  std::string Location = resourceURI;

  // cout << ausf_Href.getHref().c_str() << endl;
  ausf_links["5G_AKA"] = ausf_Href;
  UEAuthCtx.setLinks(ausf_links);

  //----------5gAuthData(Av5gAka):rand autn hxresStar
  Av5gAka ausf_5gAuthData;
  ausf_5gAuthData.setRand(rand_s);
  ausf_5gAuthData.setAutn(autn_s);
  ausf_5gAuthData.setHxresStar(hxresStar_s);
  UEAuthCtx.setR5gAuthData(ausf_5gAuthData);

  // nlohmann::json UEAuthCtx_json;
  to_json(json_data, UEAuthCtx);
  Logger::ausf_server().debug("auth response:\n %s", json_data.dump().c_str());
}

//------------------------------------------------------------------------------
void ausf_app::handle_ue_authentications_confirmation(
    const std::string& authCtxId, const ConfirmationData& confirmationData,
    nlohmann::json& json_data, uint16_t& http_response_code) {
  // seaf --> ausf
  ProblemDetails problemDetails;
  nlohmann::json problemDetails_json = {};
  Logger::ausf_server().debug("Handling 5g-aka-confirmation-put...");

  if (SUPI_AUSF.empty())  // no ue-authentications request ever
  {
    problemDetails.setCause("SERVING_NETWORK_NOT_AUTHORIZED");
    problemDetails.setStatus(403);
    problemDetails.setDetail("Serving Network Not Authorized");
    to_json(problemDetails_json, problemDetails);

    Logger::ausf_server().error("Serving Network Not Authorized");
    Logger::ausf_server().info("Send 403 Forbidden response to AUSF");
    http_response_code = 403;  // TODO:Pistache::Http::Code::Forbidden
    json_data          = problemDetails_json;
    // response.send(Pistache::Http::Code::Forbidden,
    // problemDetails_json.dump());
    return;
  }

  // getting params
  Logger::ausf_server().info(
      "Received authCtxId %s", authCtxId.c_str());  // authCtxId
  Logger::ausf_server().info(
      "Received res* %s", confirmationData.getResStar().c_str());

  uint8_t resStar[16] = {0};
  conv::hex_str_to_uint8(
      confirmationData.getResStar().c_str(),
      resStar);  // string->uint8, res*(uint8)

  ConfirmationDataResponse confirmResponse;
  uint8_t authCtxId_seaf[16];
  conv::hex_str_to_uint8(
      authCtxId.c_str(), authCtxId_seaf);  // authCtxId in seaf

  Logger::ausf_server().debug(
      "authCtxId in ausf: %s",
      (conv::uint8_to_hex_string(ausf_av_s.autn, 16)).c_str());

  bool is_auth_vectors_present =
      Authentication_5gaka::equal_uint8(ausf_av_s.autn, authCtxId_seaf, 16);
  if (!is_auth_vectors_present)  // AV expired
  {
    Logger::ausf_server().error(
        "Authentication failure by home network with authCtxId %s: AV expired",
        authCtxId.c_str());
    confirmResponse.setAuthResult(is_auth_vectors_present);
    KAUSF_TMP = "invalid";
  } else  // AV valid
  {
    Logger::ausf_server().info("AV is up to date, handling received res*...");

    // store Kausf
    // get stored xres* -----
    uint8_t xresStar[16] = {0};
    memcpy(xresStar, XRES_STAR, 16);  // xres* stored for 5g-aka-confirmation
    Logger::ausf_server().debug(
        "xres* in ausf: %s", (conv::uint8_to_hex_string(xresStar, 16)).c_str());
    Logger::ausf_server().debug(
        "xres in amf: %s", (conv::uint8_to_hex_string(resStar, 16)).c_str());

    bool authResult = Authentication_5gaka::equal_uint8(xresStar, resStar, 16);
    confirmResponse.setAuthResult(authResult);

    if (!authResult)  // fail
    {
      Logger::ausf_server().error(
          "Authentication failure by home network with authCtxId %s: res* != "
          "xres*",
          authCtxId.c_str());
    } else  // success
    {
      Logger::ausf_server().info("Authentication successful by home network!");

      // 4.send KSEAF to SEAF
      string kseaf_s;
      kseaf_s = conv::uint8_to_hex_string(
          ausf_av_s.kseaf, 32);  // convert uint8_t to string
      confirmResponse.setKseaf(kseaf_s);

      // 5.send supi when supi_ausf exists
      if (!SUPI_AUSF.empty()) {
        confirmResponse.setSupi(SUPI_AUSF);
      }

      // 6. send authResult to udm
      // UDM POST interface ----- send authentication result
      // info--------------------

      std::string udmUri;
      std::string Method;
      std::string Response;

      udmUri = "http://" +
               std::string(inet_ntoa(
                   *((struct in_addr*) &ausf_cfg.udm_addr.ipv4_addr))) +
               ":" + std::to_string(ausf_cfg.udm_addr.port) + "/nudm-ueau/v1/" +
               SUPI_AUSF + "/auth-events";
      cout << udmUri.c_str() << endl;
      Logger::ausf_server().debug("POST Request:" + udmUri);
      Method = "POST";

      // form udm request body
      nlohmann::json confirmResultInfo = {};
      confirmResultInfo["nfInstanceId"] =
          "400346f4-087e-40b1-a4cd-00566953999d";  // fixed, may need to change
      confirmResultInfo["success"] = true;

      time_t rawtime;
      time(&rawtime);
      char buf[32];
      strftime(buf, sizeof(buf), "%FT%TZ", gmtime(&rawtime));
      confirmResultInfo["timeStamp"] = buf;  // timestamp generated

      confirmResultInfo["authType"] = AUTH_TYPE;  // authType stored in ausf
      confirmResultInfo["servingNetworkName"] =
          SERVING_NN;  // snn stored in ausf
      confirmResultInfo["authRemovalInd"] = false;

      cout << confirmResultInfo.dump() << endl;
      ausf_client::curl_http_client(
          udmUri, Method, confirmResultInfo.dump(), Response);
    }
  }

  // nlohmann::json confirmResponse_json;
  to_json(json_data, confirmResponse);
  http_response_code = 200;  // TODO
}
