#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* OP-TEE TEE client API (built by optee_client) */
#include <tee_client_api.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <attestation_ta.h>
#include <tee_session.h>
#include <jni.h>

JNIEXPORT jobjectArray
JNICALL Java_com_android_twallet_secure_utils_TWalletUtils_attestComponentsTA (JNIEnv * env, jobject obj, jint nonce)
{
    TEEC_UUID uuid = TA_ATTESTATION_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;
    char digest[BASE64_MAX_LENGTH];
    char proof[BASE64_MAX_LENGTH];
    char mod[BASE64_MAX_LENGTH];
    char exp[PUBKEY_EXP_LENGTH];

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct*/
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INOUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_MEMREF_TEMP_OUTPUT);

    op.params[0].value.a = nonce;
    
    op.params[1].tmpref.buffer = digest;
    op.params[1].tmpref.size = sizeof(digest);

    op.params[2].tmpref.buffer = proof;
    op.params[2].tmpref.size = sizeof(proof);

    op.params[3].tmpref.buffer = mod;
    op.params[3].tmpref.size = sizeof(mod);

    jobjectArray values;
    values = (*env)->NewObjectArray(env, 4, (*env)->FindClass(env, "java/lang/String"), NULL);

    res = TEEC_InvokeCommand(&ctx.sess, TA_ATTESTATION_GET_PROOF, &op, &err_origin);
	terminate_tee_session(&ctx);
	
    if (res != TEEC_SUCCESS)
        return values;
    printf("TA successfully attested\n");

    digest[op.params[1].tmpref.size] = '\0';
    proof[op.params[2].tmpref.size] = '\0';
    mod[op.params[3].tmpref.size] = '\0';
    snprintf(exp, PUBKEY_EXP_LENGTH, "%d", op.params[0].value.b);

    (*env)->SetObjectArrayElement(env, values, 0, (*env)->NewStringUTF(env, digest));
    (*env)->SetObjectArrayElement(env, values, 1, (*env)->NewStringUTF(env, proof));
    (*env)->SetObjectArrayElement(env, values, 2, (*env)->NewStringUTF(env, exp));
    (*env)->SetObjectArrayElement(env, values, 3, (*env)->NewStringUTF(env, mod));
	
    return values;
}
