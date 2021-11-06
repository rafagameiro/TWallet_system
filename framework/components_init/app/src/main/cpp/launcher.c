#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <launcher.h>

void prepare_tee_session(struct test_ctx *ctx, TEEC_UUID uuid)
{
    uint32_t err_origin;
    TEEC_Result res;

    /* Initialize a context connecting us to the TEE */
    res = TEEC_InitializeContext(NULL, &ctx->ctx);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_InitializeContext failed with code 0x%x", res);

    /* Opens session to the Authentication Service TA*/
    res = TEEC_OpenSession(&ctx->ctx, &ctx->sess, &uuid, TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);
    if (res != TEEC_SUCCESS)
        errx(1, "TEEC_Opensession failed with code 0x%x origin 0x%x", res, err_origin);
}

void terminate_tee_session(struct test_ctx *ctx)
{
    /* Close session*/
    TEEC_CloseSession(&ctx->sess);
    TEEC_FinalizeContext(&ctx->ctx);
}

JNIEXPORT void JNICALL
Java_com_android_twallet_initializer_service_ComponentsLauncher_launchComponentsTA(JNIEnv* env, jclass clazz) {

    struct test_ctx ctx;

    /* Adaptation Layer */
    TEEC_UUID uuid_tee_layer = TA_ADAPTATION_LAYER_UUID;
    prepare_tee_session(&ctx, uuid_tee_layer);
    terminate_tee_session(&ctx);

    /* Authentication Service */
    TEEC_UUID uuid_auth = TA_AUTH_SERVICE_UUID;
    prepare_tee_session(&ctx, uuid_auth);
    terminate_tee_session(&ctx);

    /* Logging Service */
    TEEC_UUID uuid_logging = TA_LOGGING_UUID;
    prepare_tee_session(&ctx, uuid_logging);
    terminate_tee_session(&ctx);

    /* Monitoring Service */
    TEEC_UUID uuid_monitoring = TA_MONITORING_UUID;
    prepare_tee_session(&ctx, uuid_monitoring);
    terminate_tee_session(&ctx);

    /* Secure Storage */
    TEEC_UUID uuid_secure_storage = TA_SECURE_STORAGE_UUID;
    prepare_tee_session(&ctx, uuid_secure_storage);
    terminate_tee_session(&ctx);

}