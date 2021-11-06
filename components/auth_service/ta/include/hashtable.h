/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef HASH_TABLE_H
#define HASH_TABLE_H

#define SIZE 300
#define KEYS_MAX_LENGTH 1000
#define NO_KEYS "(empty)"

typedef struct {
   char* value;   
   char* key;
} Item;

char* put(char* key, char* value);

Item* findItem(char* key);

Item* removeItem(char* key);

void itemKeys(char* ptr);

void clear(void);

#endif /* HASH_TABLE_H */
