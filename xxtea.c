#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "xxtea.h"
 

uint32_t *  u8_to_U32 (uint8_t b[16]){
  uint32_t *res;
  res = (uint32_t*)b;
  return res;
}  
void btea(uint32_t *v, int n, uint32_t const key[4]) {
  uint32_t y, z, sum;
  unsigned p, rounds, e;
  if (n > 1) {          /* Coding Part */
    rounds = 6 + 52/n;
    sum = 0;
    z = v[n-1];
    do {
      sum += DELTA;
      e = (sum >> 2) & 3;
      for (p=0; p<n-1; p++) {
	y = v[p+1]; 
	z = v[p] += MX;
      }
      y = v[0];
      z = v[n-1] += MX;
    } while (--rounds);
  } else if (n < -1) {  /* Decoding Part */
    n = -n;
    rounds = 6 + 52/n;
    sum = rounds*DELTA;
    y = v[0];
    do {
      e = (sum >> 2) & 3;
      for (p=n-1; p>0; p--) {
	z = v[p-1];
	y = v[p] -= MX;
      }
      z = v[n-1];
      y = v[0] -= MX;
      sum -= DELTA;
    } while (--rounds);
  }
}

uint8_t * u32_to_u8(uint32_t b){
  uint8_t * d = NULL;
  d = malloc(sizeof (*d));
  for (int i=0; i<4 ;i++)
    d[i] = ((uint8_t*)&b)[3-i];
  return d;
}
