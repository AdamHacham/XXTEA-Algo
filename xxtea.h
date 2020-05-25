#ifndef h_xxtea
#define h_xxtea

#define DELTA 0x9e3779b9
#define MX (((z>>5^y<<2) + (y>>3^z<<4)) ^ ((sum^y) + (key[(p&3)^e] ^ z)))

uint32_t *  u8_to_U32 (uint8_t b[16]);
void btea(uint32_t *v, int n, uint32_t const key[4]);
uint8_t * u32_to_u8(uint32_t b);

#endif
