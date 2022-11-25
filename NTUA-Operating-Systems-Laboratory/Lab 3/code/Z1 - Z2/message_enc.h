#include <stdbool.h>
#include "aes.h"

#define ENCRYPT 0
#define DECRYPT 1
#define KEY_SIZE 16

void encrypt_decrypt_message (bool function, char* message, int* message_size, int cfd); 
