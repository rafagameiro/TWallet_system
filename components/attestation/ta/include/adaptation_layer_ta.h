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

/* The function IDs implemented in this TA */

/*
* TA_ADAPTATION_LAYER_GET_PROOF - Generates an attestation proof to be send to the attestation service
* param[0] (memref) attestation proof to be send back
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_ADAPTATION_LAYER_GET_PROOF 	                8

#endif /*TA_ADAPTATION_LAYER_H*/
