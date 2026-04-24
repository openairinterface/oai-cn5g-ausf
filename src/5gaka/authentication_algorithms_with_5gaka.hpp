/*
 * SPDX-License-Identifier: LicenseRef-CSSL-1.0
 */

#ifndef _5GAKA_H_
#define _5GAKA_H_

#include <gmp.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <string>

#define SQN_LENGTH_BITS (48)
#define SQN_LENGTH_OCTEST (SQN_LENGTH_BITS / 8)
#define IK_LENGTH_BITS (128)
#define IK_LENGTH_OCTETS (IK_LENGTH_BITS / 8)
#define CK_LENGTH_OCTETS (16)
#define AK_LENGTH_OCTETS (6)

#define RAND_LENGTH_OCTETS (16)
#define RAND_LENGTH_BITS (RAND_LENGTH_OCTETS * 8)
#define XRES_LENGTH_OCTETS (8)
#define AUTN_LENGTH_OCTETS (16)
#define KASME_LENGTH_OCTETS (32)
#define MAC_S_LENGTH (8)
#define HXRES_LENGTH_OCTETS (16)
#define XRES_STAR_LENGTH_OCTETS (16)
#define HXRES_STAR_LENGTH_OCTETS (16)

#define AMF_LENGTH_OCTETS (2)

#define KSEAF_LENGTH_OCTETS (32)
#define KAUSF_LENGTH_OCTETS (32)
#define KAMF_LENGTH_OCTETS (32)
#define KGNB_LENGTH_OCTETS (32)

#define AUTH_VECTOR_LENGTH_OCTETS 32

#define OP_LENGTH_OCTETS (16)
#define K_LENGTH_OCTETS (16)

typedef mpz_t random_t;
typedef mpz_t sqn_t;

typedef struct {
  uint8_t rand[RAND_LENGTH_OCTETS];
  uint8_t rand_new;
  uint8_t xres[XRES_LENGTH_OCTETS];
  uint8_t autn[AUTN_LENGTH_OCTETS];
  uint8_t kasme[KASME_LENGTH_OCTETS];
} auc_vector_t;

typedef struct {
  uint8_t avType;
  uint8_t rand[RAND_LENGTH_OCTETS];
  uint8_t xres[XRES_LENGTH_OCTETS];
  uint8_t xresStar[XRES_STAR_LENGTH_OCTETS];
  uint8_t autn[AUTN_LENGTH_OCTETS];
  uint8_t kausf[KAUSF_LENGTH_OCTETS];
} _5G_HE_AV_t;  // clause 6.3.6.2.5, ts33.501

typedef struct _5G_AV_s {
  uint8_t avType;
  uint8_t rand[RAND_LENGTH_OCTETS];
  uint8_t hxres[HXRES_LENGTH_OCTETS];
  uint8_t hxresStar[HXRES_STAR_LENGTH_OCTETS];
  uint8_t autn[AUTN_LENGTH_OCTETS];
  uint8_t kseaf[KSEAF_LENGTH_OCTETS];
} _5G_AV_t;

typedef struct random_state_s {
  pthread_mutex_t lock;
  gmp_randstate_t state;
} random_state_t;

typedef enum {
  NAS_ENC_ALG = 0x01,
  NAS_INT_ALG = 0x02,
  RRC_ENC_ALG = 0x03,
  RRC_INT_ALG = 0x04,
  UP_ENC_ALG  = 0x05,
  UP_INT_ALG  = 0x06
} algorithm_type_dist_t;

class Authentication_5gaka {
 public:
 public:
  /*
   * f1: Computes network authentication code MAC-A from key K, random,
  challenge RAND, sequence number SQN and authentication management field AMF.
   * @param [const uint8_t[OP_LENGTH_OCTETS]] opc
   * @param [const uint8_t[K_LENGTH_OCTETS]] k
   * @param [const uint8_t[RAND_LENGTH_OCTETS]] _rand
   * @param [const uint8_t[SQN_LENGTH_OCTETS]] sqn
   * @param [const uint8_t[AMF_LENGTH_OCTETS]] amf
   * @param [uint8_t[8]] mac_a
   * @return
   */
  static void f1(
      const uint8_t opc[OP_LENGTH_OCTETS], const uint8_t k[K_LENGTH_OCTETS],
      const uint8_t _rand[RAND_LENGTH_OCTETS],
      const uint8_t sqn[SQN_LENGTH_OCTETS],
      const uint8_t amf[AMF_LENGTH_OCTETS], uint8_t mac_a[8]);

  /*
   * f1star: Computes resynch authentication code MAC-S from key K, random
     challenge RAND, sequence number SQN and authentication management
     field AMF.
   * @param [const uint8_t[K_LENGTH_OCTETS]] kP
   * @param [const uint8_t[K_LENGTH_OCTETS]] k
   * @param [const uint8_t[RAND_LENGTH_OCTETS]] _rand
   * @param [const uint8_t[SQN_LENGTH_OCTETS]] sqn
   * @param [const uint8_t[AMF_LENGTH_OCTETS]] amf
   * @param [uint8_t[8]] mac_s
   * @return
   */
  static void f1star(
      const uint8_t kP[K_LENGTH_OCTETS], const uint8_t k[K_LENGTH_OCTETS],
      const uint8_t rand[RAND_LENGTH_OCTETS],
      const uint8_t sqn[SQN_LENGTH_OCTETS],
      const uint8_t amf[AMF_LENGTH_OCTETS], uint8_t mac_s[8]);

  /*
   * f2345: Takes key K and random challenge RAND, and returns response RES,
     confidentiality key CK, integrity key IK and anonymity key AK
   * @param [const uint8_t[OP_LENGTH_OCTETS]] opc
   * @param [const uint8_t[K_LENGTH_OCTETS]] k
   * @param [const uint8_t[RAND_LENGTH_OCTETS]] _rand
   * @param [const uint8_t[8]] res
   * @param [const uint8_t[CK_LENGTH_OCTETS]] ck
   * @param [uint8_t[IK_LENGTH_OCTETS]] ik
   * @param [uint8_t[AK_LENGTH_OCTETS]] ak
   * @return
   */
  static void f2345(
      const uint8_t opc[OP_LENGTH_OCTETS], const uint8_t k[K_LENGTH_OCTETS],
      const uint8_t _rand[RAND_LENGTH_OCTETS], uint8_t res[8],
      uint8_t ck[CK_LENGTH_OCTETS], uint8_t ik[IK_LENGTH_OCTETS],
      uint8_t ak[AK_LENGTH_OCTETS]);

  /*
   * F5star: Takes key K and random challenge RAND, and returns resynch
     anonymity key AK
   * @param [const uint8_t[K_LENGTH_OCTETS]] kP
   * @param [const uint8_t[K_LENGTH_OCTETS]] k
   * @param [const uint8_t[RAND_LENGTH_OCTETS]] rand
   * @param [const uint8_t[AK_LENGTH_OCTETS]] ak
   * @return
   */
  static void f5star(
      const uint8_t kP[K_LENGTH_OCTETS], const uint8_t k[K_LENGTH_OCTETS],
      const uint8_t rand[RAND_LENGTH_OCTETS], uint8_t ak[AK_LENGTH_OCTETS]);

 public:
  static void kdf(
      uint8_t* key, uint16_t key_len, uint8_t* s, uint16_t s_len, uint8_t* out,
      uint16_t out_len);
  static void derive_kasme(
      uint8_t ck[CK_LENGTH_OCTETS], uint8_t ik[IK_LENGTH_OCTETS],
      uint8_t plmn[3], uint8_t sqn[SQN_LENGTH_OCTETS],
      uint8_t ak[AK_LENGTH_OCTETS], uint8_t kasme[KASME_LENGTH_OCTETS]);
  static void derive_kausf(
      uint8_t ck[CK_LENGTH_OCTETS], uint8_t ik[IK_LENGTH_OCTETS],
      std::string serving_network, uint8_t sqn[SQN_LENGTH_OCTETS],
      uint8_t ak[AK_LENGTH_OCTETS], uint8_t kausf[KAUSF_LENGTH_OCTETS]);
  static void derive_kseaf(
      std::string serving_network, uint8_t kausf[KAUSF_LENGTH_OCTETS],
      uint8_t kseaf[KSEAF_LENGTH_OCTETS]);
  static void derive_kamf(
      std::string imsi, uint8_t* kseaf, uint8_t* kamf, uint16_t abba);
  static void derive_knas(
      algorithm_type_dist_t nas_alg_type, uint8_t nas_alg_id,
      uint8_t kamf[KAMF_LENGTH_OCTETS], uint8_t* knas);
  static void derive_kgnb(
      uint32_t uplinkCount, uint8_t accessType,
      uint8_t kamf[KAMF_LENGTH_OCTETS], uint8_t* kgnb);
  static void handover_ncc_derive_knh(
      uint32_t uplinkCount, uint8_t accessType,
      uint8_t kamf[KAMF_LENGTH_OCTETS], uint8_t (&kgnb)[KGNB_LENGTH_OCTETS],
      uint8_t (&knh)[KAMF_LENGTH_OCTETS], int ncc, bool is_prev_kgnb_set,
      uint8_t (&prev_kgnb)[KGNB_LENGTH_OCTETS]);
  static uint8_t* sqn_ms_derive(
      const uint8_t opc[OP_LENGTH_OCTETS], uint8_t* key, uint8_t* auts,
      uint8_t* rand);

 public:
  /*
   * ComputeOPc: Function to compute OPc from OP and K.
   * @param [const uint8_t[K_LENGTH_OCTETS]] kP
   * @param [const uint8_t[OP_LENGTH_OCTETS]] opP
   * @param [uint8_t[OP_LENGTH_OCTETS]] opcP
   * @return
   */
  static void ComputeOPc(
      const uint8_t kP[K_LENGTH_OCTETS], const uint8_t opP[OP_LENGTH_OCTETS],
      uint8_t opcP[OP_LENGTH_OCTETS]);

  // TODO
  static void generate_autn(
      const uint8_t sqn[SQN_LENGTH_OCTETS], const uint8_t ak[AK_LENGTH_OCTETS],
      const uint8_t amf[AMF_LENGTH_OCTETS], const uint8_t mac_a[8],
      uint8_t autn[RAND_LENGTH_OCTETS]);

  // TODO
  static int generate_vector(
      const uint8_t opc[OP_LENGTH_OCTETS], uint64_t imsi,
      uint8_t key[K_LENGTH_OCTETS], uint8_t plmn[3],
      uint8_t sqn[SQN_LENGTH_OCTETS], auc_vector_t* vector);

  // TODO
  static void annex_a_4_33501(
      uint8_t ck[CK_LENGTH_OCTETS], uint8_t ik[IK_LENGTH_OCTETS],
      uint8_t* input, uint8_t rand[RAND_LENGTH_OCTETS],
      std::string serving_network, uint8_t* output);

  // TODO
  static void generate_random(uint8_t* random_p, ssize_t length);

  // TODO
  static void sha256(
      unsigned char* message, int msg_len, unsigned char* output);

  // TODO
  static void generate_Hxres(
      uint8_t rand[RAND_LENGTH_OCTETS],
      uint8_t xresStar[XRES_STAR_LENGTH_OCTETS], uint8_t* hxresStar);
  // static void generate_authCtxId(uint8_t autn[RAND_LENGTH_OCTETS],
  //                                           uint8_t *authCtxId);

  // TODO
  static bool equal_uint8(uint8_t* oldVal, uint8_t* newVal, int msg_len);

 public:
  static void RijndaelKeySchedule(const uint8_t key[K_LENGTH_OCTETS]);
  static void RijndaelEncrypt(const uint8_t in[16], uint8_t out[16]);

 private:
  auc_vector_t auc_vector;
};

#endif
