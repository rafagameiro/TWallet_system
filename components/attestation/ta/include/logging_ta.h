/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef TA_LOGGING_H
#define TA_LOGGING_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_LOGGING_UUID \
	{ 0x31b125d8, 0xfae8, 0x4e9e, \
		{ 0x82, 0xdc, 0x30, 0x64, 0x22, 0x53, 0x59, 0x03} }

/* The function IDs implemented in this TA */

/*
* TA_LOGGING_GET_PROOF - Generates an attestation proof to be send to the attestation service
* param[0] (memref) attestation proof to be send back
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_LOGGING_GET_PROOF     	                2

#endif /*TA_LOGGING_H*/
