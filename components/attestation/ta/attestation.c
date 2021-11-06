/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */

#include <string.h>
#include <mbedtls/base64.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <string_ext.h>
#include <attestation_defines.h>

static void* tee_layer_proof;
static void* monitoring_proof;
static void* secure_storage_proof;
static void* auth_service_proof;
static void* logging_proof;

static TEE_ObjectHandle key_pair;

/*
 * Stored the generated keypair persistently in memory
 * using the OPTEE Secure Storage Mechanism
 */
static TEE_Result store_keys(void)
{
        TEE_ObjectHandle object;
	TEE_Result res;
	uint32_t obj_data_flag;
 
        const char* keys_id = ATTESTATION_KEYS_ID;
        uint32_t keys_id_sz = strlen(keys_id); 

	/*
	 * Create object in secure storage and fill with data
	 */
	obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |		/* we can later read the oject */
			TEE_DATA_FLAG_ACCESS_WRITE |		/* we can later write into the object */
			TEE_DATA_FLAG_ACCESS_WRITE_META |	/* we can later destroy or rename the object */
			TEE_DATA_FLAG_OVERWRITE;		/* destroy existing object of same ID */

	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, keys_id, keys_id_sz,
					obj_data_flag, key_pair, NULL, 0, &object);

	if (res != TEE_SUCCESS) 
		EMSG("TEE_CreatePersistentObject failed 0x%08x", res);
        else
                IMSG("Keys successfully stored!");
        
        TEE_CloseObject(object);
        return res;
}

/*
 * Retrieves the keypair stored persistently in memory
 */
static TEE_Result load_keys(void)
{
	TEE_Result res;
	key_pair = TEE_HANDLE_NULL;
        
        const char* keys_id = ATTESTATION_KEYS_ID;
        uint32_t keys_id_sz = strlen(keys_id); 

        /*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
        IMSG("Accessing secure storage");
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					keys_id, keys_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&key_pair);
	if (res != TEE_SUCCESS) 
		IMSG("Object not yet created, will be created after the end of a new session");
        else
                IMSG("Key successfully retrieved");

        return res;
}

/*
 * Generates a new RSA keypair to be stored persistently in Secure Storage
 */
static TEE_Result generate_keys(void)
{
    TEE_Result res;
    const uint32_t key_sz = 2048;

    IMSG("Allocating object\n");
    TEE_FreeTransientObject(key_pair);
    res = TEE_AllocateTransientObject(TEE_TYPE_RSA_KEYPAIR, key_sz, &key_pair);
    if (res) {
	EMSG("TEE_AllocateTransientObject(%#" PRIx32 ", %" PRId32 "): %#" PRIx32, TEE_TYPE_RSA_KEYPAIR, key_sz, res);
	return res;
    }

    IMSG("Generate key\n");
    res = TEE_GenerateKey(key_pair, key_sz, NULL, 0);
    if (res) {
	EMSG("TEE_GenerateKey(%" PRId32 "): %#" PRIx32, 2048, res);
	TEE_FreeTransientObject(key_pair);
	return res;
    }

    store_keys();

    return TEE_SUCCESS;
}

/*
 * The method will sign the already generated hash value using the retrieved/generated keypair
 */
static TEE_Result sign_digest(void* digest, uint32_t digest_sz, void** proof, uint32_t* proof_sz)
{
    TEE_Result res;
    TEE_OperationHandle op;
    TEE_ObjectInfo key_info;

    if(key_pair == TEE_HANDLE_NULL) {
        res = generate_keys();
        if (res != TEE_SUCCESS)
            return res; 
    }

    res = TEE_GetObjectInfo1(key_pair, &key_info);
    if (res) {
	EMSG("TEE_GetObjectInfo1: %#" PRIx32, res);
	return res;
    }

    IMSG("Allocation operation\n");
    res = TEE_AllocateOperation(&op, TEE_ALG_RSASSA_PKCS1_V1_5_SHA256, TEE_MODE_SIGN, key_info.maxObjectSize);
    if (res != TEE_SUCCESS) {
	EMSG("TEE_AllocateOperation(TEE_MODE_SIGN, %#" PRIx32 ", %" PRId32 "): %#" PRIx32, TEE_ALG_RSASSA_PKCS1_V1_5_SHA256, key_info.keySize, res);
	return res;
    }

    res = TEE_SetOperationKey(op, key_pair);
    if (res != TEE_SUCCESS) {
	EMSG("TEE_SetOperationKey: %#" PRIx32, res);
        TEE_FreeOperation(op);
	return res;
    }

    IMSG("Asymmetric Sign\n");
    res = TEE_AsymmetricSignDigest(op, (TEE_Attribute*) NULL, 0, digest, digest_sz, *proof, proof_sz);
    if (res != TEE_SUCCESS)
	EMSG("TEE_AsymmetricSignDigest(%" PRId32 ", %" PRId32 "): %#" PRIx32, *proof_sz, 256, res);

    TEE_FreeOperation(op);
    return res;
}

/*
 * Generates an hash of the generated proof to futurely be signed 
 */
static TEE_Result generate_unsigned_proof(void* digest, uint32_t digest_sz, void** u_digest, uint32_t* u_digest_sz)
{
    TEE_Result res;
    TEE_OperationHandle op;
    char hash[HASH_MAX_LENGTH];
    uint32_t hash_sz = HASH_MAX_LENGTH;
        
    IMSG("Started hash generation...");
    res = TEE_AllocateOperation(&op, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0); 
    if (res != TEE_SUCCESS)
        return res;
        
    /* Generate hash to be signed*/
    IMSG("Generating hash to be signed...");
    res = TEE_DigestDoFinal(op, digest, digest_sz, (void*) hash, &hash_sz);
    if (res != TEE_SUCCESS)
        return res;
       
    *u_digest_sz = hash_sz;
    *u_digest = TEE_Malloc(*u_digest_sz, 0);
    if (!*u_digest)
	return TEE_ERROR_OUT_OF_MEMORY;

    TEE_MemMove(*u_digest, hash, hash_sz);
    return res;
}

/*
 * Generates the proof using the many retrieved hash proof from the other Secure Components
 * The methods will combine the retrieved hash proofs and add the nonce value 
 * sent from the Normal World
 */
static TEE_Result generate_digest(char** digest, uint32_t* digest_sz, size_t nonce)
{
	TEE_OperationHandle op;
	TEE_Result res;
        char hash[HASH_MAX_LENGTH];
        uint32_t hash_sz = HASH_MAX_LENGTH;
        char nonce_str[30];
        
        IMSG("Started hash generation...");
        res = TEE_AllocateOperation(&op, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0); 
        if (res != TEE_SUCCESS)
            return res;
        
        /* Generate the cumulative hash*/
        TEE_DigestUpdate(op, tee_layer_proof, strlen( (char *) tee_layer_proof));	
        TEE_DigestUpdate(op, monitoring_proof, strlen( (char*)  monitoring_proof));	
        TEE_DigestUpdate(op, secure_storage_proof, strlen( (char*) secure_storage_proof));	
        TEE_DigestUpdate(op, auth_service_proof, strlen( (char*) auth_service_proof));	
        
        IMSG("Generating cumulative hash...");
        res = TEE_DigestDoFinal(op, logging_proof, strlen( (char*) logging_proof), (void*) hash, &hash_sz);
        if (res != TEE_SUCCESS)
            return res;

        IMSG("Generating digest...");
        /* Generate the proof, before the sign*/
        snprintf(nonce_str, 30, "%" PRIu32, nonce);
        
        *digest_sz = hash_sz + strlen(nonce_str) + 3;
        *digest = TEE_Malloc(*digest_sz, 0);
	if (!*digest)
		return TEE_ERROR_OUT_OF_MEMORY;

        strlcpy(*digest, hash, *digest_sz);
        strlcat(*digest, "|", *digest_sz);
        strlcat(*digest, nonce_str, *digest_sz);

        return TEE_SUCCESS;
}

/*
 * Generates the attestation proof so that it can be sent to the Normal World
 */
static TEE_Result generate_proof(void** digest, uint32_t* digest_sz, void** att_proof, uint32_t* att_proof_sz, size_t nonce)
{
	TEE_Result res;
        void* u_digest;
        uint32_t u_digest_sz;
   
        res = generate_digest((char**) digest, digest_sz, nonce); 
        if (res != TEE_SUCCESS)
            return res;

        *att_proof = TEE_Malloc(PROOF_MAX_LENGTH, 0); 
        if (!*att_proof)
	    return TEE_ERROR_OUT_OF_MEMORY;
        *att_proof_sz = PROOF_MAX_LENGTH;

        IMSG("Signing proof...");
        res = generate_unsigned_proof(*digest, *digest_sz, &u_digest, &u_digest_sz);
        if (res != TEE_SUCCESS)
            return res;

        res = sign_digest(u_digest, u_digest_sz, att_proof, att_proof_sz);
        if (res != TEE_SUCCESS)
            return res;

        TEE_Free(u_digest);
        return res;

}

/*
 * Retrieves the keypair's public key exponent and modulus 
 * to be sent back to the Normal World 
 */
static TEE_Result get_pubkey(void** pubkey_exp, uint32_t* pubkey_exp_sz, void** pubkey_mod, uint32_t* pubkey_mod_sz)
{
    TEE_Result res;

    IMSG("Retrieving public key exponent");
    *pubkey_exp = TEE_Malloc(PUBKEY_EXP_LENGTH, 0);
    if (!*pubkey_exp)
	return TEE_ERROR_OUT_OF_MEMORY;
    *pubkey_exp_sz = PUBKEY_EXP_LENGTH;

    res = TEE_GetObjectBufferAttribute(key_pair, TEE_ATTR_RSA_PUBLIC_EXPONENT, *pubkey_exp, pubkey_exp_sz);
    if (res != TEE_SUCCESS)
        return res;

    IMSG("Retrieving public key modulus");
    *pubkey_mod = TEE_Malloc(PUBKEY_MOD_LENGTH, 0);
    if (!*pubkey_mod)
	return TEE_ERROR_OUT_OF_MEMORY;
    *pubkey_mod_sz = PUBKEY_MOD_LENGTH;

    res = TEE_GetObjectBufferAttribute(key_pair, TEE_ATTR_RSA_MODULUS, *pubkey_mod, pubkey_mod_sz);
    if (res != TEE_SUCCESS)
        return res;

    return TEE_SUCCESS;
}

/*
 * Retrieves the Logging Service generated hash proof 
 * and stores it on local variable
 */
static TEE_Result get_logging_proof(void)
{
        
        const TEE_UUID system_uuid = TA_LOGGING_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        char hash[HASH_MAX_LENGTH];

        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_MEMREF_OUTPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        params[0].memref.buffer = (void*) hash;
        params[0].memref.size = sizeof(hash);

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_LOGGING_GET_PROOF, param_types, params, &ret_origin);

	TEE_CloseTASession(session);

        logging_proof = TEE_Malloc(params[0].memref.size, 0);
        if (!logging_proof) 
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(logging_proof, params[0].memref.buffer, params[0].memref.size); 

        return res;
}

/*
 * Retrieves the Authentication Service generated hash proof 
 * and stores it on local variable
 */
static TEE_Result get_auth_service_proof(void)
{
        
        const TEE_UUID system_uuid = TA_AUTH_SERVICE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        char hash[HASH_MAX_LENGTH];

        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_MEMREF_OUTPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        params[0].memref.buffer = (void*) hash;
        params[0].memref.size = sizeof(hash);

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_AUTH_SERVICE_GET_PROOF, param_types, params, &ret_origin);

	TEE_CloseTASession(session);

        auth_service_proof = TEE_Malloc(params[0].memref.size, 0);
        if (!auth_service_proof) 
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(auth_service_proof, params[0].memref.buffer, params[0].memref.size); 

        return res;
}

/*
 * Retrieves the Secure Storage generated hash proof 
 * and stores it on local variable
 */
static TEE_Result get_secure_storage_proof(void)
{
        
        const TEE_UUID system_uuid = TA_SECURE_STORAGE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        char hash[HASH_MAX_LENGTH];

        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_MEMREF_OUTPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        params[0].memref.buffer = (void*) hash;
        params[0].memref.size = sizeof(hash);

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_SECURE_STORAGE_GET_PROOF, param_types, params, &ret_origin);

	TEE_CloseTASession(session);

        secure_storage_proof = TEE_Malloc(params[0].memref.size, 0);
        if (!secure_storage_proof) 
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(secure_storage_proof, params[0].memref.buffer, params[0].memref.size); 

        return res;
}

/*
 * Retrieves the Monitoring Service generated hash proof 
 * and stores it on local variable
 */
static TEE_Result get_monitoring_proof(void)
{
        
        const TEE_UUID system_uuid = TA_MONITORING_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        char hash[HASH_MAX_LENGTH];

        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_MEMREF_OUTPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        params[0].memref.buffer = (void*) hash;
        params[0].memref.size = sizeof(hash);

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_MONITORING_GET_PROOF, param_types, params, &ret_origin);

	TEE_CloseTASession(session);

        monitoring_proof = TEE_Malloc(params[0].memref.size, 0);
        if (!monitoring_proof) 
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(monitoring_proof, params[0].memref.buffer, params[0].memref.size); 

        return res;
}

/*
 * Retrieves the Adaptation Layer generated hash proof 
 * and stores it on local variable
 */
static TEE_Result get_tee_layer_proof(void)
{
        
        const TEE_UUID system_uuid = TA_ADAPTATION_LAYER_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        char hash[HASH_MAX_LENGTH];

        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_MEMREF_OUTPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        params[0].memref.buffer = (void*) hash;
        params[0].memref.size = sizeof(hash);

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_ADAPTATION_LAYER_GET_PROOF, param_types, params, &ret_origin);

	TEE_CloseTASession(session);

        tee_layer_proof = TEE_Malloc(params[0].memref.size, 0);
        if (!tee_layer_proof) 
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(tee_layer_proof, params[0].memref.buffer, params[0].memref.size); 

        return res;
}

/*
 * Retrieves the generated hash proofs generated by each Secure Component during system startup
 * so that it can generate an attestation proof to send back to the Normal World
 */
static TEE_Result get_attestation_proof(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
				                        TEE_PARAM_TYPE_MEMREF_OUTPUT,
				                        TEE_PARAM_TYPE_MEMREF_OUTPUT,
				                        TEE_PARAM_TYPE_MEMREF_OUTPUT);

        TEE_Result res;
        size_t nonce;

        /* Digest*/
	void* digest = NULL;
	size_t digest_sz;
        unsigned char* digest_b64 = NULL;
        size_t digest_b64_sz;

        /* Attestation Proof*/
	void* att_proof = NULL;
	size_t att_proof_sz;
        unsigned char* att_proof_b64 = NULL;
        size_t att_proof_b64_sz;

        /* Public Key*/
	void* pubkey_exp = NULL;
	size_t pubkey_exp_sz;
	void* pubkey_mod = NULL;
	size_t pubkey_mod_sz;
        unsigned char* pubkey_mod_b64 = NULL;
        size_t pubkey_mod_b64_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

        nonce = params[0].value.a;

        /* Retrieves each component proof*/
        IMSG("Getting TEE Layer proof");
        res = get_tee_layer_proof();
        if (res != TEE_SUCCESS)
            return TEE_ERROR_SIGNATURE_INVALID;
        
        IMSG("Getting Monitoring proof");
        res = get_monitoring_proof();
        if (res != TEE_SUCCESS)
            return TEE_ERROR_SIGNATURE_INVALID;

        IMSG("Getting Secure Storage proof");
        res = get_secure_storage_proof();
        if (res != TEE_SUCCESS)
            return TEE_ERROR_SIGNATURE_INVALID;

        IMSG("Getting Auth Service proof");
        res = get_auth_service_proof();
        if (res != TEE_SUCCESS)
            return TEE_ERROR_SIGNATURE_INVALID;

        IMSG("Getting Logging proof");
        res = get_logging_proof();
        if (res != TEE_SUCCESS)
            return TEE_ERROR_SIGNATURE_INVALID;

        /* Generates proof*/
        res = generate_proof(&digest, &digest_sz, &att_proof, &att_proof_sz, nonce);
        if (res != TEE_SUCCESS) {
            return res;
        }
        
        /* Retrieves Public key modulus and exponent*/
        res = get_pubkey(&pubkey_exp, &pubkey_exp_sz, &pubkey_mod, &pubkey_mod_sz);
        if (res != TEE_SUCCESS) {
            return res;
        }

        IMSG("Converting...");
        /* Converts to Base64*/
        digest_b64 = TEE_Malloc(BASE64_MAX_LENGTH, 0);
        if (!digest_b64)
	    return TEE_ERROR_OUT_OF_MEMORY;

        mbedtls_base64_encode(digest_b64, BASE64_MAX_LENGTH, &digest_b64_sz, digest, digest_sz);
        
        att_proof_b64 = TEE_Malloc(BASE64_MAX_LENGTH, 0);
        if (!att_proof_b64)
	    return TEE_ERROR_OUT_OF_MEMORY;

        mbedtls_base64_encode(att_proof_b64, BASE64_MAX_LENGTH, &att_proof_b64_sz, att_proof, att_proof_sz);

        IMSG("Converting pubkey mod");
        pubkey_mod_b64 = TEE_Malloc(BASE64_MAX_LENGTH, 0);
        if (!pubkey_mod_b64)
	    return TEE_ERROR_OUT_OF_MEMORY;
        
        mbedtls_base64_encode(pubkey_mod_b64, BASE64_MAX_LENGTH, &pubkey_mod_b64_sz, pubkey_mod, pubkey_mod_sz);
        
        /* Sends proof to application*/
	params[0].value.b = *((int*) pubkey_exp);
	
        TEE_MemMove(params[1].memref.buffer, digest_b64, digest_b64_sz);
	params[1].memref.size = digest_b64_sz;
	
        TEE_MemMove(params[2].memref.buffer, att_proof_b64, att_proof_b64_sz);
	params[2].memref.size = att_proof_b64_sz;
        
        TEE_MemMove(params[3].memref.buffer, pubkey_mod_b64, pubkey_mod_b64_sz);
	params[3].memref.size = pubkey_mod_b64_sz;

        TEE_Free(digest);
        TEE_Free(digest_b64);
        TEE_Free(att_proof);
        TEE_Free(att_proof_b64);
        TEE_Free(pubkey_exp);
        TEE_Free(pubkey_mod);
        TEE_Free(pubkey_mod_b64);
        return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
    /* Nothing to do */	
    load_keys();
    return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
    /* Nothing to do */
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
		TEE_Param __unused params[4], void __unused **sess_ctx)
{

        tee_layer_proof = NULL;
        monitoring_proof = NULL;
        secure_storage_proof = NULL;
        auth_service_proof = NULL;
        logging_proof = NULL;
	
        /*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Service started!\n");

	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __unused *sess_ctx)
{ 
        TEE_Free(tee_layer_proof);
        TEE_Free(monitoring_proof);
        TEE_Free(secure_storage_proof);
        TEE_Free(auth_service_proof);
        TEE_Free(logging_proof);

	IMSG("Goodbye!\n");
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __unused *sess_ctx, uint32_t command, uint32_t param_types, TEE_Param params[4])
{
	switch (command) {
	case TA_ATTESTATION_GET_PROOF:
		return get_attestation_proof(param_types, params);
	default:
                EMSG("Command ID 0x%x is not supported", command);
                return TEE_ERROR_NOT_SUPPORTED;
	}
}
