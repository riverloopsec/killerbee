#ifndef PACKET_ZBEE_SECURITY_H
#define PACKET_ZBEE_SECURITY_H

/* Bit masks for the Security Control Field. */
#define ZBEE_SEC_CONTROL_LEVEL  0x07
#define ZBEE_SEC_CONTROL_KEY    0x18
#define ZBEE_SEC_CONTROL_NONCE  0x20

/* ZigBee security levels. */
#define ZBEE_SEC_NONE           0x00
#define ZBEE_SEC_MIC32          0x01
#define ZBEE_SEC_MIC64          0x02
#define ZBEE_SEC_MIC128         0x03
#define ZBEE_SEC_ENC            0x04
#define ZBEE_SEC_ENC_MIC32      0x05
#define ZBEE_SEC_ENC_MIC64      0x06
#define ZBEE_SEC_ENC_MIC128     0x07

/* ZigBee Key Types */
#define ZBEE_SEC_KEY_LINK       0x00
#define ZBEE_SEC_KEY_NWK        0x01
#define ZBEE_SEC_KEY_TRANSPORT  0x02
#define ZBEE_SEC_KEY_LOAD       0x03

/* ZigBee Security Constants. */
#define ZBEE_SEC_CONST_L            2
#define ZBEE_SEC_CONST_BLOCKSIZE    16
#define ZBEE_SEC_CONST_NONCE_LEN    (ZBEE_SEC_CONST_BLOCKSIZE-ZBEE_SEC_CONST_L-1)
#define ZBEE_SEC_CONST_MICSIZE		16
#define ZBEE_SEC_CONST_KEYSIZE		16

/* CCM* Flags */
#define ZBEE_SEC_CCM_FLAG_L             0x01    /* 3-bit encoding of (L-1). */
#define ZBEE_SEC_CCM_FLAG_M(m)          ((((m-2)/2) & 0x7)<<3)  /* 3-bit encoding of (M-2)/2 shifted 3 bits. */
#define ZBEE_SEC_CCM_FLAG_ADATA(l_a)    ((l_a>0)?0x40:0x00)     /* Adata flag. */

#endif
