/*
 * zigbee_crypt.c
 * Copyright 2011 steiner <steiner@localhost.localdomain>
 * zigbee convenience functions
 * 
 * alot of this code was "borrowed" from wireshark
 * packet-zbee-security.c & pzcket-zbee-security.h
 * function: zbee_sec_ccm_decrypt
 */
 
// Explaination of Python Build Values http://docs.python.org/c-api/arg.html#Py_BuildValue

#include <Python.h>
#include <stdio.h>
#include <gcrypt.h>
#include "zigbee_crypt.h"

static PyObject *zigbee_crypt_encrypt_ccm(PyObject *self, PyObject *args) {
	// This was modeled after zigbee_crypt_decrypt_ccm in reverse
	const char			*pZkey;
	int					sizeZkey;
	const char			*pNonce;
	int					sizeNonce;
	int					sizeMIC;
	const char			*pUnencryptedData;
	int					sizeUnencryptedData;
	const char			*zigbeeData;
	int					sizeZigbeeData;
	int i, j;

	char				pMIC[ZBEE_SEC_CONST_MICSIZE];
	char				pEncMIC[ZBEE_SEC_CONST_MICSIZE];
	char				*pEncrypted;
	char				cipher_in[ZBEE_SEC_CONST_BLOCKSIZE];
	char				cipher_out[ZBEE_SEC_CONST_BLOCKSIZE];
	/* Cipher Instance. */
	gcry_cipher_hd_t	cipher_hd;
	
	if (!PyArg_ParseTuple(args, "s#s#is#s#",
								&pZkey, &sizeZkey,
								&pNonce, &sizeNonce,
								&sizeMIC,
								&pUnencryptedData, &sizeUnencryptedData,
								&zigbeeData, &sizeZigbeeData)) {
								return NULL;
	}
	if (sizeZkey != ZBEE_SEC_CONST_KEYSIZE) {
		PyErr_SetString(PyExc_ValueError, "incorrect key size (must be 16)");
		return NULL;
	}
	
	if (sizeNonce != ZBEE_SEC_CONST_NONCE_LEN) {
		PyErr_SetString(PyExc_ValueError, "incorrect nonce size (must be 13)");
		return NULL;
	}
	
	if ((sizeMIC != 0) && (sizeMIC != 4) && (sizeMIC != 8) && (sizeMIC != 16)) {
		PyErr_SetString(PyExc_ValueError, "incorrect mic size (must be 0, 4, 8, or 16 bytes)");
		return NULL;
	}

	memset(pMIC, 0, ZBEE_SEC_CONST_MICSIZE);	// set both mics to 0
	memset(pEncMIC, 0, ZBEE_SEC_CONST_MICSIZE);
	pEncrypted = malloc(sizeUnencryptedData);
	memset(pEncrypted, 0, sizeUnencryptedData);
	
	/* Open the cipher in ECB mode. */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 ECB cipher failed");
		return NULL;
	}
	/* Load the key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Generate the first cipher block B0. */
	cipher_in[0] = ZBEE_SEC_CCM_FLAG_M(sizeMIC) | ((sizeZigbeeData>0)?0x40:0x00) | ZBEE_SEC_CCM_FLAG_L;
	memcpy(cipher_in + 1, pNonce, ZBEE_SEC_CONST_NONCE_LEN);
	for (i = 0; i < ZBEE_SEC_CONST_L; i++) {
		cipher_in[(ZBEE_SEC_CONST_BLOCKSIZE-1)-i] = (sizeUnencryptedData >> (8*i)) & 0xff;
	}
	/* Generate the first cipher block, X1 = E(Key, 0^128 XOR B0). */
	if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "mic creation failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	
	j = 0;
	if (sizeZigbeeData > 0) {
		/* Process L(a) into the cipher block. */
		cipher_in[j] = cipher_out[j] ^ ((sizeZigbeeData >> 8) & 0xff);
		j++;
		cipher_in[j] = cipher_out[j] ^ ((sizeZigbeeData >> 0) & 0xff);
		j++;
		/* Process a into the cipher block. */
		for (i = 0; i < sizeZigbeeData; i++, j++) {
			if ( j >= ZBEE_SEC_CONST_BLOCKSIZE) {
				/* Generate the next cipher block. */
				if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
					PyErr_SetString(PyExc_Exception, "mic creation failed");
					gcry_cipher_close(cipher_hd);
					return NULL;
				}
				/* Reset j to point back to the start of the new cipher block. */
				j = 0;
			}
			/* Cipher in = cipher_out ^ a */
			cipher_in[j] = cipher_out[j] ^ zigbeeData[i];
		} /* for */
		/* Process padding into the cipher block. */
		for (; j < ZBEE_SEC_CONST_BLOCKSIZE; j++)
			cipher_in[j] = cipher_out[j];
	}
	/* Process m into the cipher block. */
	for (i = 0; i < sizeUnencryptedData; i++, j++) {
		if (j >= ZBEE_SEC_CONST_BLOCKSIZE) {
			/* Generate the next cipher block. */
			if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
				PyErr_SetString(PyExc_Exception, "mic creation failed");
				gcry_cipher_close(cipher_hd);
				return NULL;
			}
			/* Reset j to point back to the start of the new cipher block. */
			j = 0;
		}
		/* Cipher in = cipher out ^ m */
		cipher_in[j] = cipher_out[j] ^ pUnencryptedData[i];
	} /* for */
	/* Padding. */
	for (; j < ZBEE_SEC_CONST_BLOCKSIZE; j++) {
		cipher_in[j] = cipher_out[j];
	}
	/* Generate the last cipher block, which will be the MIC tag. */
	if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "mic creation failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	
	gcry_cipher_close(cipher_hd);
	memcpy(pMIC, cipher_out, sizeMIC);
	
	/* Create the CCM* counter block A0 */
	memset(cipher_in, 0, ZBEE_SEC_CONST_BLOCKSIZE);
	cipher_in[0] = ZBEE_SEC_CCM_FLAG_L;
	memcpy(cipher_in + 1, pNonce, ZBEE_SEC_CONST_NONCE_LEN);

	if (pEncrypted == NULL) {
		PyErr_SetString(PyExc_MemoryError, "could not allocate memory");
		return NULL;
	}
	/*
	 * The encryption/decryption process of CCM* works in CTR mode. Open a CTR
	 * mode cipher for this phase. NOTE: The 'counter' part of the CCM* counter
	 * block is the last two bytes, and is big-endian.
	 */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CTR, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 CTR cipher failed");
		return NULL;
	}
	/* Re-load the Key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Set the counter. */
	if (gcry_cipher_setctr(cipher_hd, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the counter failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pEncMIC, ZBEE_SEC_CONST_MICSIZE, pMIC, ZBEE_SEC_CONST_MICSIZE)) {
		PyErr_SetString(PyExc_Exception, "encryption of the mic failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pEncrypted, sizeUnencryptedData, pUnencryptedData, sizeUnencryptedData)) {
		PyErr_SetString(PyExc_Exception, "encryption of the payload failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Done with the CTR Cipher. */
	gcry_cipher_close(cipher_hd);
	
	return Py_BuildValue("(s#s#)", pEncrypted, sizeUnencryptedData, pEncMIC, sizeMIC);
};

static PyObject *zigbee_crypt_decrypt_ccm(PyObject *self, PyObject *args) {
	const char			*pZkey;
	int					sizeZkey;
	const char			*pNonce;
	int					sizeNonce;
	const char			*pOldMIC;
	int					sizeMIC;
	const char			*pEncryptedData;
	int					sizeEncryptedData;
	const char			*zigbeeData;
	int					sizeZigbeeData;

	char				pMIC[ZBEE_SEC_CONST_MICSIZE];
	char				pUnencMIC[ZBEE_SEC_CONST_MICSIZE];
	char				*pUnencrypted;
	char				cipher_in[ZBEE_SEC_CONST_BLOCKSIZE];
	char				cipher_out[ZBEE_SEC_CONST_BLOCKSIZE];
	if (!PyArg_ParseTuple(args, "s#s#s#s#s#",
								&pZkey, &sizeZkey,
								&pNonce, &sizeNonce,
								&pOldMIC, &sizeMIC,
								&pEncryptedData, &sizeEncryptedData,
								&zigbeeData, &sizeZigbeeData)) {
								return NULL;
	}
	if (sizeZkey != ZBEE_SEC_CONST_KEYSIZE) {
		PyErr_SetString(PyExc_ValueError, "incorrect key size (must be 16)");
		return NULL;
	}
	
	if (sizeNonce != ZBEE_SEC_CONST_NONCE_LEN) {
		PyErr_SetString(PyExc_ValueError, "incorrect nonce size (must be 13)");
		return NULL;
	}

	if (sizeMIC > 16) {
		PyErr_SetString(PyExc_ValueError, "incorrect mic size (must be between 0 and 16)");
		return NULL;
	}

	memset(pMIC, 0, ZBEE_SEC_CONST_MICSIZE);
	memcpy(pMIC, pOldMIC, sizeMIC);
	
	memset(pUnencMIC, 0, ZBEE_SEC_CONST_MICSIZE);
	
	pUnencrypted = malloc(sizeEncryptedData);
	memset(pUnencrypted, 0, sizeEncryptedData);
	
	/* Create the CCM* counter block A0 */
	memset(cipher_in, 0, ZBEE_SEC_CONST_BLOCKSIZE);
	cipher_in[0] = ZBEE_SEC_CCM_FLAG_L;
	memcpy(cipher_in + 1, pNonce, ZBEE_SEC_CONST_NONCE_LEN);

	if (pUnencrypted == NULL) {
		PyErr_SetString(PyExc_MemoryError, "could not allocate memory");
		return NULL;
	}

	/* Cipher Instance. */
	gcry_cipher_hd_t	cipher_hd;

	/*
	 * The encryption/decryption process of CCM* works in CTR mode. Open a CTR
	 * mode cipher for this phase. NOTE: The 'counter' part of the CCM* counter
	 * block is the last two bytes, and is big-endian.
	 */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CTR, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 CTR cipher failed");
		return NULL;
	}
	/* Set the Key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Set the counter. */
	if (gcry_cipher_setctr(cipher_hd, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the counter failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pUnencMIC, ZBEE_SEC_CONST_MICSIZE, pMIC, ZBEE_SEC_CONST_MICSIZE)) {
		PyErr_SetString(PyExc_Exception, "decryption of the mic failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pUnencrypted, sizeEncryptedData, pEncryptedData, sizeEncryptedData)) {
		PyErr_SetString(PyExc_Exception, "decryption of the payload failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Done with the CTR Cipher. */
	gcry_cipher_close(cipher_hd);
	
	int i, j;
	/* Re-open the cipher in ECB mode. */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 ECB cipher failed");
		return NULL;
	}
	/* Re-load the key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/* Generate the first cipher block B0. */
	cipher_in[0] = ZBEE_SEC_CCM_FLAG_M(sizeMIC) | ((sizeZigbeeData>0)?0x40:0x00) | ZBEE_SEC_CCM_FLAG_L;
	memcpy(cipher_in + 1, pNonce, ZBEE_SEC_CONST_NONCE_LEN);
	for (i = 0; i < ZBEE_SEC_CONST_L; i++) {
		cipher_in[(ZBEE_SEC_CONST_BLOCKSIZE-1)-i] = (sizeEncryptedData >> (8*i)) & 0xff;
	}
	/* Generate the first cipher block, X1 = E(Key, 0^128 XOR B0). */
	if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "mic verification failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	/*
	 * We avoid mallocing() big chunks of memory by recycling small stack
	 * buffers for the encryption process. Throughout this process, j is always
	 * pointed to the position within the current buffer.
	 */
	j = 0;
	/* AuthData = L(a) || a || Padding || m || Padding
	 * Where L(a) =
	 *	  - an empty string if l(a) == 0.
	 *	  - 2-octet encoding of l(a) if 0 < l(a) < (2^16 - 2^8)
	 *	  - 0xff || 0xfe || 4-octet encoding of l(a) if (2^16 - 2^8) <= l(a) < 2^32
	 *	  - 0xff || 0xff || 8-octet encoding of l(a)
	 * But for ZigBee, the largest packet size we should ever see is 2^7, so we
	 * are only really concerned with the first two cases.
	 *
	 * To generate the MIC tag CCM* operates similar to CBC-MAC mode. Each block
	 * of AuthData is XOR'd with the last block of cipher output to produce the
	 * next block of cipher output. Padding sections have the minimum non-negative
	 * length such that the padding ends on a block boundary. Padded bytes are 0.
	 */
	if (sizeZigbeeData > 0) {
		/* Process L(a) into the cipher block. */
		cipher_in[j] = cipher_out[j] ^ ((sizeZigbeeData >> 8) & 0xff);
		j++;
		cipher_in[j] = cipher_out[j] ^ ((sizeZigbeeData >> 0) & 0xff);
		j++;
		/* Process a into the cipher block. */
		for (i = 0; i < sizeZigbeeData; i++, j++) {
			if (j >= ZBEE_SEC_CONST_BLOCKSIZE) {
				/* Generate the next cipher block. */
				if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
					PyErr_SetString(PyExc_Exception, "mic verification failed");
					gcry_cipher_close(cipher_hd);
					return NULL;
				}
				/* Reset j to point back to the start of the new cipher block. */
				j = 0;
			}
			/* Cipher in = cipher_out ^ a */
			cipher_in[j] = cipher_out[j] ^ zigbeeData[i];
		} /* for */
		/* Process padding into the cipher block. */
		for (; j<ZBEE_SEC_CONST_BLOCKSIZE; j++)
			cipher_in[j] = cipher_out[j];
	}
	/* Process m into the cipher block. */
	for (i = 0; i < sizeEncryptedData; i++, j++) {
		if (j >= ZBEE_SEC_CONST_BLOCKSIZE) {
			/* Generate the next cipher block. */
			if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
				PyErr_SetString(PyExc_Exception, "mic verification failed");
				gcry_cipher_close(cipher_hd);
				return NULL;
			}
			/* Reset j to point back to the start of the new cipher block. */
			j = 0;
		}
		/* Cipher in = cipher out ^ m */
		cipher_in[j] = cipher_out[j] ^ pUnencrypted[i];
	} /* for */
	/* Padding. */
	for (; j < ZBEE_SEC_CONST_BLOCKSIZE; j++) {
		cipher_in[j] = cipher_out[j];
	}
	/* Generate the last cipher block, which will be the MIC tag. */
	if (gcry_cipher_encrypt(cipher_hd, cipher_out, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "mic verification failed");
		gcry_cipher_close(cipher_hd);
		return NULL;
	}
	
	gcry_cipher_close(cipher_hd);
	
	// now use j to indicate whether the MICs match
	j = 0;
	if (memcmp(cipher_out, pUnencMIC, sizeMIC) == 0) {
		j = 1;
	}
	
	return Py_BuildValue("(s#i)", pUnencrypted, sizeEncryptedData, j);
};

static PyMethodDef zigbee_crypt_Methods[] = {
	{ "decrypt_ccm", zigbee_crypt_decrypt_ccm, METH_VARARGS, "decrypt_ccm(key, nonce, mic, encrypted_payload, zigbee_data)\nDecrypt data with a 0, 32, 64, or 128-bit MIC\n\n@type key: String\n@param key: 16-byte decryption key\n@type nonce: String\n@param nonce: 13-byte nonce\n@type mic: String\n@param mic: 4-16 byte message integrity check (MIC)\n@type encrypted_payload: String\n@param encrypted_payload: The encrypted data to decrypt\n@type zigbee_data: String\n@param zigbee_data: The zigbee data within the frame, without the encrypted payload, MIC, or FCS" },
	{ "encrypt_ccm", zigbee_crypt_encrypt_ccm, METH_VARARGS, "encrypt_ccm(key, nonce, mic_size, decrypted_payload, zigbee_data)\nEncrypt data with a 0, 32, 64, or 128-bit MIC\n\n@type key: String\n@param key: 16-byte decryption key\n@type nonce: String\n@param nonce: 13-byte nonce\n@type mic_size: Integer\n@param mic_size: the size in bytes of the desired MIC\n@type decrypted_payload: String\n@param decrypted_payload: The decrypted data to encrypt\n@type zigbee_data: String\n@param zigbee_data: The zigbee data within the frame, without the decrypted payload, MIC or FCS" },
	{ NULL, NULL, 0, NULL },
};

PyMODINIT_FUNC initzigbee_crypt(void) {
	(void) Py_InitModule("zigbee_crypt", zigbee_crypt_Methods);
}

int main(int argc, char *argv[]) {
	Py_SetProgramName(argv[0]);
	Py_Initialize();
	initzigbee_crypt();
	return 0;
}
