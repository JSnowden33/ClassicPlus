/* 
 * File:   crypto.h  
 * Author: Jackson Snowden
 */

#ifndef _CRYPTO_H_
#define	_CRYPTO_H_

u8 ROR8(u8 a, u8 b);

u8 GenEncryption();

u8 InitKeys(u8 *buf);

u8 Encrypt(u8 addr, u8 data);

u8 Decrypt(u8 addr, u8 data);

#endif  /* _CRYPTO_H_ */