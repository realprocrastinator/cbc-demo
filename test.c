#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX_K_LEN 32

int str2bin(const char *s, size_t key_len) {
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
      // char *key = strchr(buf, ":") + 1; // move one byte forward to excape the token.
      buf[bufsize - 1] = '\0';
      printf("Got the %dth key entry: %s.\n", i, buf);
      char dummy[MAX_K_LEN]; // trick the sscanf
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


int main() {

  int *keymap = parse_keymap("./key", 4);
  if (!keymap) {
    printf("parse keymap failed.\n");
    exit(1);
  }
  
  show_keymap(keymap, 4);
  free(keymap);

  return 0;
}