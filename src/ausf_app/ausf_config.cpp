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

/*! \file ausf_config.cpp
 \brief
 \author  Fengjiao He, BUPT
 \date 2021
 \email: contact@openairinterface.org
 */

#include "ausf_config.hpp"

#include <iostream>
#include <libconfig.h++>
#include "string.hpp"

#include "logger.hpp"
#include "if.hpp"

extern "C" {
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "common_defs.h"
}

using namespace libconfig;

namespace config {

//------------------------------------------------------------------------------
ausf_config::ausf_config() {
  // TODO:
}

//------------------------------------------------------------------------------
ausf_config::~ausf_config() {}

//------------------------------------------------------------------------------
int ausf_config::load(const std::string& config_file) {
  Logger::config().debug(
      "\nLoad AUSF system configuration file(%s)", config_file.c_str());
  Config cfg;
  unsigned char buf_in6_addr[sizeof(struct in6_addr)];

  try {
    cfg.readFile(config_file.c_str());
  } catch (const FileIOException& fioex) {
    Logger::config().error(
        "I/O error while reading file %s - %s", config_file.c_str(),
        fioex.what());
    throw;
  } catch (const ParseException& pex) {
    Logger::config().error(
        "Parse error at %s:%d - %s", pex.getFile(), pex.getLine(),
        pex.getError());
    throw;
  }
  const Setting& root = cfg.getRoot();

  try {
    const Setting& ausf_cfg = root[AUSF_CONFIG_STRING_AUSF_CONFIG];
  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error("%s : %s", nfex.what(), nfex.getPath());
    return -1;
  }
  const Setting& ausf_cfg = root[AUSF_CONFIG_STRING_AUSF_CONFIG];
  try {
    ausf_cfg.lookupValue(AUSF_CONFIG_STRING_INSTANCE_ID, instance);
  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }

  try {
    ausf_cfg.lookupValue(AUSF_CONFIG_STRING_PID_DIRECTORY, pid_dir);
  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }
  try {
    ausf_cfg.lookupValue(AUSF_CONFIG_STRING_AUSF_NAME, AUSF_Name);
  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
  }
  try {
    const Setting& new_if_cfg = ausf_cfg[AUSF_CONFIG_STRING_INTERFACES];

    const Setting& sbi_ausf_cfg =
        new_if_cfg[AUSF_CONFIG_STRING_INTERFACE_SBI_AUSF];
    load_interface(sbi_ausf_cfg, sbi);

    const Setting& nudm_cfg = new_if_cfg[AUSF_CONFIG_STRING_INTERFACE_NUDM];
    load_interface(nudm_cfg, nudm);

    const Setting& namf_cfg = new_if_cfg[AUSF_CONFIG_STRING_INTERFACE_NAMF];
    load_interface(namf_cfg, namf);

  } catch (const SettingNotFoundException& nfex) {
    Logger::config().error(
        "%s : %s, using defaults", nfex.what(), nfex.getPath());
    return -1;
  }
}

//------------------------------------------------------------------------------
void ausf_config::display() {
  Logger::config().info(
      "======================    AUSF   =====================");
  Logger::config().info("Configuration AUSF:");
  Logger::config().info(
      "- Instance ...........................................: %d", instance);
  Logger::config().info(
      "- PID dir ............................................: %s",
      pid_dir.c_str());
  Logger::config().info(
      "- AUSF NAME............................................: %s",
      AUSF_Name.c_str());

  Logger::config().info("- SBI Networking:");
  Logger::config().info("    iface ................: %s", sbi.if_name.c_str());
  Logger::config().info("    ip ...................: %s", inet_ntoa(sbi.addr4));
  Logger::config().info("    port .................: %d", sbi.port);

  Logger::config().info("- Nudm Networking:");
  Logger::config().info("    iface ................: %s", nudm.if_name.c_str());
  Logger::config().info(
      "    ip ...................: %s", inet_ntoa(nudm.addr4));
  Logger::config().info("    port .................: %d", nudm.port);

  Logger::config().info("- Namf Networking:");
  Logger::config().info("    iface ................: %s", namf.if_name.c_str());
  Logger::config().info(
      "    ip ...................: %s", inet_ntoa(namf.addr4));
  Logger::config().info("    port .................: %d", namf.port);
}

//------------------------------------------------------------------------------
int ausf_config::load_interface(
    const libconfig::Setting& if_cfg, interface_cfg_t& cfg) {
  if_cfg.lookupValue(AUSF_CONFIG_STRING_INTERFACE_NAME, cfg.if_name);
  util::trim(cfg.if_name);
  if (not boost::iequals(cfg.if_name, "none")) {
    std::string address = {};
    if_cfg.lookupValue(AUSF_CONFIG_STRING_IPV4_ADDRESS, address);
    util::trim(address);
    if (boost::iequals(address, "read")) {
      if (get_inet_addr_infos_from_iface(
              cfg.if_name, cfg.addr4, cfg.network4, cfg.mtu)) {
        Logger::config().error(
            "Could not read %s network interface configuration", cfg.if_name);
        return RETURNerror;
      }
    } else {
      std::vector<std::string> words;
      boost::split(
          words, address, boost::is_any_of("/"), boost::token_compress_on);
      if (words.size() != 2) {
        Logger::config().error(
            "Bad value " AUSF_CONFIG_STRING_IPV4_ADDRESS " = %s in config file",
            address.c_str());
        return RETURNerror;
      }
      unsigned char buf_in_addr[sizeof(struct in6_addr)];  // you never know...
      if (inet_pton(AF_INET, util::trim(words.at(0)).c_str(), buf_in_addr) ==
          1) {
        memcpy(&cfg.addr4, buf_in_addr, sizeof(struct in_addr));
      } else {
        Logger::config().error(
            "In conversion: Bad value " AUSF_CONFIG_STRING_IPV4_ADDRESS
            " = %s in config file",
            util::trim(words.at(0)).c_str());
        return RETURNerror;
      }
      cfg.network4.s_addr = htons(
          ntohs(cfg.addr4.s_addr) &
          0xFFFFFFFF << (32 - std::stoi(util::trim(words.at(1)))));
    }
    if_cfg.lookupValue(AUSF_CONFIG_STRING_PORT, cfg.port);
  }
  return RETURNok;
}

}  // namespace config
