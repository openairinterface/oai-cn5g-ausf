/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "ausf_profile.hpp"

#include "logger.hpp"
#include "string.hpp"

using namespace oai::ausf::app;

//------------------------------------------------------------------------------
ausf_profile::ausf_profile() : oai::sba::nf_profile(), ausf_info() {
  nf_type = "AUSF";
}

//------------------------------------------------------------------------------
ausf_profile::ausf_profile(const std::string& id)
    : oai::sba::nf_profile(id), ausf_info() {
  nf_type = "AUSF";
}

//------------------------------------------------------------------------------
ausf_profile::ausf_profile(const ausf_profile& other)
    : oai::sba::nf_profile(), ausf_info() {
  *this = other;
}

//------------------------------------------------------------------------------
ausf_profile& ausf_profile::operator=(const ausf_profile& other) {
  if (this == &other) return *this;

  nf_instance_id   = other.nf_instance_id;
  nf_instance_name = other.nf_instance_name;
  nf_type          = other.nf_type;
  nf_status        = other.nf_status;
  heartBeat_timer  = other.heartBeat_timer;
  plmn_list        = other.plmn_list;
  snssais          = other.snssais;
  fqdn             = other.fqdn;
  ipv4_addresses   = other.ipv4_addresses;
  ipv6_addresses   = other.ipv6_addresses;
  priority         = other.priority;
  capacity         = other.capacity;
  json_data        = other.json_data;
  nf_services      = other.nf_services;
  custom_info      = other.custom_info;
  is_updated       = other.is_updated;
  ausf_info        = other.ausf_info;
  return *this;
}

//------------------------------------------------------------------------------
void ausf_profile::set_ausf_info(const oai::common::sbi::ausf_info_t& info) {
  ausf_info = info;
}

//------------------------------------------------------------------------------
void ausf_profile::get_ausf_info(oai::common::sbi::ausf_info_t& info) const {
  info = ausf_info;
}

//------------------------------------------------------------------------------
void ausf_profile::display() {
  oai::sba::nf_profile::display();

  Logger::ausf_app().debug("\tAUSF Info");
  Logger::ausf_app().debug("\t\tGroupId: %s", ausf_info.groupid);
  for (const auto& supi : ausf_info.supi_ranges) {
    Logger::ausf_app().debug(
        "\t\t SupiRanges: Start - %s, End - %s, Pattern - %s",
        supi.supi_range.start, supi.supi_range.end, supi.supi_range.pattern);
  }
  for (const auto& route_ind : ausf_info.routing_indicators) {
    Logger::ausf_app().debug("\t\t Routing Indicators: %s", route_ind);
  }
}

//------------------------------------------------------------------------------
void ausf_profile::to_json(nlohmann::json& data) const {
  oai::sba::nf_profile::to_json(data);

  // Preserve the AUSF registration payload produced before using the common
  // NF profile implementation.
  data.erase("json_data");
  data.erase("nfServices");
  if (snssais.empty()) data["sNssais"] = nlohmann::json::array();
  data["fqdn"] = fqdn;

  data["ausfInfo"]["groupId"]           = ausf_info.groupid;
  data["ausfInfo"]["supiRanges"]        = nlohmann::json::array();
  data["ausfInfo"]["routingIndicators"] = nlohmann::json::array();
  for (const auto& supi : ausf_info.supi_ranges) {
    nlohmann::json item = {};
    item["start"]       = supi.supi_range.start;
    item["end"]         = supi.supi_range.end;
    item["pattern"]     = supi.supi_range.pattern;
    data["ausfInfo"]["supiRanges"].push_back(item);
  }
  for (const auto& route_ind : ausf_info.routing_indicators) {
    data["ausfInfo"]["routingIndicators"].push_back(route_ind);
  }

  Logger::ausf_app().debug("AUSF profile to JSON:\n %s", data.dump());
}

//------------------------------------------------------------------------------
void ausf_profile::from_json(const nlohmann::json& data) {
  snssais.clear();
  ipv4_addresses.clear();
  ausf_info = {};

  if (data.find("nfInstanceId") != data.end()) {
    nf_instance_id = data["nfInstanceId"].get<std::string>();
  }
  if (data.find("nfInstanceName") != data.end()) {
    nf_instance_name = data["nfInstanceName"].get<std::string>();
  }
  if (data.find("nfType") != data.end()) {
    nf_type = data["nfType"].get<std::string>();
  }
  if (data.find("nfStatus") != data.end()) {
    nf_status = data["nfStatus"].get<std::string>();
  }
  if (data.find("heartBeatTimer") != data.end()) {
    heartBeat_timer = data["heartBeatTimer"].get<int>();
  }
  if (data.find("sNssais") != data.end()) {
    for (const auto& item : data["sNssais"]) {
      snssai_t snssai = {};
      snssai.sst      = item["sst"].get<int>();
      snssai.sd       = item["sd"].get<std::string>();
      snssais.push_back(snssai);
    }
  }
  if (data.find("fqdn") != data.end()) {
    fqdn = data["fqdn"].get<std::string>();
  }
  if (data.find("ipv4Addresses") != data.end()) {
    for (const auto& item : data["ipv4Addresses"]) {
      struct in_addr address = {};
      auto value             = item.get<std::string>();
      if (inet_pton(AF_INET, oai::utils::trim(value).c_str(), &address) != 1) {
        Logger::ausf_app().warn(
            "Address conversion: Bad value %s", oai::utils::trim(value));
        continue;
      }
      ipv4_addresses.push_back(address);
    }
  }
  if (data.find("priority") != data.end()) {
    priority = data["priority"].get<int>();
  }
  if (data.find("capacity") != data.end()) {
    capacity = data["capacity"].get<int>();
  }

  if (data.find("ausfInfo") != data.end()) {
    const auto& info = data["ausfInfo"];
    if (info.find("groupId") != info.end()) {
      ausf_info.groupid = info["groupId"].get<std::string>();
    }
    if (info.find("routingIndicators") != info.end()) {
      for (const auto& indicator : info["routingIndicators"]) {
        ausf_info.routing_indicators.push_back(indicator);
      }
    }
    if (info.find("supiRanges") != info.end()) {
      for (const auto& range : info["supiRanges"]) {
        oai::common::sbi::supi_range_info_item_t item = {};
        item.supi_range.start                         = range["start"];
        item.supi_range.end                           = range["end"];
        item.supi_range.pattern                       = range["pattern"];
        ausf_info.supi_ranges.push_back(item);
      }
    }
  }
  display();
}

//------------------------------------------------------------------------------
void ausf_profile::handle_heartbeart_timeout(uint64_t ms) {
  Logger::ausf_app().info(
      "Handle heartbeart timeout profile %s, time %d", nf_instance_id, ms);
  set_nf_status("SUSPENDED");
}
