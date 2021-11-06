/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef TA_ATTESTATION_H
#define TA_ATTESTATION_H

/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_ATTESTATION_UUID \
	{ 0xed5a424e, 0xbeef, 0x4431, \
		{ 0x93, 0xbe, 0xfe, 0x84, 0x18, 0xd9, 0xa8, 0xb9} }


#define ATTESTATION_KEYS_ID \
	"ed5a424e-beef-4431-93be-fe8418d9a8b9-keys"


/* Additional Constants used in TA functions*/
#define PUBKEY_EXP_LENGTH                               20 
#define BASE64_MAX_LENGTH                               1000 

/* The function IDs implemented in this TA */

/*
* TA_ATTESTATION_GET_PROOF - Generates and sends back an attestation proof of all secure components
* param[0] (value) Nonce send from the Normal World
* param[1] (memref) Attestation proof
* param[2] unused
* param[3] unused
*/
#define TA_ATTESTATION_GET_PROOF               		0

#endif /*TA_ATTESTATION_H*/
