#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <inttypes.h>
#include <tee_session.h>
#include <adaptation_layer_ta.h>

#define MAX_DATA_SIZE        7000

JNIEXPORT jboolean
JNICALL Java_com_android_twallet_secure_utils_TWalletUtils_writeDataTA(JNIEnv *env, jobject obj, jstring jId,
                                                             jstring jData) {
    TEEC_UUID uuid = TA_ADAPTATION_LAYER_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE);


    /* Gets data contents*/
    const char *id = (*env)->GetStringUTFChars(env, jId, 0);
    const char *data = (*env)->GetStringUTFChars(env, jData, 0);

    size_t id_len = (size_t)(*env)->GetStringUTFLength(env, jId);
    size_t data_len = (size_t)(*env)->GetStringUTFLength(env, jData);

    op.params[0].tmpref.buffer = (void*) id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = (void*) data;
    op.params[1].tmpref.size = data_len;

    res = TEEC_InvokeCommand(&ctx.sess, AL_STORAGE_WRITE_DATA, &op, &err_origin);
	terminate_tee_session(&ctx);
	
    if (res != TEEC_SUCCESS)
        return false;
    printf("TA successfully wrote data\n");

    return true;

}

JNIEXPORT jstring JNICALL
Java_com_android_twallet_secure_utils_TWalletUtils_readDataTA(JNIEnv *env, jobject obj,
                                                              jstring jId) {

    TEEC_UUID uuid = TA_ADAPTATION_LAYER_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;
    char data[MAX_DATA_SIZE];

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE,
                                     TEEC_NONE);

    /* Gets data contents*/
    const char *id = (*env)->GetStringUTFChars(env, jId, 0);
    size_t id_len = (size_t) (*env)->GetStringUTFLength(env, jId);

    op.params[0].tmpref.buffer = (void *) id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = (void *) data;
    op.params[1].tmpref.size = sizeof(data);

    res = TEEC_InvokeCommand(&ctx.sess, AL_STORAGE_READ_DATA, &op, &err_origin);
    terminate_tee_session(&ctx);

    if (res != TEEC_SUCCESS)
        return NULL;
    printf("TA successfully retrieved content\n");

    data[op.params[1].tmpref.size] = '\0';
    return (*env)->NewStringUTF(env, data);

}

JNIEXPORT jboolean JNICALL
Java_com_android_twallet_secure_utils_TWalletUtils_deleteDataTA(JNIEnv *env, jobject obj, jstring jId) {

    TEEC_UUID uuid = TA_ADAPTATION_LAYER_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    /* Gets data contents*/
    const char *id = (*env)->GetStringUTFChars(env, jId, 0);
    size_t id_len = (size_t) (*env)->GetStringUTFLength(env, jId);

    op.params[0].tmpref.buffer = (void *) id;
    op.params[0].tmpref.size = id_len;

    res = TEEC_InvokeCommand(&ctx.sess, AL_STORAGE_DELETE_DATA, &op, &err_origin);
	terminate_tee_session(&ctx);
	
    if (res != TEEC_SUCCESS)
        return false;
    printf("TA successfully deleted content\n");

    return true;
}