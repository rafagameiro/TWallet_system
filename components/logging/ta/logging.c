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

#include <stdlib.h>
#include <string.h>
#include <string_ext.h>
#include <logging_ta.h>


typedef struct {
   char*        service;
   char*        command;
   TEE_Time        time;
} Entry;

static Entry event_log[LOG_MAX_SIZE];
static uint32_t size;
static bool dirty;

/* 
 * Reads the value in val, and places it in the corresponding entry field
 */
static void entry_segment(Entry *entry, char* val, int pos)
{
    
    switch(pos)
    {
        case 0: 
            entry->service = TEE_Malloc(strlen(val), 0);
            strlcpy(entry->service, val, strlen(val) + 1);
            break;
        case 1: 
            entry->command = TEE_Malloc(strlen(val), 0);
            strlcpy(entry->command, val, strlen(val) + 1);
            break;
        case 2: entry->time.seconds = (int) strtoul(val, (char **)NULL, 10);
            break;
        case 3: entry->time.millis = (int) strtoul(val, (char **)NULL, 10);
            break;
        default: break;
    }

}

/* 
 * Reads the string entry, and processes each one of the numbers 
 * in order to create a full entry struct
 */
static void process_entry(char *entry)
{
    Entry n_entry = {NULL, NULL, {0, 0}};
    int len = strlen(entry);
    char val[len];
    int pos = 0;
    int num = 0;
    
    for(int i = 1; i < len; i++)
    {
        if( entry[i] == ';' || entry[i] == '}' )
        {
            val[pos] = '\0';
            entry_segment(&n_entry, val, num);
            num++;
            memset(val, 0, pos);
            pos = 0;
            continue;
        }        
        val[pos++] = entry[i];
    }

    event_log[size++] = n_entry;
}

/* 
 * Processes the string that contains the entire log
 */
static void get_data(char *data)
{
    char substr[100];
    int pos = 0;
    int len = strlen(data);
    size = 0;
    dirty = false;
    
    for(int i = 1; i < len; i++)
    {
        substr[pos++] = data[i];
        if( data[i] == '}' ) 
        {
            substr[pos] = '\0';
            process_entry(substr);
            memset(substr, 0, pos);
            pos = 0;
        }
    }

}

/* 
 * Transforms the log stored in memory, 
 * and generates a string to later be saved in OPTEE private storage
 */
static void generate_data(char* string)
{
    char field[30];
    strlcpy(string, "[", LOG_DATA_MAX_LENGTH);
    
    for(uint32_t i = 0; i < size; i++) 
    {
    
        strlcat(string, "{", LOG_DATA_MAX_LENGTH);
        strlcpy(field, event_log[i].service, LOG_DATA_MAX_LENGTH);

        strlcat(field, ";", LOG_DATA_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);
        strlcpy(field, event_log[i].command, LOG_DATA_MAX_LENGTH);
        
        strlcat(field, ";", LOG_DATA_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);
        snprintf(field, strlen(field), "%" PRIu32, event_log[i].time.seconds);  

        strlcat(field, ";", LOG_DATA_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);
        snprintf(field, strlen(field), "%" PRIu32, event_log[i].time.millis);  

        strlcat(field, "}", LOG_DATA_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);

        if(i + 1 < size)
            strlcat(string, ",", 1);

    }

    strlcat(string, "]", LOG_DATA_MAX_LENGTH);
}

/* 
 * Transforms the log stored in memory, 
 * and generates a string to be send to the Normal World
 */
static void export_data(char* string)
{
    char field[SEGMENT_MAX_LENGTH];
    strlcpy(string, "[", LOG_DATA_MAX_LENGTH);
    
    for(uint32_t i = 0; i < size; i++) 
    {
    
        strlcat(string, "{", LOG_DATA_MAX_LENGTH);

        strlcpy(field, "\"service\": \"", SEGMENT_MAX_LENGTH);
        strlcat(field, event_log[i].service, SEGMENT_MAX_LENGTH);
        strlcat(field, "\",", SEGMENT_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);
        
        strlcpy(field, "\"command\": \"", SEGMENT_MAX_LENGTH);
        strlcat(field, event_log[i].command, SEGMENT_MAX_LENGTH);
        strlcat(field, "\",", SEGMENT_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);
        
        strlcpy(field, "\"time\": \"", SEGMENT_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);
        
        snprintf(field, SEGMENT_MAX_LENGTH, "%" PRIu32, event_log[i].time.millis);  
        strlcat(string, field, LOG_DATA_MAX_LENGTH);

        strlcat(field, "}", SEGMENT_MAX_LENGTH);
        strlcat(string, field, LOG_DATA_MAX_LENGTH);

        if(i + 1 < size)
            strlcat(string, ",", 1);

    }

    strlcat(string, "]", LOG_DATA_MAX_LENGTH);
}

/* 
 * Retrieves th log from OPTEE private storage and loads into memory
 */
static void read_log(void)
{

	TEE_ObjectHandle object;
	TEE_ObjectInfo object_info;
	TEE_Result res;
	uint32_t read_bytes;
	char* data;
        const char* obj_id = TA_LOGGING_STRING; 
        size_t obj_id_sz = strlen(TA_LOGGING_STRING); 

	/*
	 * Check the object exist and can be dumped into output buffer
	 * then dump it.
	 */
	res = TEE_OpenPersistentObject(TEE_STORAGE_PRIVATE, obj_id, obj_id_sz,
					TEE_DATA_FLAG_ACCESS_READ | TEE_DATA_FLAG_SHARE_READ,
					&object);
	if (res != TEE_SUCCESS) {
		IMSG("Object not yet created, will be created after the end of a new session");
                size = 0;
                dirty = false;
		return;
	}
        IMSG("Getting Object info!");
        /* Gets object info*/
	res = TEE_GetObjectInfo1(object, &object_info);
	if (res != TEE_SUCCESS) {
		EMSG("Failed to create persistent object, res=0x%08x", res);
	        TEE_CloseObject(object);
		return;
	}

        IMSG("getting object data Size!");
	if (object_info.dataSize > LOG_DATA_MAX_LENGTH) {
		
                /* Provided buffer is too short.*/
		res = TEE_ERROR_SHORT_BUFFER;
	        TEE_CloseObject(object);
		return;
	}
	data = TEE_Malloc(object_info.dataSize, 0);

        /* Effectively retrieves object data*/
	res = TEE_ReadObjectData(object, data, object_info.dataSize,
				 &read_bytes);

        IMSG("Read Object data");
	if (res == TEE_SUCCESS)
	        get_data(data);	
	if (res != TEE_SUCCESS || read_bytes != object_info.dataSize) {
		EMSG("TEE_ReadObjectData failed 0x%08x, read %" PRIu32 " over %u",
				res, read_bytes, object_info.dataSize);
	}

	TEE_CloseObject(object);
        TEE_Free(data);
}

/* 
 * Writes hash table data into persistent memory 
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
        TEE_UUID uuid = TA_LOGGING_UUID; 
        
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
        
        writeIntoMemory(TA_LOGGING_PROOF, strlen(TA_LOGGING_PROOF), hash, hash_sz);

        TEE_Free(hash);
        return TEE_SUCCESS;

}

/* 
 * Deletes the 10 oldest entries registered in the log
 */
static void delete_old_entries(void)
{
        Entry n_log[LOG_MAX_SIZE];
        uint32_t i = 0;
        for(; i < LOG_MAX_SIZE - 10; i++)
            n_log[i] = event_log[i + 10];
        
        for(int j = i; j < LOG_MAX_SIZE; j++) {
            TEE_Free(event_log[j].service);
            TEE_Free(event_log[j].command);
        }
        
        memcpy(event_log, n_log, i);
        size = i;
}

/* 
 * Creates a new log entry and stores it in the log
 */
static TEE_Result log_new_entry(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
				                        TEE_PARAM_TYPE_MEMREF_INPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE);
	
        Entry entry = {NULL, NULL, {0, 0}};
        TEE_Time time;
        uint32_t id_sz;
        uint32_t command_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

        /* Moves parameters values to local variables*/        
        id_sz = params[0].memref.size;
        command_sz = params[1].memref.size;
        TEE_GetSystemTime(&time);
	
        /*Create log entry */
	entry.service = TEE_Malloc(id_sz, 0);
	entry.command = TEE_Malloc(command_sz, 0);
        
        TEE_MemMove(entry.service, params[0].memref.buffer, id_sz);
        TEE_MemMove(entry.command, params[1].memref.buffer, command_sz);
        entry.time = time;
        	
        if(size >= LOG_MAX_SIZE)
            delete_old_entries();

        IMSG("Enter new log entry");
        event_log[size++] = entry;
        dirty = true; 
        return TEE_SUCCESS;
}

/* 
 * Retrieves the in memory and sends it to the Normal World
 */
static TEE_Result log_read_data(uint32_t param_types, TEE_Param params[4])
{
	const uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
				                        TEE_PARAM_TYPE_NONE,
				                        TEE_PARAM_TYPE_NONE,
                                                        TEE_PARAM_TYPE_NONE);

        char data[LOG_DATA_MAX_LENGTH];	
	size_t data_sz;

	/*
	 * Safely get the invocation parameters
	 */
	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

        /* Retrieves data in the form of a string*/        
        IMSG("Retrieving log stored in memory");
        export_data(data);
        data_sz = strlen(data);

        /* Sends string to application*/
	TEE_MemMove(params[0].memref.buffer, data, data_sz);
	params[0].memref.size = data_sz;
        
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
        const char* obj_id = TA_LOGGING_PROOF;
        uint32_t obj_id_sz = strlen(TA_LOGGING_PROOF);
        
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

    IMSG("Loading log");    
    read_log();
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
        IMSG("Closing Session");
        if (dirty) {
	    char data[LOG_DATA_MAX_LENGTH];
	    size_t data_sz;

            /* Fills object data*/
            generate_data(data);
            data_sz = strlen(data);

            /* Generates a string with the information of the log array in memory, 
             * and the saves it in OPTEE private storage*/
            writeIntoMemory(TA_LOGGING_STRING, strlen(TA_LOGGING_STRING), data, data_sz);
        }       
	
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
	case TA_LOGGING_NEW_ENTRY:
		return log_new_entry(param_types, params);
	case TA_LOGGING_READ_DATA:
		return log_read_data(param_types, params);
	case TA_LOGGING_GET_PROOF:
		return get_attestation_proof(param_types, params);
	default:
                EMSG("Command ID 0x%x is not supported", command);
                return TEE_ERROR_NOT_SUPPORTED;
	}
}
