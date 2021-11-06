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
#include <split.h>
#include <hashtable.h>
#include <secure_storage_ta.h>

/* 
 * Writes the hash table data into persistent memory 
 */
static TEE_Result writeIntoMemory(const char* obj_id, size_t obj_id_sz, char* data, size_t data_sz)
{
        
        TEE_ObjectHandle object;
	TEE_Result res;
	uint32_t obj_data_flag;
        
	/*
	 * Create object in secure storage and fill with data
	 */
	obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |		/* we can later read the oject */
			TEE_DATA_FLAG_ACCESS_WRITE |		/* we can later write into the object */
			TEE_DATA_FLAG_ACCESS_WRITE_META |	/* we can later destroy or rename the object */
			TEE_DATA_FLAG_OVERWRITE;		/* destroy existing object of same ID */

        IMSG("Creating obj: %s->%s\n", obj_id, data);
	res = TEE_CreatePersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
					obj_data_flag, TEE_HANDLE_NULL, NULL, 0, &object);
        
	if (res != TEE_SUCCESS) {
		EMSG("Acessing Persistent Object failed 0x%08x", res);
                return res;
	}
        IMSG("Writing info!\n");
	/* Effectively writes the data into the object*/
        res = TEE_WriteObjectData(object, data, data_sz);
	if (res != TEE_SUCCESS) {
		EMSG("TEE_WriteObjectData failed 0x%08x", res);
		TEE_CloseAndDeletePersistentObject1(object);
	} else 
		TEE_CloseObject(object);
        
        return res;
}

/* 
 * Reads the persistent object from memory and adds it to the hash table 
 */
static void addItem(char* obj_id, size_t obj_id_sz)
{
	
        TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;
	uint32_t read_bytes;
	char *data;
                
        /*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		return;
	}

        /* Retrieves the object information*/
	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to create persistent object, res=0x%08x", res);
	        TEE_CloseObject(object);
                return;
	}
	data = TEE_Malloc(object_info.dataSize, 0);

        /* Effectively retrieves the object data*/
	res = TEE_ReadObjectData(object, data, object_info.dataSize, &read_bytes);
	if (res == TEE_SUCCESS)
                put(obj_id, data);
	if (res != TEE_SUCCESS || read_bytes != object_info.dataSize)
		EMSG("TEE_ReadObjectData failed 0x%08x, read %" PRIu32 " over %u", res, read_bytes, object_info.dataSize);
	    
        TEE_CloseObject(object);
        TEE_Free(data);

}

/* 
 * Processes each string in the data string, and add its content to the hash table 
 */
static void processItems(char* data)
{
        IMSG("Data to read: %s\n", data);
        if(!strcmp(data, NO_KEYS))
            return;

        char** ptr;
        int count, i;
        count = split(data, '|', &ptr);
        for(i = 0; i < count; i++)
            addItem(ptr[i], strlen(ptr[i]));
        
        TEE_Free(ptr);
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
        TEE_UUID uuid = TA_SECURE_STORAGE_UUID; 
        
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
        
        writeIntoMemory(TA_SECURE_STORAGE_PROOF, strlen(TA_SECURE_STORAGE_PROOF), hash, hash_sz);

        TEE_Free(hash);
        return TEE_SUCCESS;

}

/*
 * Retrieves the set of keys that corresponds to the key-value 
 * entries stored persistently in memory
 */
static TEE_Result readFromMemory(void)
{
	TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;
	uint32_t read_bytes;
        const char* service_id;
        size_t service_id_sz;
	char *data;
                
        service_id = TA_SECURE_STORAGE_STRING;
        service_id_sz = strlen(service_id);

        /*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE,
					service_id, service_id_sz,
					TEE_DATA_FLAG_ACCESS_READ |
					TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		IMSG("Object not yet created, will be created after the end of a new session");
		return TEE_SUCCESS;
	}

        /* Retrieves the object information*/
	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
	        TEE_CloseObject(object);
                return res;
	}
	data = TEE_Malloc(object_info.dataSize, 0);

        /* Effectively retrieves the object data*/
	res = TEE_ReadObjectData(object, data, object_info.dataSize, &read_bytes);
	if (res == TEE_SUCCESS)
                processItems(data);
	if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
		EMSG("TEE_ReadObjectData failed 0x%08x, read %" PRIu32 " over %u", res, read_bytes, object_info.dataSize);
	        TEE_CloseObject(object);
                return res;
	}

	TEE_CloseObject(object);
        TEE_Free(data);

        return TEE_SUCCESS;
}


/* 
 * Reads the persistent object from memory and removes it 
 */
static TEE_Result delete_object(char* obj_id, size_t obj_id_sz)
{
	
        TEE_ObjectHandle object;
	TEE_Result res;
	uint32_t obj_data_flag;
	
        /*
	 * Create object in secure storage and fill with data
	 */
	obj_data_flag = TEE_DATA_FLAG_ACCESS_READ |		/* we can later read the oject */
			TEE_DATA_FLAG_ACCESS_WRITE_META;	/* we can later destroy or rename the object */
	
        /*
	 * Check object exists and delete it
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
					obj_data_flag, /* Verifies if we have permission to delete object */
					&object);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to open persistent object, res=0x%08x", res);
		TEE_Free(obj_id);
		return res;
	}

        /* Effectively deletes objec Effectively deletes object*/
	TEE_CloseAndDeletePersistentObject1(object);
	TEE_Free(obj_id);

	return res;

}

/* 
 * Registers a new entry into the hash table, if there is no entry with the same obj_id 
 */
static TEE_Result create_item(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				                        TEE_PARAM_TYPE_MEMREF_INPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);

	TEE_Result res;
        char *obj_id;
	size_t obj_id_sz;
	char *data;
	size_t data_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

        /* Moves parameters to local pointers*/
	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	data_sz = params[1].memref.size;
	data = TEE_Malloc(data_sz, 0);
	if (!data) {
	        TEE_Free(obj_id);
		return TEE_ERROR_OUT_OF_MEMORY;
        }
	
        TEE_MemMove(data, params[1].memref.buffer, data_sz);
        
        IMSG("object to store: \n%s:%s\n", obj_id, data);
        put(obj_id, data);

        res = writeIntoMemory(obj_id, obj_id_sz, data, data_sz);
        TEE_Free(obj_id);
	TEE_Free(data);
        
        if (res != TEE_SUCCESS)
            return res;

        /* Rewrites the object that contains all keys*/
        IMSG("Updating persistently stored database");
        char* keys = TEE_Malloc(KEYS_MAX_LENGTH, 0);
        itemKeys(keys);
        res = writeIntoMemory(TA_SECURE_STORAGE_STRING, strlen(TA_SECURE_STORAGE_STRING), keys, strlen(keys));
	
        TEE_Free(keys);
        return TEE_SUCCESS;
}

/* 
 * Retrieves a entry from the hash table, identified by the same obj_id 
 */
static TEE_Result read_item(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				                        TEE_PARAM_TYPE_MEMREF_OUTPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);

        Item* item;
	char *obj_id;
	size_t obj_id_sz;
	char *data;
	size_t data_sz;
	size_t value_sz = 0;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

        /* Moves parameters to local pointers*/
	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

	data_sz = params[1].memref.size;
	data = TEE_Malloc(data_sz, 0);
	if (!data) {
                TEE_Free(obj_id);
		return TEE_ERROR_OUT_OF_MEMORY;
        }


        /* Retrieves object from hash table */
        IMSG("Loading data object: %s\n", obj_id);
        item = findItem(obj_id);
        if (item == NULL) {
                TEE_Free(obj_id);
                TEE_Free(data);
                return TEE_ERROR_ITEM_NOT_FOUND;
        }

        value_sz = strlen(item->value);
        TEE_MemMove(params[1].memref.buffer, item->value, value_sz);
	params[1].memref.size = value_sz;
    
        IMSG("successfully retrieved object");
        TEE_Free(obj_id);
        TEE_Free(data);
        return TEE_SUCCESS;
}

/* 
 * Deletes an entry from the hash table, identified by the same obj_id 
 */
static TEE_Result delete_item(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);
        Item* item;
	TEE_Result res;
	char *obj_id;
	size_t obj_id_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

        /* Moves parameters values to local pointers*/
	obj_id_sz = params[0].memref.size;
	obj_id = TEE_Malloc(obj_id_sz, 0);
	if (!obj_id)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(obj_id, params[0].memref.buffer, obj_id_sz);

        /* Deletes object from hash table */
        IMSG("Deleting object: %s\n", obj_id);
        item = removeItem(obj_id);
        if (item == NULL) {
                TEE_Free(obj_id);
                return TEE_ERROR_ITEM_NOT_FOUND;
        }

        /* Deletes object that contains the key-value*/
        IMSG("Deleting persistent object");
        res = delete_object(obj_id, obj_id_sz);
        
        /* Rewrites the object that contains all keys*/
        IMSG("Updating persistently stored database");
        char* keys = TEE_Malloc(KEYS_MAX_LENGTH, 0);
        itemKeys(keys);
        res = writeIntoMemory(TA_SECURE_STORAGE_STRING, strlen(TA_SECURE_STORAGE_STRING), keys, strlen(keys));

        TEE_Free(keys);
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
        const char* obj_id = TA_SECURE_STORAGE_PROOF;
        uint32_t obj_id_sz = strlen(TA_SECURE_STORAGE_PROOF);
        
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
        IMSG("Started Entry Point Creation");
        readFromMemory();	
        IMSG("Generating attestation proof");
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
	clear();
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t __unused param_types,
				    TEE_Param __unused params[4],
				    void __unused **session)
{
	/* Nothing to do */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __unused *session)
{

	IMSG("Goodbye!\n");
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from Normal World.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __unused *session,
				      uint32_t command,
				      uint32_t param_types,
				      TEE_Param params[4])
{
	switch (command) {
	case TA_SECURE_STORAGE_WRITE_DATA:
		return create_item(param_types, params);
	case TA_SECURE_STORAGE_READ_DATA:
		return read_item(param_types, params);
	case TA_SECURE_STORAGE_DELETE_DATA:
		return delete_item(param_types, params);
	case TA_SECURE_STORAGE_GET_PROOF:
		return get_attestation_proof(param_types, params);
	default:
		EMSG("Command ID 0x%x is not supported", command);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
