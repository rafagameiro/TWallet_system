/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef __SECURE_STORAGE_H__
#define __SECURE_STORAGE_H__

/* UUID of the trusted application */
#define TA_SECURE_STORAGE_UUID \
		{ 0x57ebb946, 0xa4c5, 0x496b, \
			{ 0x93, 0xfc, 0x27, 0x3f, 0x65, 0xb3, 0x0f, 0xb1 } }
/*
 * TA_SECURE_STORAGE_READ_DATA - Retrieve data from a secure storage entry
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Content the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_READ_DATA		0

/*
 * TA_SECURE_STORAGE_WRITE_DATA - Create and a secure storage entry
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Data to be writen in the persistent object
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_WRITE_DATA		1

/*
 * TA_SECURE_STORAGE_DELETE_DATA - Delete a persistent object
 * param[0] (memref) ID used the identify the persistent object
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define TA_SECURE_STORAGE_DELETE_DATA		2

#endif /* __SECURE_STORAGE_H__ */
