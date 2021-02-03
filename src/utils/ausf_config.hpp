/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
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

/*! \file ausf_config.hpp
 \brief
 \author  Fengjiao He, BUPT
 \date 2021
 \email: contact@openairinterface.org
 */

#ifndef _AUSF_CONFIG_H_
#define _AUSF_CONFIG_H_

#include "ausf_config.hpp"

#include <arpa/inet.h>
#include <libconfig.h++>
#include <netinet/in.h>
#include <sys/socket.h>
#include <mutex>
#include <vector>
#include <string>
//#include "thread_sched.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>

#define AUSF_CONFIG_STRING_AUSF_CONFIG                    "AUSF"
#define AUSF_CONFIG_STRING_PID_DIRECTORY                 "PID_DIRECTORY"
#define AUSF_CONFIG_STRING_INSTANCE_ID                   "INSTANCE_ID"
#define AUSF_CONFIG_STRING_AUSF_NAME                      "AUSF_NAME"


#define AUSF_CONFIG_STRING_INTERFACES                    "INTERFACES"
#define AUSF_CONFIG_STRING_INTERFACE_SBI_AUSF             "SBI_AUSF"
#define AUSF_CONFIG_STRING_INTERFACE_NUDM                "NUDM"
#define AUSF_CONFIG_STRING_INTERFACE_NAMF                 "NAMF"
#define AUSF_CONFIG_STRING_INTERFACE_NAME                "INTERFACE_NAME"
#define AUSF_CONFIG_STRING_IPV4_ADDRESS                  "IPV4_ADDRESS"
#define AUSF_CONFIG_STRING_PORT                          "PORT"
#define AUSF_CONFIG_STRING_PPID                          "PPID"


// #define AUSF_CONFIG_STRING_UDM_INSTANCES_POOL            "UDM_INSTANCES_POOL"
// #define AUSF_CONFIG_STRING_UDM_INSTANCE_ID               "UDM_INSTANCE_ID"
// #define AUSF_CONFIG_STRING_UDM_INSTANCE_PORT             "PORT"
// #define AUSF_CONFIG_STRING_UDM_INSTANCE_VERSION          "VERSION"
// #define AUSF_CONFIG_STRING_UDM_INSTANCE_SELECTED         "SELECTED"

// #define AUSF_CONFIG_STRING_STATISTICS_TIMER_INTERVAL     "STATISTICS_TIMER_INTERVAL"

// #define AUSF_CONFIG_STRING_GUAMI                         "GUAMI"
// #define AUSF_CONFIG_STRING_SERVED_GUAMI_LIST             "SERVED_GUAMI_LIST"
// #define AUSF_CONFIG_STRING_RegionID                      "RegionID"
// #define AUSF_CONFIG_STRING_AMFSetID                      "AMFSetID"
// #define AUSF_CONFIG_STRING_AMFPointer                    "AMFPointer"
// #define AUSF_CONFIG_STRING_RELATIVE_AMF_CAPACITY         "RELATIVE_CAPACITY"

// #define AUSF_CONFIG_STRING_TAC                           "TAC"
// #define AUSF_CONFIG_STRING_MCC                           "MCC"
// #define AUSF_CONFIG_STRING_MNC                           "MNC"
// #define AUSF_CONFIG_STRING_PLMN_SUPPORT_LIST             "PLMN_SUPPORT_LIST"

// #define AUSF_CONFIG_STRING_SLICE_SUPPORT_LIST            "SLICE_SUPPORT_LIST"
// #define AUSF_CONFIG_STRING_SST                           "SST"
// #define AUSF_CONFIG_STRING_SD                            "SD"

// #define AUSF_CONFIG_STRING_CORE_CONFIGURATION            "CORE_CONFIGURATION"
// #define AUSF_CONFIG_STRING_EMERGENCY_SUPPORT             "EMERGENCY_SUPPORT"

// #define AUSF_CONFIG_STRING_AUTHENTICATION                "AUTHENTICATION"
// #define AUSF_CONFIG_STRING_AUTH_MYSQL_SERVER             "MYSQL_server"
// #define AUSF_CONFIG_STRING_AUTH_MYSQL_USER               "MYSQL_user"
// #define AUSF_CONFIG_STRING_AUTH_MYSQL_PASS               "MYSQL_pass"
// #define AUSF_CONFIG_STRING_AUTH_MYSQL_DB                 "MYSQL_db"
// #define AUSF_CONFIG_STRING_AUTH_OPERATOR_KEY             "OPERATOR_key"
// #define AUSF_CONFIG_STRING_AUTH_RANDOM                   "RANDOM"


using namespace libconfig;

namespace config {

typedef struct interface_cfg_s {
  std::string if_name;
  struct in_addr addr4;
  struct in_addr network4;
  struct in6_addr addr6;
  unsigned int mtu;
  unsigned int port;
} interface_cfg_t;

// typedef struct slice_s {
//   std::string sST;
//   std::string sD;
// } slice_t;

// typedef struct plmn_support_item_s {
//   std::string mcc;
//   std::string mnc;
//   uint32_t tac;
//   std::vector<slice_t> slice_list;
// } plmn_item_t;

// typedef struct {
//   int id;
//   std::string ipv4;
//   std::string port;
//   std::string version;
//   bool selected;
// } udr_inst_t;

// typedef struct {
//   std::string mysql_server;
//   std::string mysql_user;
//   std::string mysql_pass;
//   std::string mysql_db;
//   std::string operator_key;
//   std::string random;
// } auth_conf;

class ausf_config {
 public:
  ausf_config();
  ~ausf_config();
  int load(const std::string &config_file);
  int load_interface(const Setting &if_cfg, interface_cfg_t &cfg);
  void display();

  unsigned int instance;
  std::string pid_dir;
  std::string AUSF_Name;

  interface_cfg_t sbi;
  interface_cfg_t nudm;
  interface_cfg_t namf;

  //unsigned int statistics_interval;
  //std::vector<plmn_item_t> plmn_list;
  //std::string is_emergency_support;
  //auth_conf auth_para;
  //std::vector<udr_inst_t> udr_pool;
};

}

#endif
