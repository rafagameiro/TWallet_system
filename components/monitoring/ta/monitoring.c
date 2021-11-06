/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <string.h>
#include <monitoring_ta.h>

typedef struct {
   uint32_t  service;
   uint32_t  allowed_cmd;
} Component;

static Component permissions[COMPONENT_MAX_NUM];
static bool filter;

/* 
 * Writes the generated hash proof data into persistent memory 
 */
static TEE_Result writeIntoMemory(char* data, size_t data_sz)
{
        
        TEE_ObjectHandle object;
	TEE_Result res;
	uint32_t obj_data_flag;
        const char* obj_id = TA_MONITORING_PROOF;
        size_t obj_id_sz = strlen(TA_MONITORING_PROOF);
                
	/*
	 * Create object in secure storage and fill with data
	 */
	obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |		/* we can later read the oject */
			TEE_DATA_FLAG_ACCESS_WRITE |		/* we can later write into the object */
			TEE_DATA_FLAG_ACCESS_WRITE_META |	/* we can later destroy or rename the object */
			TEE_DATA_FLAG_OVERWRITE;		/* destroy existing object of same ID */

	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
					obj_data_flag, TEE_HANDLE_NULL, NULL, 0, &object);

	if (res != TEE_SUCCESS) {
		EMSG("TEE_CreatePersistentObject failed 0x%08x", res);
                return res;
	}

        IMSG("Writing Info!");
	/* Effectively writes the data into the object*/
        res = TEE_WriteObjectData(object, data, data_sz);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_WriteObjectData failed 0x%08x", res);
		TEE_CloseAndDeletePersistentObject1(object);
	} else {
                IMSG("Data successfully stored");
		TEE_CloseObject(object);
	}
        
        return res;
}

/*
 * Generate an hash proof so that in can be stored securely 
 * to later be used during the attestation process
 */
static TEE_Result generate_proof(void)
{
        
        char segment[SEGMENT_MAX_LENGTH];
        void* hash;
        uint32_t hash_sz = 50;
	TEE_OperationHandle op;
	TEE_Result res;
        TEE_UUID uuid = TA_MONITORING_UUID; 
        
        hash = TEE_Malloc(hash_sz, 0);
	if (!hash)
		return TEE_ERROR_OUT_OF_MEMORY;

        res = TEE_AllocateOperation(&op, TEE_ALG_SHA256, TEE_MODE_DIGEST, 0); 
        if (res != TEE_SUCCESS)
            return res;

        /* For each uuid element, it adds to the digest function*/
        snprintf(segment, strlen(segment), "%" PRIu32, uuid.timeLow);
        TEE_DigestUpdate(op, (void*) segment, strlen(segment));	
        
        snprintf(segment, strlen(segment), "%" PRIu32, uuid.timeMid);
        TEE_DigestUpdate(op, (void*) segment, strlen(segment));
        	
        snprintf(segment, strlen(segment), "%" PRIu32, uuid.timeHiAndVersion);
        TEE_DigestUpdate(op, (void*) segment, strlen(segment));
        	
        res = TEE_DigestDoFinal(op, (void*) uuid.clockSeqAndNode, strlen( (char*) uuid.clockSeqAndNode), hash, &hash_sz);
        if (res != TEE_SUCCESS)
            return res;
        
        writeIntoMemory(hash, hash_sz);

        TEE_Free(hash);
        return TEE_SUCCESS;

}

/*
 * Verifies, according with its database 
 * if the following command can be requested to the Secure Storage
 */
static bool isSecureStorageAllowed(uint32_t service, uint32_t command)
{
        
        if(service != permissions[0].service)
            return false;

        if(command >= permissions[0].allowed_cmd)
            return false;

        return true;
}

/*
 * Verifies, according with its database 
 * if the following command can be requested to the Authentication Service
 */
static bool isAuthServiceAllowed(uint32_t service, uint32_t command)
{
        
        if(service != permissions[1].service)
            return false;

        if(command >= permissions[1].allowed_cmd)
            return false;

        return true;

}

/*
 * Verifies, according with its database 
 * if the following command can be requested to the Logging Service
 */
static bool isLoggingAllowed(uint32_t service, uint32_t command)
{

        if(service != permissions[2].service)
            return false;

        if(command >= permissions[2].allowed_cmd)
            return false;

        return true;
}

/*
 * Verifies if the service and command passed as argument can be requested
 */
static TEE_Result filter_op(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);
	uint32_t service;
        uint32_t command;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
	    return TEE_ERROR_BAD_PARAMETERS;

        if (!filter)
            return TEE_SUCCESS;

        /* Moves parameters values to local variables*/        
        service = params[0].value.a;
        command = params[0].value.b;

        IMSG("Checking if operation is allowed");
        if (isSecureStorageAllowed(service, command))
            return TEE_SUCCESS;
        else if (isAuthServiceAllowed(service, command))
            return TEE_SUCCESS;
        else if (isLoggingAllowed(service, command))
            return TEE_SUCCESS;

        return TEE_ERROR_ACCESS_DENIED;
}

/*
 * Changes the trigger variable value, 
 * so that the TA can or not perform the filtering of requests
 */
static TEE_Result trigger_filter(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);
	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
	    return TEE_ERROR_BAD_PARAMETERS;

        IMSG("Changed trigger value to: %s\n", params[0].value.a ? "true" : "false");
        filter = params[0].value.a;        
        	
        return TEE_SUCCESS;
}

/*
 * Retrieves the generated hash proof during the TA startup 
 * and sends it back to the Normal World
 */
static TEE_Result get_attestation_proof(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);
        
	TEE_Result res;
        TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
        const char* obj_id = TA_MONITORING_PROOF;
        uint32_t obj_id_sz = strlen(TA_MONITORING_PROOF);
        
        void* hash;
        uint32_t hash_sz;
        
        /*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
	    return TEE_ERROR_BAD_PARAMETERS;

        /*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
        IMSG("Retrieving proof");
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		return res;
	}

        /* Retrieves the object information*/
	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to create persistent object, res=0x%08x", res);
	        TEE_CloseObject(object);
                return res;
	}
	hash_sz = params[0].memref.size;
        hash = TEE_Malloc(hash_sz, 0);
	if (!hash)
		return TEE_ERROR_OUT_OF_MEMORY;

        /* Effectively retrieves the object data*/
        IMSG("Reading object with proof data");
	res = TEE_ReadObjectData(object, hash, object_info.dataSize, &hash_sz);
	if (res == TEE_SUCCESS) {
                TEE_MemMove(params[0].memref.buffer, hash, hash_sz);
                params[0].memref.size = hash_sz;
        }

	if (res != TEE_SUCCESS || hash_sz != object_info.dataSize)
		EMSG("TEE_ReadObjectData failed 0x%08x, read %" PRIu32 " over %u", res, hash_sz, object_info.dataSize);

	TEE_CloseObject(object);
        TEE_Free(hash);
        return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
    filter = true;
    
    /* Info on Secure Storage*/
    permissions[0].service = 0;
    permissions[0].allowed_cmd = 3;

    /* Info on Authentication Service*/
    permissions[1].service = 1;
    permissions[1].allowed_cmd = 3;
        
    /* Info on Logging Service*/
    permissions[2].service = 2;
    permissions[2].allowed_cmd = 1;

    IMSG("Generating proof");
    generate_proof();
	
    IMSG("Entry Point successfuly created!\n");
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
	case TA_MONITORING_FILTER_OP:
	    return filter_op(param_types, params);
	case TA_MONITORING_TRIGGER_FILTER:
	    return trigger_filter(param_types, params);
	case TA_MONITORING_GET_PROOF:
	    return get_attestation_proof(param_types, params);
	default:
            EMSG("Command ID 0x%x is not supported", command);
            return TEE_ERROR_NOT_SUPPORTED;
	}
}
