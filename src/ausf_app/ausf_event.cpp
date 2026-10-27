/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#include "ausf_event.hpp"
using namespace oai::ausf::app;

//------------------------------------------------------------------------------
bs2::connection ausf_event::subscribe_loss_of_connectivity(
    const loss_of_connectivity_sig_t::slot_type& sig) {
  return loss_of_connectivity.connect(sig);
}

//------------------------------------------------------------------------------
bs2::connection ausf_event::subscribe_ue_reachability_for_data(
    const ue_reachability_for_data_sig_t::slot_type& sig) {
  return ue_reachability_for_data.connect(sig);
}
