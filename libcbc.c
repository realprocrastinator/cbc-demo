/* Library for exncrypting and decrypting */

#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdint.h>
#include<string.h>
#include"libcbc.h"
#include"utils.h"

#define MAX_K_LEN 32

/*
 * helper functions
 * keymap format 0001:1000
 */
int *parse_keymap (const char* f_name, size_t key_len) {
  FILE *fp;
  const size_t bufsize = key_len * 2 + 2; // twobyets for ':' and '\n'
  char *buf = (char*) malloc(bufsize);
  if (!buf) {
    printf("Failed to malloc()\n.");
    return NULL;
  }
  memset(buf, 0, bufsize);

  int *keymap = (int*)malloc((1 << key_len) * sizeof(int));
  if (!keymap) {
    printf("Failed to malloc()\n.");
    free(buf);
    return NULL;
  }
  memset(keymap, 0, sizeof((1 << key_len) * sizeof(int)));
  
  fp = fopen(f_name, "r");
  if (!fp) {
    printf("Failed to open file %s.\n", f_name);
    free(buf);
    free(keymap);
    return NULL;
  }

  // no matter what user provide we only parse first key_len entries
  for (int i = 0; i < (1 << key_len); ++i) {
    size_t rc = fread(buf, bufsize, 1, fp);
    if (rc != 1) {
      if (ferror(fp)) {
        printf("fread() failed %lu bytes read.\n", rc);
        free(buf);
        free(keymap);
        fclose(fp);
        return NULL;

      } else if (feof(fp)) {
        // reach the EOF already
        break;
      } else {
        puts("We should not reach this part otherwise there is a bug.");
      }
    } else {
     
      buf[bufsize - 1] = '\0';
      printf("Got the %dth key entry: %s.\n", i, buf);

       // char *key = strchr(buf, ":") + 1; // move one byte forward to excape the token.
      int key_value = str2bin(&strchr(buf, ':')[1], key_len);
      int idx = str2bin(strtok(buf, ":"), key_len);
      keymap[idx] = key_value;
    }
  }

  free(buf);
  fclose(fp);
  // do free keymap in caller!
  return keymap;
}


void show_keymap (int *keymap, size_t k_len) {
  for (int i = 0; i < 1 << k_len; ++i) {
    printf("Key[%d]=%d\n", i, keymap[i]);
  }
}


int *build_reverse_keymap (int keymap[], size_t key_len) {
  const size_t n_elemts = (1 << key_len);
  int *reverse_keymap = (int *) malloc(n_elemts * sizeof(int));
  if (reverse_keymap == NULL) {
    printf("Can't allocate memory.\n");
    return NULL;
  }

  for (int i = 0; i < n_elemts; ++i) {
    int idx = keymap[i];
    reverse_keymap[idx] = i;
    printf("Reversing Keymap: r_k[%u] = %u\n", idx, i);
  }

  return reverse_keymap;
}

// For simplicty we don't check error no here
int do_encrypt(const char *f_plaintext, const int iv, const size_t block_size, int *keymap, int keylen) {
  int fd = open(f_plaintext, O_RDONLY, 0700);
  if (fd < 0) {
    printf("Failed to open file %s.\n", f_plaintext);
    return -1;
  }

  // hard coded the output cipher file path here.
  const char *f_cipher = DEFAULT_CIPHER_OUT;
  int ofd = open(f_cipher, O_RDWR | O_CREAT, 0700);
  if (ofd < 0) {
    printf("Failed to open file %s.\n", f_cipher);
    return -1;
  }

  // since block size is calculated in bits, we find the lcm of block_size 
  // and a byte size to make future life ezer!
  const size_t n_bytes = lcm(block_size, CHAR_BIT) / CHAR_BIT;
  
  char *buf = malloc(n_bytes);
  if (!buf) {
    printf("Failed to allocated memory.\n");
    close(fd);
    close(ofd);
    return -1;
  }
  memset(buf, 0, n_bytes);

  int n_read = 0;
  int n_blocks = 0;
  int rc = 0;

  // We don't handle if block_size if larger than 8 bits...Why because it can't be put into a unsigned int type if
  // block_size is not aligned with 8 bit, then it would be tricky to handle!!!
  while ((rc = read(fd, buf, n_bytes))) {
    puts("");
    printf("Have read %d bytes yet.\n", rc);

    // concatinating the plaintext bit by bit
    uint64_t b_data = 0;
    for (int i = 0; i < rc; ++i) {
      printf("The buf[%d] has %x\n", i, (uint8_t)buf[i]);
      b_data |= ((uint8_t)buf[i] << (i * CHAR_BIT));
    }
    printf("Got new b_data: 0x%lx.\n", b_data);

    uint64_t b_data_xored = 0;
    uint64_t c_data = 0;
    uint64_t prev_c_data = 0;

    size_t n_blocks_in_grp = rc * CHAR_BIT / block_size;
    size_t n_bits_in_grp = rc * CHAR_BIT;

    for (int i = 0; i < n_blocks_in_grp; ++i) {
      //TODO(2): show block bit by bit.
      printf("encrypting block[%d] with size %lu.\n", n_blocks, block_size);

      if (n_blocks == 0) prev_c_data = iv & BLOCK_MSK(block_size); 
      uint64_t cur_p_data = GET_BLOCK_DATA(b_data, (n_blocks * keylen) % n_bits_in_grp, block_size);
      b_data_xored = (cur_p_data ^ prev_c_data) & BLOCK_MSK(block_size);
      printf("Got Block[%d]: 0x%lx.\nAfter xoring with 0x%lx we got xored-bits: 0x%lx\n", \
            n_blocks, cur_p_data, prev_c_data, b_data_xored);

      // look up the key map && and write to chipher text file
      prev_c_data = keymap[b_data_xored];
      printf("Mapping xored data: 0x%lx -> 0x%lx, using keymap entry: %ld\n", b_data_xored, prev_c_data, b_data_xored);

      c_data |=  prev_c_data << ((n_blocks * keylen) % n_bits_in_grp);
      printf("Updating cipher we got: 0x%lx\n", c_data);
      ++n_blocks;
    }

    if (!write(ofd, (const void*) &c_data, rc)) {
        printf("Write cipher text failed.\n");
        free(buf);
        close(fd);
        close(ofd);
        return -1;
    }

    n_read += rc;
  }

  printf("Successfully read %d bytes.\n", n_read);
  free(buf);
  close(fd);
  close(ofd);
  return 0;
}

int do_decrypt(const char *f_ciphertext, const int iv, const size_t block_size, int *r_keymap, int keylen) {
  int fd = open(f_ciphertext, O_RDONLY, 0700);
  if (fd < 0) {
    printf("Failed to open cipher text file.\n");
    exit(1);
  }

  int ofd = open(DEFAULT_DECRYPTED_OUT, O_RDWR | O_CREAT, 0700);
  if (ofd < 0) {
    printf("Failed to creat decrypted text file.\n");
    exit(1);
  }

  // since block size is calculated in bits, we find the lcm of block_size 
  // and a byte size to make future life ezer!
  const size_t n_bytes = lcm(block_size, CHAR_BIT) / CHAR_BIT;
  
  char *buf = malloc(n_bytes);
  if (!buf) {
    printf("Failed to allocated memory.\n");
    close(fd);
    close(ofd);
    return -1;
  }
  memset(buf, 0, n_bytes);

  int n_read = 0;
  int n_blocks = 0;
  int rc = 0;

  // We don't handle if block_size if larger than 8 bits...Why because it can't be put into a unsigned int type if
  // block_size is not aligned with 8 bit, then it would be tricky to handle!!!
  while ((rc = read(fd, buf, n_bytes))) {
    puts("");
    printf("Have read %d bytes yet.\n", rc);
    // concatinating the plaintext bit by bit
    uint64_t b_data = 0;
    for (int i = 0; i < rc; ++i) {
      printf("The buf[%d] has %x\n", i, (uint8_t)buf[i]);
      b_data |= ((uint8_t)buf[i] << (i * CHAR_BIT));
    }
    printf("Got new b_data: 0x%lx.\n", b_data);

    uint64_t b_data_xored = 0;
    uint64_t p_data = 0;
    uint64_t cur_c_data = 0;
    uint64_t prev_c_data = 0;

    size_t n_blocks_in_grp = rc * CHAR_BIT / block_size;
    size_t n_bits_in_grp = rc * CHAR_BIT;

    for (int i = 0; i < n_blocks_in_grp; ++i) {
      //TODO(2): show block bit by bit.
      printf("encrypting block[%d] with size %lu.\n", n_blocks, block_size);

      cur_c_data = GET_BLOCK_DATA(b_data, (n_blocks * keylen) % n_bits_in_grp, block_size);
      b_data_xored = (r_keymap[cur_c_data] ^ (n_blocks == 0 ? iv : prev_c_data)) & BLOCK_MSK(block_size);
      printf("Mapping Cipher data: 0x%lx -> 0x%x,\nAfter xoring with 0x%lx we got decrypted bits: 0x%lx\n", \
              cur_c_data, r_keymap[cur_c_data], n_blocks == 0 ? iv : prev_c_data, b_data_xored);

      prev_c_data = cur_c_data;

      // look up the key map && and write to chipher text file
      p_data |= b_data_xored << ((n_blocks * keylen) % n_bits_in_grp);
      printf("Updating decrypted cipher we got: 0x%lx\n", p_data);
      ++n_blocks;
    }

    if (!write(ofd, (const void*) &p_data, rc)) {
        printf("Write decrypted cipher text failed.\n");
        free(buf);
        close(fd);
        close(ofd);
        return -1;
    }

    n_read += rc;
  }

  printf("Successfully read %d bytes.\n", n_read);
  free(buf);
  close(fd);
  close(ofd);
  return 0;
}