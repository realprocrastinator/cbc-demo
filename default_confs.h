#ifndef DEFAULT_CONF_H_
#define DEFAULT_CONF_H_

#include <stdlib.h>

#define DEBUG 0
#define MAX_K_LEN 32

#define DEFAULT_SEED 0
#define DEFAULT_KEYLEN 4
#define DEFAULT_BLOCKSIZE 4 // welcome to change it!
#if DEBUG
  srandom(DEFAULT_SEED);
#else
#endif

// hard-coded here for demostrating the WK01 
// short question review using key map 
// @Lectnotes WK01-03 P32
// int default_keymap[8] = {0b110, 0b001, 0b111, 0b101, 
//                           0b100, 0b011, 0b010, 0b000};
int default_keymap[16] = { 0b1100, 0b0010, 0b1001, 0b1010, 
                           0b1011, 0b0111, 0b0011, 0b0001,
                           0b1101, 0b1000, 0b0101, 0b0100,
                           0b1111, 0b1110, 0b0110, 0b0000 };

#define MODE_ENCRYPT 0x1
#define MODE_DECRYPT 0x2
#define MODE_ENDE (MODE_ENCRYPT | MODE_DECRYPT)
#define MODE_ENMSK 0x1
#define MODE_DEMSK 0x2


#endif // DEFAULT_CONF_H_