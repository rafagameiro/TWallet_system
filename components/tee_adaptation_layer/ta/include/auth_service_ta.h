/*
 *
 * Thesis: TWallet - ARM TrustZone Enabled Trustable Mobile Wallet: A Case for Cryptocurrency Wallets
 * NOVA School of Science and Technology, NOVA University of Lisbon
 *
 * Student: Rafael Rodrigues Gameiro nº50677
 * Advisor: Henrique João Domingues
 *
 */
#ifndef TA_AUTH_SERVICE_H
#define TA_AUTH_SERVICE_H


/*
 * This UUID is generated with uuidgen
 * the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html
 */
#define TA_AUTH_SERVICE_UUID \
	{ 0x105c9ddb, 0x730f, 0x440c, \
		{ 0x98, 0x9e, 0x1c, 0x06, 0xb1, 0xf4, 0x06, 0x6e} }

/* The function IDs implemented in this TA */

/*
* TA_AUTH_SERVICE_STORE_CREDENTIALS - Create and store a credentials entry
* param[0] (memref) ID used to identify the program that requested the store operation
* param[1] (memref) Credentials stored
* param[2] unused
* param[3] unused
*/
#define TA_AUTH_SERVICE_STORE_CREDENTIALS		0

/*
* TA_AUTH_SERVICE_LOAD_CREDENTIALS - Load a credentials entry
* param[0] (memref) ID used to identify the program that requested the store operation
* param[1] (memref) Credentials stored
* param[2] unused
* param[3] unused
*/
#define TA_AUTH_SERVICE_LOAD_CREDENTIALS		1

/*
* TA_AUTH_SERVICE_DELETE_CREDENTIALS - Deletes a credentials entry
* param[0] (memref) ID used to identify the program that requested the store operation
* param[1] unused
* param[2] unused
* param[3] unused
*/
#define TA_AUTH_SERVICE_DELETE_CREDENTIALS		2

#endif /*TA_AUTH_SERVICE_H*/
