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

/* The function IDs implemented in this TA */

/*
* TA_LOGGING_MONITORING_FILTER_OP - Filters and analyses operations 
* requested by applications to the components
* param[0] (memref) ID of the service used to execute the operation
* param[1] (memref) operation executed
* param[2] unused
* param[3] unused
*/
#define TA_MONITORING_FILTER_OP		                0

/*
* TA_LOGGING_MONITORING_TRIGGER_FILTER - Locks or unlocks access to the secure components operations
* param[0] (value) boolean to trigger or not the monitoring filter
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_MONITORING_TRIGGER_FILTER	                1

#endif /*TA_LOGGING_H*/
