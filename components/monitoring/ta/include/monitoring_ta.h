/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef TA_MONITORING_H
#define TA_MONITORING_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_MONITORING_UUID \
	{ 0x9e5e8e3a, 0xeb89, 0x4e66, \
		{ 0xa4, 0xfe, 0xad, 0x40, 0x84, 0xbc, 0x1b, 0x93} }

/* UUID used for storing the proof*/
#define TA_MONITORING_PROOF \
	"9e5e8e3a-eb89-4e66-a4fe-ad4084bc1b93-proof"

/* Additional Constants used in TA functions*/
#define COMPONENT_MAX_NUM               3
#define SEGMENT_MAX_LENGTH              30 

/* The function IDs implemented in this TA */

/*
* TA_MONITORING_FILTER_OP - Filters and analyses operations 
* requested by applications to the components
* param[0] (memref) ID of the service used to execute the operation
* param[1] (memref) operation executed
* param[2] unused
* param[3] unused
*/
#define TA_MONITORING_FILTER_OP		                0

/*
* TA_MONITORING_TRIGGER_FILTER - Locks or unlocks access to the secure components operations
* param[0] (value) boolean to trigger or not the monitoring filter
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_MONITORING_TRIGGER_FILTER	                1

/*
* TA_MONITORING_GET_PROOF - Generates an attestation proof to be send to the attestation service
* param[0] (memref) attestation proof to be send back
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_MONITORING_GET_PROOF 	                2

#endif /*TA_LOGGING_H*/
