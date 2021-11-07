
#ifndef TEE_SESSION_H
#define TEE_SESSION_H

#include <tee_client_api.h>

/* TEE resources */
struct test_ctx {
	TEEC_Context ctx;
	TEEC_Session sess;
};

void prepare_tee_session(struct test_ctx *ctx, TEEC_UUID uuid);

void prepare_tee_session_parameters(struct test_ctx *ctx, TEEC_UUID uuid, TEEC_Operation op);

void terminate_tee_session(struct test_ctx *ctx);

#endif /*TEE_SESSION_H*/
