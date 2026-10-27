/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_AUSF_PROFILE_HPP_SEEN
#define FILE_AUSF_PROFILE_HPP_SEEN

#include <nlohmann/json.hpp>

#include "ausf.h"
#include "nf_profile.hpp"

namespace oai {
namespace ausf {
namespace app {

class ausf_profile : public oai::sba::nf_profile {
 public:
  ausf_profile();
  explicit ausf_profile(const std::string& id);
  ausf_profile(const ausf_profile& other);
  ausf_profile& operator=(const ausf_profile& other);
  ~ausf_profile() override = default;

  void set_ausf_info(const oai::common::sbi::ausf_info_t& info);
  void get_ausf_info(oai::common::sbi::ausf_info_t& info) const;

  void display() override;
  void to_json(nlohmann::json& data) const override;
  void from_json(const nlohmann::json& data);

  void handle_heartbeart_timeout(uint64_t ms);

 private:
  oai::common::sbi::ausf_info_t ausf_info;
};

}  // namespace app
}  // namespace ausf
}  // namespace oai

#endif
