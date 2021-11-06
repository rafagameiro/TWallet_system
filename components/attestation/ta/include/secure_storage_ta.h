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

/* The function IDs implemented in this TA */

/*
* TA_SECURE_STORAGE_GET_PROOF - Generates an attestation proof to be send to the attestation service
* param[0] (memref) attestation proof to be send back
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_SECURE_STORAGE_GET_PROOF 	        3

#endif /* __SECURE_STORAGE_H__ */
