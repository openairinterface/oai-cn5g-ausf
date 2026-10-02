/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "ausf_sbi.hpp"

#include <nlohmann/json.hpp>

#include "ausf.h"
#include "ausf_config.hpp"
#include "logger.hpp"

using namespace oai::ausf::app;

extern oai::config::ausf_config ausf_cfg;

//------------------------------------------------------------------------------
ausf_sbi::ausf_sbi(
    const std::shared_ptr<ausf_event>& ev,
    const std::shared_ptr<oai::sba::sbi_http_client>& client_inst)
    : oai::sba::nf_service(ev, client_inst) {
  generate_ausf_profile();
}

//------------------------------------------------------------------------------
void ausf_sbi::generate_ausf_profile() {
  // TODO: remove hardcoded values
  ausf_nf_profile_.set_nf_instance_id(nf_instance_id);
  ausf_nf_profile_.set_nf_instance_name("OAI-AUSF");
  ausf_nf_profile_.set_nf_type("AUSF");
  ausf_nf_profile_.set_fqdn(ausf_cfg.ausf_name);
  ausf_nf_profile_.set_nf_status("REGISTERED");
  ausf_nf_profile_.set_nf_heartBeat_timer(HEART_BEAT_TIMER);
  ausf_nf_profile_.set_nf_priority(1);
  ausf_nf_profile_.set_nf_capacity(100);
  ausf_nf_profile_.add_nf_ipv4_addresses(ausf_cfg.sbi.addr4);

  oai::common::sbi::ausf_info_t ausf_info_item;
  oai::common::sbi::supi_range_info_item_t supi_ranges;
  ausf_info_item.groupid = "oai-ausf-testgroupid";
  ausf_info_item.routing_indicators.push_back("0210");
  ausf_info_item.routing_indicators.push_back("9876");
  supi_ranges.supi_range.start   = "109238210938";
  supi_ranges.supi_range.pattern = "209238210938";
  supi_ranges.supi_range.start   = "q0930j0c80283ncjf";
  ausf_info_item.supi_ranges.push_back(supi_ranges);
  ausf_nf_profile_.set_ausf_info(ausf_info_item);

  ausf_nf_profile_.display();
}

//------------------------------------------------------------------------------
std::string ausf_sbi::get_nf_instance_id() const {
  return nf_instance_id;
}

//------------------------------------------------------------------------------
bool ausf_sbi::register_to_nrf() {
  nlohmann::json json_data = {};
  ausf_nf_profile_.to_json(json_data);
  Logger::ausf_sbi().info("Sending NF registration request");
  return oai::sba::nf_service::register_to_nrf(ausf_cfg.nrf_addr, json_data);
}

//------------------------------------------------------------------------------
bool ausf_sbi::deregister_to_nrf() {
  Logger::ausf_sbi().info("Sending NF deregistration request");
  return oai::sba::nf_service::deregister_to_nrf();
}

//------------------------------------------------------------------------------
bool ausf_sbi::nrf_registration_enabled() const {
  return ausf_cfg.register_nrf;
}

//------------------------------------------------------------------------------
uint64_t ausf_sbi::nrf_registration_retry_seconds() const {
  return NRF_REGISTRATION_RETRY_TIMER;
}

//------------------------------------------------------------------------------
void ausf_sbi::on_registration_outcome(
    bool success, const oai::sba::sbi_http_response& resp) {
  if (success) {
    Logger::ausf_sbi().info(
        "NF registration procedure successful (status %d)", resp.status_code);
    start_event_nf_heartbeat(HEART_BEAT_TIMER);
    stop_nrf_registration_retry();
  } else {
    Logger::ausf_sbi().info("NF registration procedure failed, try again");
    start_nrf_registration_retry();
  }
}
