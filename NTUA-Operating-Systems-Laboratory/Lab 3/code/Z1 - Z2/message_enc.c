#include "message_enc.h"

char crypt_key[KEY_SIZE] = {0x64, 0x65, 0x65, 0x70, 0x64, 0x61, 0x70, 0x75, 0x66, 0x6c, 0x79, 0x62, 0x6c, 0x75, 0x65, 0x00};

void encrypt_decrypt_message (bool function, char* message, int* message_size, int cfd) {
	int offset, offset_limit, i;
	char data[AES_BLOCK_SIZE];
	/* We want to ensure that we will have AES_BLOCK_SIZE
	 * bytes of memory for the text we want to encrypt/decrypt,
	 * even after the address alignment. So, We declare not
	 * only AES_BLOCK_SIZE bytes, but also the next 16 - 1 
	 * bytes for the worst case */ /* (!) */
	char raw[AES_BLOCK_SIZE + 63], *text;
	char iv[AES_BLOCK_SIZE];
	struct cryptodev_ctx ctx;

	offset_limit = (*message_size)/AES_BLOCK_SIZE + ((*message_size) % AES_BLOCK_SIZE > 0);

	/* Empty the remaining space to end up with
	 * a message with length equal to a multiple
	 * of AES_BLOCK_SIZE (necessary for the encryption)*/
	for (i = *message_size; i < offset_limit*AES_BLOCK_SIZE; i++)
		message[i] = 0;

	/* We assume a zeroed initial vector for simplicity */
	memset(iv, 0x0, sizeof(iv));

	/* Initialize the cryptographic session */
	aes_ctx_init(&ctx, cfd, crypt_key, KEY_SIZE);

	if (ctx.alignmask)
		/* If needed, we fix the address alignment according to
		 * the given alignment mask */
		text = (char *)(((unsigned long)raw + ctx.alignmask) & ~ctx.alignmask);
	else
		text = raw;
		
	for (offset = 0; offset < offset_limit; offset++) {
		/* We store the data we want to encrypt at the aligned address */
		memcpy(text, message + offset*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		if (function == ENCRYPT)
			aes_encrypt(&ctx, iv, text, message + offset*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
		else
			aes_decrypt(&ctx, iv, text, message + offset*AES_BLOCK_SIZE, AES_BLOCK_SIZE);
	}

	/* End the session */
	aes_ctx_deinit(&ctx);

	/* Renew the message size */
	*message_size = offset_limit * AES_BLOCK_SIZE;
}
