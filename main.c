#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h> 
#include <unistd.h>
#include "libcbc.h"
#include "utils.h"
#include "default_confs.h"

static bool use_default_keymap = false;

static void print_usage() {
  // straight copy paste from the assignment spec!
  puts(
      "Usage: demoCBC [D|E]...[OPTIONS]...\n If no OPTIONS specified, this program wll run in a demo mode.\n"
      "with all default settings."
      "Notice: block_size must be either even number or a factor of the total bits of plaintext,\n"
      "otherwise the algorithm won't work.\n"
      "\n"
      "   E             encryption mode\n"
      "   D             decryption mode\n"
      "  -P PLAINTEXT   file path of your plian text file\n"
      "  -h             this usage message\n"
      "  -L KEYLEN      length for your keys in bytes\n"
      "  -K KEYFILE     you key file. please match the format\n"
      "  -B BLOCKSIZE   BLOCKSIZE must be either an even number\n"
      "                 or is dividable by totoal size of the plaintext in bits\n"
      "  -v             print version information\n"
      "\n"
      "Example usage:\n"
      "For encryption try ./demoCBC E -P text -L 4 -K keymap -B 4\n"
      "This will encrypt a text file named text with keymap called keymap\n"
      "key length and block size of 4\n");
}

static bool 
parse_args( 
  int argc, 
  char** argv, 
  char** f_plaintext, 
  char** keyfile, 
  size_t* key_len, 
  size_t* block_size, 
  int* optidx) 
{  
  int opt_switch;
  while ((opt_switch = getopt(argc, argv, "hvP:K:L:B:")) != -1) {
    switch (opt_switch) {
      case 'h':
        goto invalid_args;
      case 'v':
        puts("Version: V2.0");
        return false;
      case 'P':
        *f_plaintext = optarg;
        break;
      case 'K':
        *keyfile = optarg;
        break;
      case 'L':
        if (sscanf(optarg, "%lu", key_len) < 1) {
          goto invalid_args;
        }
        break;
      case 'B':
        if (sscanf(optarg, "%lu", block_size) < 1) {
          goto invalid_args;
        }
        break;
      default:
        goto invalid_args;
    }
  }

  *optidx = optind;
  printf("optind is %d\n", optind);
  return true;

invalid_args:
  print_usage();
  return false;
}

int main (int argc, char *argv[]) {
  puts("Program for Demostrating the Block Cipher Chaining Process...\n");

  char *f_plaintext = NULL;
  size_t block_size = 0; // Notive this must be dividable by even number or dividable by total plaintext in bits 
  size_t k_len = 0;
  char *f_keymap = NULL;
  int *keymap = NULL;
  int optidx;
  char *f_output = DEFAULT_CIPHER_OUT; // cipher output path

  if (!parse_args(argc, argv, &f_plaintext, &f_keymap, &k_len, &block_size, &optidx)) {
    exit(1);
  }

  uint8_t run_mode = 0;

  if (!keymap) {
    use_default_keymap = true;
    keymap = default_keymap;
  }

  if (!f_plaintext) {
    f_plaintext = DEFAULT_PLAINTEXT;
  }

  if (!k_len) {
    k_len = DEFAULT_KEYLEN;
  }

  if (!block_size) {
    block_size = DEFAULT_BLOCKSIZE; 
  }

  if (optidx == 1) {
    run_mode |= MODE_ENDE;
  }
  else if (optidx >= argc) {
    // plaintext, key_map, key_len, block_size in bits
    printf("Failed to handle commands.\n");
    exit(1);
  } 
  else {
    if (strcmp(argv[optidx], "D") == 0) {run_mode |= MODE_DECRYPT;}
    else if (strcmp(argv[optidx], "E") == 0) {run_mode |= MODE_ENCRYPT;}
    else {printf("Invalid argument!\n"); exit(1);}
  } 

  if (!use_default_keymap) {
    printf("Parsing costum keymap file.\n");
    keymap = parse_keymap(f_keymap, k_len);
  }

  // generate IV with block size, currently won't be able to handle a block larger then
  // 64 * 8 bits!
  uint64_t iv = GEN_IV(block_size);
  printf("\nThe IV is: %lu.\n", iv);

  if ((run_mode & (MODE_ENMSK | MODE_DEMSK)) == MODE_ENDE) {
    puts("Running demo mode, demo encryption first then demo decryption.");
  }

  if (run_mode & MODE_ENMSK) {
    printf("\nNow start demostraing the encryption process.\n");

    if (do_encrypt(f_plaintext, iv, block_size, keymap, k_len)) {
      printf("Encryption failed.\n");
      exit(1);
    }
    printf("\nThe cipher file is saved to %s.\n", f_output);

    //TODO(1): show encryption file;
  }


  if (run_mode & MODE_DEMSK) {
    // build reverse key_map
    int *reverse_keymap = build_reverse_keymap(keymap, k_len);

    printf("\nNow start demostraing the decryption process.\n");
    if (do_decrypt(f_output, iv, block_size, reverse_keymap, k_len)) {
        printf("Decryption failed.\n");
        exit(1);
    }
    // TODO(1):show decryption file;
  }

  // end clean up if not using the default key map ;)
  if (!use_default_keymap) {
    free(keymap);
  }

  return 0;
}