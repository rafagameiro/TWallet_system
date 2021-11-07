#include <err.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>
#include <tee_session.h>

/* For the UUID (found in the TA's h-file(s)) */
#include <adaptation_layer_ta.h>

#define MAX_DATA_SIZE        7000


JNIEXPORT jboolean
JNICALL Java_com_android_twallet_secure_utils_TWalletUtils_setTriggerTA(JNIEnv *env, jobject obj,
                                                                        jboolean jbool) {
    TEEC_UUID uuid = TA_ADAPTATION_LAYER_UUID;
    struct test_ctx ctx;
    TEEC_Result res;
    TEEC_Operation op;
    uint32_t err_origin;

    prepare_tee_session(&ctx, uuid);

    /* Clear the TEEC_Operation struct */
    memset(&op, 0, sizeof(op));

    /* Specify the argument type*/
    op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE, TEEC_NONE, TEEC_NONE);

    /* Gets data contents*/
    op.params[0].value.a = (jbool == JNI_TRUE);

    res = TEEC_InvokeCommand(&ctx.sess, AL_TRIGGER_MONITORING, &op, &err_origin);
    if (res != TEEC_SUCCESS)
        return false;
    printf("TA successfully read event log\n");

    terminate_tee_session(&ctx);

    return true;
}