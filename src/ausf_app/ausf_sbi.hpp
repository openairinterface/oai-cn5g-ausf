/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_AUSF_SBI_SEEN
#define FILE_AUSF_SBI_SEEN

#include "ausf_event.hpp"
#include "ausf_profile.hpp"
#include "nf_service.hpp"

namespace oai::ausf::app {

class ausf_sbi : public oai::sba::nf_service {
 public:
  ausf_sbi(
      const std::shared_ptr<ausf_event>& ev,
      const std::shared_ptr<oai::sba::sbi_http_client>& client_inst);
  ausf_sbi(ausf_sbi const&)       = delete;
  ~ausf_sbi() override            = default;
  void operator=(ausf_sbi const&) = delete;

  void generate_ausf_profile();
  std::string get_nf_instance_id() const;
  bool register_to_nrf();
  bool deregister_to_nrf();

 protected:
  bool nrf_registration_enabled() const override;
  uint64_t nrf_registration_retry_seconds() const override;
  void on_registration_outcome(
      bool success, const oai::sba::sbi_http_response& resp) override;

 private:
  ausf_profile ausf_nf_profile_;
};

}  // namespace oai::ausf::app

#endif /* FILE_AUSF_SBI_SEEN */
