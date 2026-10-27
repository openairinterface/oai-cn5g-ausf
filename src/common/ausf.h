/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef FILE_AUSF_SEEN
#define FILE_AUSF_SEEN

#include "sbi_helper.hpp"

#define HEART_BEAT_TIMER 10

#define NRF_REGISTRATION_RETRY_TIMER 5

#define _unused(x) ((void) (x))

typedef struct {
  uint8_t rand[16];
  uint8_t autn[16];
  uint8_t hxresStar[16];
  uint8_t kseaf[32];
} AUSF_AV_s;

#define NAUSF_RG_AUTH "/rg-authentications"

#endif
