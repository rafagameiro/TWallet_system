/*
 * Copyright (c) 2016-2017, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
#define HASH_MAX_LENGTH                                 50 
#define DIGEST_MAX_LENGTH                               100 
#define PROOF_MAX_LENGTH                                256 
#define PUBKEY_EXP_LENGTH                               20 
#define PUBKEY_MOD_LENGTH                               256 
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
