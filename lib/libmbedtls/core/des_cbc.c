// SPDX-License-Identifier: BSD-2-Clause
/*
 * Copyright (C) 2019, Linaro Limited
 */

#include <assert.h>
#include <compiler.h>
#include <crypto/crypto.h>
#include <crypto/crypto_impl.h>
#include <mbedtls/des.h>
#include <stdlib.h>
#include <string.h>
#include <tee_api_types.h>
#include <utee_defines.h>
#include <util.h>

struct mbed_des_cbc_ctx {
	struct crypto_cipher_ctx ctx;
	int mbed_mode;
	mbedtls_des_context des_ctx;
	unsigned char iv[TEE_DES_BLOCK_SIZE];
};

static const struct crypto_cipher_ops mbed_des_cbc_ops;

static struct mbed_des_cbc_ctx *to_des_cbc_ctx(struct crypto_cipher_ctx *ctx)
{
	assert(ctx && ctx->ops == &mbed_des_cbc_ops);

	return container_of(ctx, struct mbed_des_cbc_ctx, ctx);
}

static TEE_Result mbed_des_cbc_init(struct crypto_cipher_ctx *ctx,
				    TEE_OperationMode mode, const uint8_t *key1,
				    size_t key1_len,
				    const uint8_t *key2 __unused,
				    size_t key2_len __unused,
				    const uint8_t *iv __unused,
				    size_t iv_len  __unused)
{
	struct mbed_des_cbc_ctx *c = to_des_cbc_ctx(ctx);
	int mbed_res = 0;

	if (key1_len != MBEDTLS_DES_KEY_SIZE)
		return TEE_ERROR_BAD_PARAMETERS;
	if (iv_len != sizeof(c->iv))
		return TEE_ERROR_BAD_PARAMETERS;
	memcpy(c->iv, iv, sizeof(c->iv));

	mbedtls_des_init(&c->des_ctx);

	if (mode == TEE_MODE_ENCRYPT) {
		c->mbed_mode = MBEDTLS_DES_ENCRYPT;
		mbed_res = mbedtls_des_setkey_enc(&c->des_ctx, key1);
	} else {
		c->mbed_mode = MBEDTLS_DES_DECRYPT;
		mbed_res = mbedtls_des_setkey_dec(&c->des_ctx, key1);
	}

	if (mbed_res)
		return TEE_ERROR_BAD_STATE;

	return TEE_SUCCESS;
}

static TEE_Result mbed_des_cbc_update(struct crypto_cipher_ctx *ctx,
				      bool last_block __unused,
				      const uint8_t *data, size_t len,
				      uint8_t *dst)
{
	struct mbed_des_cbc_ctx *c = to_des_cbc_ctx(ctx);

	if (mbedtls_des_crypt_cbc(&c->des_ctx, c->mbed_mode, len, c->iv,
				  data, dst))
		return TEE_ERROR_BAD_STATE;

	return TEE_SUCCESS;
}

static void mbed_des_cbc_final(struct crypto_cipher_ctx *ctx)
{
	mbedtls_des_free(&to_des_cbc_ctx(ctx)->des_ctx);
}

static void mbed_des_cbc_free_ctx(struct crypto_cipher_ctx *ctx)
{
	free(to_des_cbc_ctx(ctx));
}

static void mbed_des_cbc_copy_state(struct crypto_cipher_ctx *dst_ctx,
				    struct crypto_cipher_ctx *src_ctx)
{
	struct mbed_des_cbc_ctx *src = to_des_cbc_ctx(src_ctx);
	struct mbed_des_cbc_ctx *dst = to_des_cbc_ctx(dst_ctx);

	memcpy(dst->iv, src->iv, sizeof(dst->iv));
	dst->mbed_mode = src->mbed_mode;
	dst->des_ctx = src->des_ctx;
}

static const struct crypto_cipher_ops mbed_des_cbc_ops = {
	.init = mbed_des_cbc_init,
	.update = mbed_des_cbc_update,
	.final = mbed_des_cbc_final,
	.free_ctx = mbed_des_cbc_free_ctx,
	.copy_state = mbed_des_cbc_copy_state,
};

TEE_Result crypto_des_cbc_alloc_ctx(struct crypto_cipher_ctx **ctx_ret)
{
	struct mbed_des_cbc_ctx *c = NULL;

	c = calloc(1, sizeof(*c));
	if (!c)
		return TEE_ERROR_OUT_OF_MEMORY;

	c->ctx.ops = &mbed_des_cbc_ops;
	*ctx_ret = &c->ctx;

	return TEE_SUCCESS;
}
