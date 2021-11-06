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
#include <string.h>
#include <string_ext.h>
#include <hashtable.h>

Item* hashArray[SIZE]; 

/*
 * Using a key as argument, 
 * generates an correspondent hashcode
 */
static unsigned long hashCode(char* key)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *key++))
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash % SIZE;
}

/*
 * Allocates space in memory to store a new key-value set,
 * and stores it in the data structure
 */
static void insert(char* key, char* value) 
{

   Item* item = (Item*) TEE_Malloc(sizeof(Item), 0);
   item->key = TEE_Malloc(strlen(key), 0);
   item->value = TEE_Malloc(strlen(value), 0);
   TEE_MemMove(item->key, key, strlen(key));
   TEE_MemMove(item->value, value, strlen(value));

   //get hash code
   int hashIndex = hashCode(key);
   
   //move in array until an empty entry
   while(hashArray[hashIndex] != NULL) {
      
      //go to next cell
      hashIndex++;
      hashIndex %= SIZE;
   }
	
   hashArray[hashIndex] = item;
}

/*
 * Searches through the list of stored key-value entries
 * and retrieves an entry that has the key passed as argument
 */
Item* findItem(char* key) 
{
   //get hash code
   int hashIndex = hashCode(key);  
	
   //move in array until an empty entry 
   while(hashArray[hashIndex] != NULL) {
      if(strcmp(hashArray[hashIndex]->key, key) == 0)
         return hashArray[hashIndex]; 
    
      //go to next cell
      hashIndex++;
      hashIndex %= SIZE;
   }        
	
   return NULL;        
}

/*
 * Tries to insert a new key-value entry into the datastructure
 * If the data structure already possess an entry 
 * with the same key as the one passed as argument,
 * it will instead update the stored value and retrived the newly added value 
 */
char* put(char* key, char* value)
{
    Item* item = findItem(key);
    if(item != NULL) {
        TEE_MemMove(item->value, value, strlen(value));
        return value;
    }
    
    insert(key, value);
    return NULL;
}

/*
 * Removes a key-value entry from the datastructure
 */
Item* removeItem(char* key) 
{

    //get hash code
    int hashIndex = hashCode(key);

    //move in array until an empty entry
    while(hashArray[hashIndex] != NULL) {
        if(strcmp(hashArray[hashIndex]->key, key) == 0) {
            
            Item* temp = (Item*) TEE_Malloc(sizeof(Item), 0);
            temp->key = TEE_Malloc(strlen(hashArray[hashIndex]->key), 0);
            temp->value = TEE_Malloc(strlen(hashArray[hashIndex]->value), 0);
            TEE_MemMove(temp->key, hashArray[hashIndex]->key, strlen(hashArray[hashIndex]->key));
            TEE_MemMove(temp->value, hashArray[hashIndex]->value, strlen(hashArray[hashIndex]->value));
			
            hashArray[hashIndex] = NULL; 
            return temp;
        }
		
        //go to next cell
        hashIndex++;
        hashIndex %= SIZE;
    }      
	
    return NULL;        
}

/*
 * Cycles through all entries stored in the data structure
 * and retrieves a string that has all entries' keys
 */
void itemKeys(char* ptr) 
{
    bool first = true;
    int i = 0;
	
    for(i = 0; i < SIZE; i++)
        if(hashArray[i] != NULL) {
            if(first) {
                strlcpy(ptr, hashArray[i]->key, KEYS_MAX_LENGTH);
                first = false;
            } else {
                strlcat(ptr, "|", KEYS_MAX_LENGTH);
                strlcat(ptr, hashArray[i]->key, KEYS_MAX_LENGTH);
            }
        }

    if(strlen(ptr) == 0)
        strlcpy(ptr, NO_KEYS, KEYS_MAX_LENGTH);
    strlcat(ptr, "\0", KEYS_MAX_LENGTH);
}

/*
 * Completely empties the datastructure
 */
void clear(void)
{
    for(int i = 0; i < SIZE; i++)
        if(hashArray[i] != NULL) {
            TEE_Free(hashArray[i]->key);
            TEE_Free(hashArray[i]->value);
            TEE_Free(hashArray[i]);
            hashArray[i] = NULL;
        }

}
