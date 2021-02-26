#ifndef LIBCBC_H_
#define LIBCBC_H_

#define DEFAULT_CIPHER_OUT "./cipher_out"
#define DEFAULT_PLAINTEXT "./demo_in"
#define DEFAULT_DECRYPTED_OUT "./decrypted_text"

int 
do_encrypt (
  const char *f_plaintext, 
  const int iv, 
  const size_t block_size, 
  int *keymap, 
  int keylen);

int 
do_decrypt (
  const char *f_ciphertext, 
  const int iv, 
  const size_t block_size, 
  int *r_keymap, 
  int keylen);

int *
parse_keymap (
  const char* f_name, 
  size_t key_len);

int *
build_reverse_keymap (
  int keymap[], 
  size_t key_len);


#endif // LIBCBC_H_