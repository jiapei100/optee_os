/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2014-2019, Linaro Limited
 */

#ifndef ACIPHER_HELPERS_H
#define ACIPHER_HELPERS_H

#include <crypto/crypto.h>
#include <tee_api_defines.h>
#include <tee_api_types.h>
#include <tomcrypt_private.h>
#include <types_ext.h>

static inline bool bn_alloc_max(struct bignum **s)
{
	*s = crypto_bignum_allocate(_CFG_CORE_LTC_BIGNUM_MAX_BITS);

	return *s;
}

static inline TEE_Result convert_ltc_verify_status(int ltc_res, int ltc_stat)
{
	switch (ltc_res) {
	case CRYPT_OK:
		if (ltc_stat == 1)
			return TEE_SUCCESS;
		else
			return TEE_ERROR_SIGNATURE_INVALID;
	case CRYPT_INVALID_PACKET:
		return TEE_ERROR_SIGNATURE_INVALID;
	default:
		return TEE_ERROR_GENERIC;
	}
}

#ifdef CFG_CRYPTOLIB_NAME_tomcrypt
TEE_Result ecc_populate_ltc_private_key(ecc_key *ltc_key,
					struct ecc_keypair *key,
					uint32_t algo, size_t *key_size_bytes);
TEE_Result ecc_populate_ltc_public_key(ecc_key *ltc_key,
				       struct ecc_public_key *key,
				       uint32_t algo, size_t *key_size_bytes);
#endif

#endif /* ACIPHER_HELPERS_H */
