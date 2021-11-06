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

/* UUID set in String*/
#define TA_SECURE_STORAGE_STRING \
	"57ebb946-a4c5-496b-93fc-273f65b30fb1"

/* UUID used for storing the proof*/
#define TA_SECURE_STORAGE_PROOF \
	"57ebb946-a4c5-496b-93fc-273f65b30fb1-proof"

/* Additional Constants used in TA functions*/
#define SEGMENT_MAX_LENGTH              30 

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

/*
* TA_SECURE_STORAGE_GET_PROOF - Generates an attestation proof to be send to the attestation service
* param[0] (memref) attestation proof to be send back
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_SECURE_STORAGE_GET_PROOF 	        3

#endif /* __SECURE_STORAGE_H__ */
