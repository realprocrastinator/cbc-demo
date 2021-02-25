#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<limits.h>
#include<fcntl.h>
#include<unistd.h>
#include<stdint.h>
#include<string.h>
#include<assert.h>

// helpers to find the least common multiple
#define MAX(a, b) a > b ? a : b;
#define BLOCK_MSK(block_size) (uint64_t)((1 << block_size) - 1)
#define GEN_IV(block_size) rand_uint64() & BLOCK_MSK(block_size) 
#define GET_BLOCK_DATA(data, i, block_size) ((data & (BLOCK_MSK(block_size) << i)) >> i)

#define DEFAULT_CIPHER_OUT "./cipher_out"
#define DEFAULT_PLAINTEXT "./demo_in"
#define DEFAULT_DECRYPTED_OUT "./decrypted_text"
#define DEFAULT_KEYLEN 4
#define DEFAULT_BLOCKSIZE 4 // welcome to change it!

// hard-coded here for demostrating the WK01 
// short question review using key map 
// @Lectnotes WK01-03 P32
// int default_keymap[8] = {0b110, 0b001, 0b111, 0b101, 
//                           0b100, 0b011, 0b010, 0b000};
int default_keymap[16] = { 0b1100, 0b0010, 0b1001, 0b1010, 
                           0b1011, 0b0111, 0b0011, 0b0001,
                           0b1101, 0b1000, 0b0101, 0b0100,
                           0b1111, 0b1110, 0b0110, 0b0000 };

static bool use_default_keymap = false;

// helpers to find the least common multiple
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

// helper to generate 64 bit unsigned
uint64_t rand_uint64(void) {
  return ((uint64_t) rand() << 32 | rand());
}

int *parse_keymap (const char* f_name, size_t key_len) {
  printf("Not implemented yet.\n");
  return NULL;
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
int do_encrypt(const char *f_plaintext, const int iv, const size_t block_size, int *keymap) {
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
      uint64_t cur_p_data = GET_BLOCK_DATA(b_data, (n_blocks * 3) % n_bits_in_grp, block_size);
      b_data_xored = (cur_p_data ^ prev_c_data) & BLOCK_MSK(block_size);
      printf("Got Block[%d]: 0x%lx.\nAfter xoring with 0x%lx we got xored-bits: 0x%lx\n", \
            n_blocks, cur_p_data, prev_c_data, b_data_xored);

      // look up the key map && and write to chipher text file
      prev_c_data = keymap[b_data_xored];
      printf("Mapping xored data: 0x%lx -> 0x%lx, using keymap entry: %ld\n", b_data_xored, prev_c_data, b_data_xored);

      c_data |=  prev_c_data << ((n_blocks * 3) % n_bits_in_grp);
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

int do_decrypt(const char *f_ciphertext, const int iv, const size_t block_size, int *r_keymap) {
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

      cur_c_data = GET_BLOCK_DATA(b_data, (n_blocks * 3) % n_bits_in_grp, block_size);
      b_data_xored = (r_keymap[cur_c_data] ^ (n_blocks == 0 ? iv : prev_c_data)) & BLOCK_MSK(block_size);
      printf("Mapping Cipher data: 0x%lx -> 0x%x,\nAfter xoring with 0x%lx we got decrypted bits: 0x%lx\n", \
              cur_c_data, r_keymap[cur_c_data], n_blocks == 0 ? iv : prev_c_data, b_data_xored);

      prev_c_data = cur_c_data;

      // look up the key map && and write to chipher text file
      p_data |= b_data_xored << ((n_blocks * 3) % n_bits_in_grp);
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

int main (int argc, char *argv[]) {
  puts("Demostrating the Block Cipher Chaining Process...");

  char *f_plaintext = DEFAULT_PLAINTEXT;
  size_t block_size; // Notive this must be dividable by even number or dividable by total plaintext in bits 
  size_t k_len;
  int *keymap = default_keymap;

  if (argc == 1) {
    // using default default_keymap for demostrating lect notes :D
    use_default_keymap = true;
    k_len = DEFAULT_KEYLEN;
    block_size = DEFAULT_BLOCKSIZE; 
  }
  else if (argc < 5) {
    // plaintext, key_map, key_len, block_size in bits
    printf("Usage: ./demoCBC <plaintext> <blocksize> <key_map> <key_len>,\n \
        Notice: block_size must be either even number or a factor of the total bits of plaintext,\n \
        otherwise the algorithm won't work.\n");
    exit(1);
  } 
  else {
    if (!sscanf(argv[2], "%lu", &block_size)) {   
      printf("Can't cast string to size_t for 'block_size'.\n");
      exit(1);
    }
    
    // TODO(1): add padding when key_length and block size mismatch
    if(!sscanf(argv[4], "%lu", &k_len) || k_len != block_size) {
      printf("Can't cast string to size_t for 'k_len'.\n");
      exit(1);
    }
  } 

  if (!use_default_keymap) {
    char *f_keymap = argv[3];
      
    // parse keymap file
    printf("Parsing costum keymap file.\n");
    keymap = parse_keymap(f_keymap, k_len);
  }

  printf("\nNow start demostraing the encryption process.\n");
  char *f_output = DEFAULT_CIPHER_OUT;

  // generate IV with block size, currently won't be able to handle a block larger then
  // 64 * 8 bits!
  uint64_t iv = GEN_IV(block_size);
  printf("\nThe IV is: %lu.\n", iv);

  if (do_encrypt(f_plaintext, iv, block_size, keymap)) {
    printf("Encryption failed.\n");
    exit(1);
  }
  printf("\nThe cipher file is saved to %s.\n", f_output);

  // show encryption file;

  // build reverse key_map
  int *reverse_keymap = build_reverse_keymap(keymap, k_len);

  printf("\nNow start demostraing the decryption process.\n");
  if (do_decrypt(f_output, iv, block_size, reverse_keymap)) {
      printf("Decryption failed.\n");
      exit(1);
  }
  // show decryption file;

  // end clean up if not using the default key map ;)
  if (!use_default_keymap) {
    free(keymap);
  }

  return 0;
}