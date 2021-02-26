#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include "utils.h"

uint64_t lcm(uint64_t a, uint64_t b) {
  assert(a > 0 && b > 0);
  uint64_t lcm = MAX(a, b);
  while (1) {
    if (lcm % a == 0 && lcm % b == 0) {
      break;
    } else {
      lcm++;
    }
  }

  return lcm;
}

uint64_t rand_uint64(void) {
  return ((uint64_t) random() << 32 | random());
}

// TODO(1): change the signature
int str2bin(const char *s, size_t key_len) {
  assert(s != NULL);

  size_t n_bytes = strlen(s);
  assert(n_bytes == key_len);
  int res = 0;

  for (int i = 0; i < key_len; ++i) {
    if (s[i] == '1') {
      res += (1 << (key_len - i - 1));
    } 
  }

  return res;
}