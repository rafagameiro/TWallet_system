#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <tee_session.h>
#include <adaptation_layer_ta.h>

#define MAX_CREDENTIALS_SIZE        7000

JNIEXPORT jboolean
JNICALL
Java_com_android_twallet_secure_utils_TWalletUtils_storeCredentialsTA(JNIEnv *env, jobject obj,
                                                                      jstring sId, jstring jId,
                                                                      jstring jPwd) {
    TEEC_UUID uuid = TA_ADAPTATION_LAYER_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_INPUT, TEEC_NONE,
                                     TEEC_NONE);


    /* Gets program UUID*/
    const char *id = (*env)->GetStringUTFChars(env, sId, 0);
    size_t id_len = (size_t) (*env)->GetStringUTFLength(env, sId);

    /* Gets credentials to store
     * The credentials will be stored in a single string, separated by a special character "|"
     */
    const char *aID = (*env)->GetStringUTFChars(env, jId, 0);
    const char *aPwd = (*env)->GetStringUTFChars(env, jPwd, 0);
    char *acc = malloc(sizeof(aID));

    strcpy(acc, aID);
    strcat(acc, "|");
    strcat(acc, aPwd);

    op.params[0].tmpref.buffer = (void *) id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = acc;
    op.params[1].tmpref.size = strlen(acc);

    res = TEEC_InvokeCommand(&ctx.sess, AL_STORE_CREDENTIALS, &op, &err_origin);
	terminate_tee_session(&ctx);
	
    if (res != TEEC_SUCCESS)
        return false;
    printf("TA successfully store credentials\n");

    return true;
}

/* Retrieves the credentials stored in the secure world*/
jobjectArray getCredentials(JNIEnv *env, char *data, size_t data_len) {
    char *content = malloc(data_len);
    char delim[] = "|";
    size_t len = data_len > 0 ? 2 : 0;

    strcpy(content, data);
    char *ptr = strtok(content, delim);

    jobjectArray credentials;
    credentials = (*env)->NewObjectArray(env, len, (*env)->FindClass(env, "java/lang/String"),
                                         NULL);

    for (size_t i = 0; i < len; i++) {
        (*env)->SetObjectArrayElement(env, credentials, i, (*env)->NewStringUTF(env, ptr));
        ptr = strtok(NULL, delim);
    }
    free(content);
    return credentials;
}

JNIEXPORT jobjectArray
JNICALL
Java_com_android_twallet_secure_utils_TWalletUtils_loadCredentialsTA(JNIEnv *env, jobject obj,
                                                                     jstring sId) {
    TEEC_UUID uuid = TA_ADAPTATION_LAYER_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;
    char read_data[MAX_CREDENTIALS_SIZE];

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_INPUT, TEEC_MEMREF_TEMP_OUTPUT, TEEC_NONE,
                                     TEEC_NONE);


    const char *id = (*env)->GetStringUTFChars(env, sId, 0);
    size_t id_len = (size_t) (*env)->GetStringUTFLength(env, sId);

    op.params[0].tmpref.buffer = (void*) id;
    op.params[0].tmpref.size = id_len;

    op.params[1].tmpref.buffer = read_data;
    op.params[1].tmpref.size = sizeof(read_data);

    res = TEEC_InvokeCommand(&ctx.sess, AL_LOAD_CREDENTIALS, &op, &err_origin);
    terminate_tee_session(&ctx);

    if (res != TEEC_SUCCESS)
        return NULL;
    printf("TA successfully store credentials\n");

    read_data[op.params[1].tmpref.size] = '\0';
    return getCredentials(env, read_data, op.params[1].tmpref.size);
}

JNIEXPORT jboolean
JNICALL
Java_com_android_twallet_secure_utils_TWalletUtils_deleteCredentialsTA(JNIEnv *env, jobject obj,
                                                                       jstring sId) {
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


    const char *id = (*env)->GetStringUTFChars(env, sId, 0);
    size_t id_len = (size_t) (*env)->GetStringUTFLength(env, sId);

    op.params[0].tmpref.buffer = id;
    op.params[0].tmpref.size = id_len;

    res = TEEC_InvokeCommand(&ctx.sess, AL_DELETE_CREDENTIALS, &op, &err_origin);
	terminate_tee_session(&ctx);
	
    if (res != TEEC_SUCCESS && res != TEEC_ERROR_ITEM_NOT_FOUND)
        return false;
    printf("TA successfully store credentials\n");

    return true;
}
