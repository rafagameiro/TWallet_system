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
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <adaptation_layer_defines.h>

/*
 * Writes data into persistently into OP-TEE Secure Storage mechanism
 */
static TEE_Result writeIntoMemory(char* data, size_t data_sz)
{
        
        TEE_ObjectHandle object;
	TEE_Result res;
	uint32_t obj_data_flag;
        const char* obj_id = TA_ADAPTATION_LAYER_PROOF;
        size_t obj_id_sz = strlen(TA_ADAPTATION_LAYER_PROOF);
                
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

        IMSG("Writing Proof into memory!");
	/* Effectively writes the data into the object*/
        res = TEE_WriteObjectData(object, data, data_sz);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_WriteObjectData failed 0x%08x", res);
		TEE_CloseAndDeletePersistentObject1(object);
	} else {
                IMSG("Proof successfully stored");
		TEE_CloseObject(object);
	}
        
        return res;
}

/**
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
        TEE_UUID uuid = TA_ADAPTATION_LAYER_UUID; 
        
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

/**
 * Performs a call to the Monitoring Service to verify if the command can be executed
 */
static TEE_Result filter_operation(uint32_t service, uint32_t command)
{
        
        const TEE_UUID system_uuid = TA_MONITORING_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_VALUE_INPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;


	params[0].value.a = service;
	params[0].value.b = command;

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_MONITORING_FILTER_OP, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
    
        return res;
}

/**
 * Performs a call to the Logging Service to register a new entry into the event log
 */
static TEE_Result log_new_entry(const char* service, const char* command)
{
        
        const TEE_UUID system_uuid = TA_LOGGING_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        const uint32_t param_types = TEE_PARAM_TYPES( TEE_PARAM_TYPE_MEMREF_INPUT,
						      TEE_PARAM_TYPE_MEMREF_INPUT,
						      TEE_PARAM_TYPE_NONE,
						      TEE_PARAM_TYPE_NONE);

	TEE_Param params[TEE_NUM_PARAMS] = { 0 };

	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

	params[0].memref.buffer = service;
	params[0].memref.size = strlen(service);
        
	params[1].memref.buffer = command;
	params[1].memref.size = strlen(command);

	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_LOGGING_NEW_ENTRY, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
    
        return res;
}

/**
 * Performs a call to the Authentication Service to register a new set of credentials
 */
static TEE_Result store_credentials(uint32_t param_types, TEE_Param params[4])
{

        const TEE_UUID system_uuid = TA_AUTH_SERVICE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;

        IMSG("Filtering operation");
        res = filter_operation(1, 0); //AUTH_SERVICE, STORE_CREDENTIALS
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }


        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_AUTH_SERVICE_STORE_CREDENTIALS, param_types, params, &ret_origin);

	TEE_CloseTASession(session);

	if (res != TEE_SUCCESS)
		return res;

        IMSG("New Log Entry");
        res = log_new_entry("AUTH_SERVICE", "STORE_CREDENTIALS");
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create new log entry, res=0x%08x", res);
                return res;
        }
        	
        return res;

}

/**
 * Performs a call to the Authentication Service to retrieve a set of credentials
 */
static TEE_Result load_credentials(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_AUTH_SERVICE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        IMSG("Filtering operation");
        res = filter_operation(1, 1); //AUTH_SERVICE, LOAD_CREDENTIALS
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }

        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_AUTH_SERVICE_LOAD_CREDENTIALS, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
        	
	if (res != TEE_SUCCESS)
		return res;

        IMSG("New Log Entry");
        res = log_new_entry("AUTH_SERVICE", "LOAD_CREDENTIALS");
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create new log entry, res=0x%08x", res);
                return res;
        }
        
        return res;

}

/**
 * Performs a call to the Authentication Service to delete a set of credentials
 */
static TEE_Result delete_credentials(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_AUTH_SERVICE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        IMSG("Filtering operation");
        res = filter_operation(1, 2); //AUTH_SERVICE, DELETE_CREDENTIALS
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }

        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_AUTH_SERVICE_DELETE_CREDENTIALS, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
        	
	if (res != TEE_SUCCESS)
		return res;

        IMSG("New Log Entry");
        res = log_new_entry("AUTH_SERVICE", "DELETE_CREDENTIALS");
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create new log entry, res=0x%08x", res);
                return res;
        }
        	
        return res;

}

/*
 * Perfoms a call to the Secure Storage to retrieve a piece of stored data
 */
static TEE_Result storage_read_data(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_SECURE_STORAGE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        IMSG("Filtering operation");
        res = filter_operation(0, 0); //SECURE_STORAGE, READ_DATA
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }

        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_SECURE_STORAGE_READ_DATA, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
	
        if (res != TEE_SUCCESS)
		return res;
        	
        IMSG("New Log Entry");
        res = log_new_entry("SECURE_STORAGE", "READ_DATA");
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create new log entry, res=0x%08x", res);
                return res;
        }
        	
        return res;

}

/*
 * Performs a call to the Secure Storage to storage a new piece of data
 */
static TEE_Result storage_write_data(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_SECURE_STORAGE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        IMSG("Filtering operation");
        res = filter_operation(0, 1); //SECURE_STORAGE, WRITE_DATA
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }

        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_SECURE_STORAGE_WRITE_DATA, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
	
        if (res != TEE_SUCCESS)
		return res;
        	
        IMSG("New Log Entry");
        res = log_new_entry("SECURE_STORAGE", "WRITE_DATA");
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create new log entry, res=0x%08x", res);
                return res;
        }
        	
        return res;

}

/*
 * Performs a call to the Secure Storage to delete a piece of data stored
 */
static TEE_Result storage_delete_data(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_SECURE_STORAGE_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        IMSG("Filtering operation");
        res = filter_operation(0, 2); //SECURE_STORAGE, DELETE_DATA
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }

        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_SECURE_STORAGE_DELETE_DATA, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
	
        if (res != TEE_SUCCESS)
		return res;
        	
        IMSG("New Log Entry");
        res = log_new_entry("SECURE_STORAGE", "DELETE_DATA");
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create new log entry, res=0x%08x", res);
                return res;
        }
        	
        return res;

}

/*
 * Performs a call to the Logging Service to retrieve the event log
 */
static TEE_Result log_read_data(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_LOGGING_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;
        
        IMSG("Filtering operation");
        res = filter_operation(2, 0); //LOGGING, LOGGING_READ_DATA
        if(res != TEE_SUCCESS) {
                EMSG("Failed to create persistent object, res=0x%08x", res);
                return res;
        }

        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_LOGGING_READ_DATA, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
        	
        return res;

}

/*
 * Performs a call to the Monitoring Service to trigger the monitoring functionality
 */
static TEE_Result trigger_monitoring(uint32_t param_types, TEE_Param params[4])
{
        
        const TEE_UUID system_uuid = TA_MONITORING_UUID;
	TEE_TASessionHandle session = TEE_HANDLE_NULL;
	TEE_Result res = TEE_ERROR_GENERIC;
	uint32_t ret_origin = 0;


        IMSG("Opening session");
	res = TEE_OpenTASession(&system_uuid, TEE_TIMEOUT_INFINITE,
				0, NULL, &session, &ret_origin);
	if (res != TEE_SUCCESS)
		return res;

        IMSG("Invoking command");
	res = TEE_InvokeTACommand(session, TEE_TIMEOUT_INFINITE,
				  TA_MONITORING_TRIGGER_FILTER, param_types, params, &ret_origin);

	TEE_CloseTASession(session);
        	
        return res;

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
        const char* obj_id = TA_ADAPTATION_LAYER_PROOF;
        uint32_t obj_id_sz = strlen(TA_ADAPTATION_LAYER_PROOF);
        
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
	case AL_STORE_CREDENTIALS:
		return store_credentials(param_types, params);
	case AL_LOAD_CREDENTIALS:
		return load_credentials(param_types, params);
	case AL_DELETE_CREDENTIALS:
		return delete_credentials(param_types, params);
	case AL_STORAGE_READ_DATA:
		return storage_read_data(param_types, params);
	case AL_STORAGE_WRITE_DATA:
		return storage_write_data(param_types, params);
	case AL_STORAGE_DELETE_DATA:
		return storage_delete_data(param_types, params);
	case AL_LOGGING_READ_DATA:
		return log_read_data(param_types, params);
	case AL_TRIGGER_MONITORING:
		return trigger_monitoring(param_types, params);
	case TA_ADAPTATION_LAYER_GET_PROOF:
		return get_attestation_proof(param_types, params);
	default:
                EMSG("Command ID 0x%x is not supported", command);
                return TEE_ERROR_NOT_SUPPORTED;
	}
}
