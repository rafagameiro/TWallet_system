/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef TA_ADAPTATION_LAYER_H
#define TA_ADAPTATION_LAYER_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_ADAPTATION_LAYER_UUID \
	{ 0xb8f6f08f, 0xff67, 0x4e71, \
		{ 0x81, 0x63, 0xc3, 0xc8, 0x2e, 0xa1, 0xd6, 0xb4} }

/* UUID used for storing the proof*/
#define TA_ADAPTATION_LAYER_PROOF \
	"b8f6f08f-ff67-4e71-8163-c3c82ea1d6b4-proof"

/* Additional Constants used in TA functions*/
#define SEGMENT_MAX_LENGTH              30 

/* The function IDs implemented in this TA */

/*
* AL_STORE_CREDENTIALS - Create and store a credentials entry
* param[0] (memref) ID used to identify the program that requested the store operation
* param[1] (memref) Credentials stored
* param[2] unused
* param[3] unused
*/
#define AL_STORE_CREDENTIALS               		0

/*
* AL_LOAD_CREDENTIALS - Load a credentials entry
* param[0] (memref) ID used to identify the program that requested the store operation
* param[1] (memref) Credentials stored
* param[2] unused
* param[3] unused
*/
#define AL_LOAD_CREDENTIALS		                1

/*
* TA_AUTH_SERVICE_DELETE_CREDENTIALS - Deletes a credentials entry
* param[0] (memref) ID used to identify the program that requested the store operation
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define AL_DELETE_CREDENTIALS           		2

/*
 * AL_STORAGE_READ_DATA - Retrieve data from a secure storage entry
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Content the persistent object
 * param[2] unused
 * param[3] unused
 */
#define AL_STORAGE_READ_DATA		                3

/*
 * AL_STORAGE_WRITE_DATA - Create and a secure storage entry
 * param[0] (memref) ID used the identify the persistent object
 * param[1] (memref) Data to be writen in the persistent object
 * param[2] unused
 * param[3] unused
 */
#define AL_STORAGE_WRITE_DATA		                4

/*
 * AL_STORAGE_DELETE_DATA - Delete a persistent object
 * param[0] (memref) ID used the identify the persistent object
 * param[1] unused
 * param[2] unused
 * param[3] unused
 */
#define AL_STORAGE_DELETE_DATA          		5

/*
* AL_LOGGING_READ_DATA - Sends the stored log to the application side
* param[0] (memref) log stored in the component to be sent
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define AL_LOGGING_READ_DATA		                6

/*
* AL_TRIGGER_MONITORING - Locks or unlocks access to the secure components operations
* param[0] (value) boolean to trigger or not the monitoring filter
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define AL_TRIGGER_MONITORING   	                7

/*
* TA_ADAPTATION_LAYER_GET_PROOF - Generates an attestation proof to be send to the attestation service
* param[0] (memref) attestation proof to be send back
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_ADAPTATION_LAYER_GET_PROOF 	                8

#endif /*TA_ADAPTATION_LAYER_H*/
