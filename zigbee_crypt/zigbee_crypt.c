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
	PyObject			*res;

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
	if (pEncrypted == NULL) {
		PyErr_SetString(PyExc_MemoryError, "could not allocate memory");
		return NULL;
	}

	memset(pEncrypted, 0, sizeUnencryptedData);

	/* Open the cipher in ECB mode. */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 ECB cipher failed");
		free(pEncrypted);
		return NULL;
	}
	/* Load the key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		free(pEncrypted);
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
		free(pEncrypted);
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
					free(pEncrypted);
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
				free(pEncrypted);
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
		free(pEncrypted);
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
		free(pEncrypted);
		return NULL;
	}
	/*
	 * The encryption/decryption process of CCM* works in CTR mode. Open a CTR
	 * mode cipher for this phase. NOTE: The 'counter' part of the CCM* counter
	 * block is the last two bytes, and is big-endian.
	 */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CTR, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 CTR cipher failed");
		free(pEncrypted);
		return NULL;
	}
	/* Re-load the Key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		free(pEncrypted);
		return NULL;
	}
	/* Set the counter. */
	if (gcry_cipher_setctr(cipher_hd, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the counter failed");
		gcry_cipher_close(cipher_hd);
		free(pEncrypted);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pEncMIC, ZBEE_SEC_CONST_MICSIZE, pMIC, ZBEE_SEC_CONST_MICSIZE)) {
		PyErr_SetString(PyExc_Exception, "encryption of the mic failed");
		gcry_cipher_close(cipher_hd);
		free(pEncrypted);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pEncrypted, sizeUnencryptedData, pUnencryptedData, sizeUnencryptedData)) {
		PyErr_SetString(PyExc_Exception, "encryption of the payload failed");
		gcry_cipher_close(cipher_hd);
		free(pEncrypted);
		return NULL;
	}
	/* Done with the CTR Cipher. */
	gcry_cipher_close(cipher_hd);

	res = Py_BuildValue("(s#s#)", pEncrypted, sizeUnencryptedData, pEncMIC, sizeMIC);
	free(pEncrypted);
	return res;
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
	PyObject			*res;

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
	if (pUnencrypted == NULL) {
		PyErr_SetString(PyExc_MemoryError, "could not allocate memory");
		return NULL;
	}

	memset(pUnencrypted, 0, sizeEncryptedData);

	/* Create the CCM* counter block A0 */
	memset(cipher_in, 0, ZBEE_SEC_CONST_BLOCKSIZE);
	cipher_in[0] = ZBEE_SEC_CCM_FLAG_L;
	memcpy(cipher_in + 1, pNonce, ZBEE_SEC_CONST_NONCE_LEN);

	/* Cipher Instance. */
	gcry_cipher_hd_t	cipher_hd;

	/*
	 * The encryption/decryption process of CCM* works in CTR mode. Open a CTR
	 * mode cipher for this phase. NOTE: The 'counter' part of the CCM* counter
	 * block is the last two bytes, and is big-endian.
	 */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_CTR, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 CTR cipher failed");
		free(pUnencrypted);
		return NULL;
	}
	/* Set the Key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		free(pUnencrypted);
		return NULL;
	}
	/* Set the counter. */
	if (gcry_cipher_setctr(cipher_hd, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the counter failed");
		gcry_cipher_close(cipher_hd);
		free(pUnencrypted);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pUnencMIC, ZBEE_SEC_CONST_MICSIZE, pMIC, ZBEE_SEC_CONST_MICSIZE)) {
		PyErr_SetString(PyExc_Exception, "decryption of the mic failed");
		gcry_cipher_close(cipher_hd);
		free(pUnencrypted);
		return NULL;
	}
	/* Encrypt/Decrypt the payload. */
	if (gcry_cipher_encrypt(cipher_hd, pUnencrypted, sizeEncryptedData, pEncryptedData, sizeEncryptedData)) {
		PyErr_SetString(PyExc_Exception, "decryption of the payload failed");
		gcry_cipher_close(cipher_hd);
		free(pUnencrypted);
		return NULL;
	}
	/* Done with the CTR Cipher. */
	gcry_cipher_close(cipher_hd);

	int i, j;
	/* Re-open the cipher in ECB mode. */
	if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, 0)) {
		PyErr_SetString(PyExc_Exception, "gcrypt open AES-128 ECB cipher failed");
		free(pUnencrypted);
		return NULL;
	}
	/* Re-load the key. */
	if (gcry_cipher_setkey(cipher_hd, pZkey, ZBEE_SEC_CONST_KEYSIZE)) {
		PyErr_SetString(PyExc_Exception, "setting the key failed");
		gcry_cipher_close(cipher_hd);
		free(pUnencrypted);
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
		free(pUnencrypted);
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
					free(pUnencrypted);
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
				free(pUnencrypted);
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
		free(pUnencrypted);
		return NULL;
	}

	gcry_cipher_close(cipher_hd);

	// now use j to indicate whether the MICs match
	j = 0;
	if (memcmp(cipher_out, pUnencMIC, sizeMIC) == 0) {
		j = 1;
	}
	res = Py_BuildValue("(s#i)", pUnencrypted, sizeEncryptedData, j);
	free(pUnencrypted);
	return res;
};

/*FUNCTION:------------------------------------------------------
 *  NAME
 *      zbee_sec_hash
 *  DESCRIPTION
 *      ZigBee Cryptographic Hash Function, described in ZigBee
 *      specification sections B.1.3 and B.6.
 *
 *      This is a Matyas-Meyer-Oseas hash function using the AES-128
 *      cipher. We use the ECB mode of libgcrypt to get a raw block
 *      cipher.
 *
 *      Input may be any length, and the output must be exactly 1-block in length.
 *
 *      Implements the function:
 *          Hash(text) = Hash[t];
 *          Hash[0] = 0^(blocksize).
 *          Hash[i] = E(Hash[i-1], M[i]) XOR M[j];
 *          M[i] = i'th block of text, with some padding and flags concatenated.
 *  PARAMETERS
 *      char *    input       - Hash Input (any length).
 *      int       input_len   - Hash Input Length.
 *      char *    output      - Hash Output (exactly one block in length).
 *  RETURNS
 *      void
 *---------------------------------------------------------------
 */
static void
zbee_sec_hash(char *input, int input_len, char *output)
{
    char              cipher_in[ZBEE_SEC_CONST_BLOCKSIZE];
    int               i, j;
    /* Cipher Instance. */
    gcry_cipher_hd_t    cipher_hd;

    /* Clear the first hash block (Hash0). */
    memset(output, 0, ZBEE_SEC_CONST_BLOCKSIZE);
    /* Create the cipher instance in ECB mode. */
    if (gcry_cipher_open(&cipher_hd, GCRY_CIPHER_AES128, GCRY_CIPHER_MODE_ECB, 0)) {
        return; /* Failed. */
    }
    /* Create the subsequent hash blocks using the formula: Hash[i] = E(Hash[i-1], M[i]) XOR M[i]
     *
     * because we can't guarantee that M will be exactly a multiple of the
     * block size, we will need to copy it into local buffers and pad it.
     *
     * Note that we check for the next cipher block at the end of the loop
     * rather than the start. This is so that if the input happens to end
     * on a block boundary, the next cipher block will be generated for the
     * start of the padding to be placed into.
     */
    i = 0;
    j = 0;
    while (i<input_len) {
        /* Copy data into the cipher input. */
        cipher_in[j++] = input[i++];
        /* Check if this cipher block is done. */
        if (j >= ZBEE_SEC_CONST_BLOCKSIZE) {
            /* We have reached the end of this block. Process it with the
             * cipher, note that the Key input to the cipher is actually
             * the previous hash block, which we are keeping in output.
             */
            (void)gcry_cipher_setkey(cipher_hd, output, ZBEE_SEC_CONST_BLOCKSIZE);
            (void)gcry_cipher_encrypt(cipher_hd, output, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE);
            /* Now we have to XOR the input into the hash block. */
            for (j=0;j<ZBEE_SEC_CONST_BLOCKSIZE;j++) output[j] ^= cipher_in[j];
            /* Reset j to start again at the beginning at the next block. */
            j = 0;
        }
    } /* for */
    /* Need to append the bit '1', followed by '0' padding long enough to end
     * the hash input on a block boundary. However, because 'n' is 16, and 'l'
     * will be a multiple of 8, the padding will be >= 7-bits, and we can just
     * append the byte 0x80.
     */
    cipher_in[j++] = 0x80;
    /* Pad with '0' until the the current block is exactly 'n' bits from the
     * end.
     */
    while (j!=(ZBEE_SEC_CONST_BLOCKSIZE-2)) {
        if (j >= ZBEE_SEC_CONST_BLOCKSIZE) {
            /* We have reached the end of this block. Process it with the
             * cipher, note that the Key input to the cipher is actually
             * the previous hash block, which we are keeping in output.
             */
            (void)gcry_cipher_setkey(cipher_hd, output, ZBEE_SEC_CONST_BLOCKSIZE);
            (void)gcry_cipher_encrypt(cipher_hd, output, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE);
            /* Now we have to XOR the input into the hash block. */
            for (j=0;j<ZBEE_SEC_CONST_BLOCKSIZE;j++) output[j] ^= cipher_in[j];
            /* Reset j to start again at the beginning at the next block. */
            j = 0;
        }
        /* Pad the input with 0. */
        cipher_in[j++] = 0x00;
    } /* while */
    /* Add the 'n'-bit representation of 'l' to the end of the block. */
    cipher_in[j++] = ((input_len * 8) >> 8) & 0xff;
    cipher_in[j] = ((input_len * 8) >> 0) & 0xff;
    /* Process the last cipher block. */
    (void)gcry_cipher_setkey(cipher_hd, output, ZBEE_SEC_CONST_BLOCKSIZE);
    (void)gcry_cipher_encrypt(cipher_hd, output, ZBEE_SEC_CONST_BLOCKSIZE, cipher_in, ZBEE_SEC_CONST_BLOCKSIZE);
    /* XOR the last input block back into the cipher output to get the hash. */
    for (j=0;j<ZBEE_SEC_CONST_BLOCKSIZE;j++) output[j] ^= cipher_in[j];
    /* Cleanup the cipher. */
    gcry_cipher_close(cipher_hd);
    /* Done */
} /* zbee_sec_hash */

/*FUNCTION:------------------------------------------------------
 *  NAME
 *      zbee_sec_key_hash
 *  DESCRIPTION
 *      ZigBee Keyed Hash Function. Described in ZigBee specification
 *      section B.1.4, and in FIPS Publication 198. Strictly speaking
 *      there is nothing about the Keyed Hash Function which restricts
 *      it to only a single byte input, but that's all ZigBee ever uses.
 *
 *      This function implements the hash function:
 *          Hash(Key, text) = H((Key XOR opad) || H((Key XOR ipad) || text));
 *          ipad = 0x36 repeated.
 *          opad = 0x5c repeated.
 *          H() = ZigBee Cryptographic Hash (B.1.3 and B.6).
 *---------------------------------------------------------------
 */
static PyObject *zigbee_sec_key_hash(PyObject *self, PyObject *args) {
	const char			*key;
	int					sizeKey;
	const char           input;

    char          hash_in[2*ZBEE_SEC_CONST_BLOCKSIZE];
    char          hash_out[ZBEE_SEC_CONST_BLOCKSIZE+1];
    int                 i;
    static const char ipad = 0x36;
    static const char opad = 0x5c;

	if (!PyArg_ParseTuple(args, "s#c", &key, &sizeKey, &input)) {
		return NULL;
	}

    /* Copy the key into hash_in and XOR with opad to form: (Key XOR opad) */
    for (i=0; i<ZBEE_SEC_CONST_KEYSIZE; i++) hash_in[i] = key[i] ^ opad;
    /* Copy the Key into hash_out and XOR with ipad to form: (Key XOR ipad) */
    for (i=0; i<ZBEE_SEC_CONST_KEYSIZE; i++) hash_out[i] = key[i] ^ ipad;
    /* Append the input byte to form: (Key XOR ipad) || text. */
    hash_out[ZBEE_SEC_CONST_BLOCKSIZE] = input;
    /* Hash the contents of hash_out and append the contents to hash_in to
     * form: (Key XOR opad) || H((Key XOR ipad) || text).
     */
    zbee_sec_hash(hash_out, ZBEE_SEC_CONST_BLOCKSIZE+1, hash_in+ZBEE_SEC_CONST_BLOCKSIZE);
    /* Hash the contents of hash_in to get the final result. */
    zbee_sec_hash(hash_in, 2*ZBEE_SEC_CONST_BLOCKSIZE, hash_out);

	return Py_BuildValue("s", hash_out);
} /* zbee_sec_key_hash */



static PyMethodDef zigbee_crypt_Methods[] = {
	{ "decrypt_ccm", zigbee_crypt_decrypt_ccm, METH_VARARGS, "decrypt_ccm(key, nonce, mic, encrypted_payload, zigbee_data)\nDecrypt data with a 0, 32, 64, or 128-bit MIC\n\n@type key: String\n@param key: 16-byte decryption key\n@type nonce: String\n@param nonce: 13-byte nonce\n@type mic: String\n@param mic: 4-16 byte message integrity check (MIC)\n@type encrypted_payload: String\n@param encrypted_payload: The encrypted data to decrypt\n@type zigbee_data: String\n@param zigbee_data: The zigbee data within the frame, without the encrypted payload, MIC, or FCS" },
	{ "encrypt_ccm", zigbee_crypt_encrypt_ccm, METH_VARARGS, "encrypt_ccm(key, nonce, mic_size, decrypted_payload, zigbee_data)\nEncrypt data with a 0, 32, 64, or 128-bit MIC\n\n@type key: String\n@param key: 16-byte decryption key\n@type nonce: String\n@param nonce: 13-byte nonce\n@type mic_size: Integer\n@param mic_size: the size in bytes of the desired MIC\n@type decrypted_payload: String\n@param decrypted_payload: The decrypted data to encrypt\n@type zigbee_data: String\n@param zigbee_data: The zigbee data within the frame, without the decrypted payload, MIC or FCS" },
	{ "sec_key_hash", zigbee_sec_key_hash, METH_VARARGS, "sec_key_hash(key, input)\nHash the supplied key as per ZigBee Cryptographic Hash (B.1.3 and B.6).\n\n@type key: String\n@param key: 16-byte key to hash\n@type input: Char\n@param input: Character terminator for key" },
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
