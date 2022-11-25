/*
 * Some functions to perform the Advanced
 * Encryption Algorithm on a dataset using
 * the cryptodev module
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <crypto/cryptodev.h>
#include "aes.h"

#define	KEY_SIZE 16

int aes_ctx_init(struct cryptodev_ctx* ctx, int cfd, const uint8_t *key, unsigned int key_size)
{
#ifdef CIOCGSESSINFO
	struct session_info_op siop;
#endif

	memset(ctx, 0, sizeof(*ctx));
	ctx->cfd = cfd;

	/* Define the parameters for the cryptographic session
	 * using the session_op field of the cryptodev_ctx
	 * structure */
	ctx->sess.cipher = CRYPTO_AES_CBC;
	ctx->sess.keylen = key_size;
	ctx->sess.key = (void*)key;
	
	/* According to the documentation */
	/* (https://www.freebsd.org/cgi/man.cgi?query=crypto&sektion=4) */
	/* Create a new cryptographic session on a file	descriptor for
	 * the device; that is,	a persistent object specific to	the
	 * chosen privacy algorithm, integrity algorithm, and keys
	 * specified in	sessp (here: stx->sess) */

	if (ioctl(ctx->cfd, CIOCGSESSION, &ctx->sess)) {
		perror("ioctl(CIOCGSESSION)");
		return -1;
	}

#ifdef CIOCGSESSINFO
	memset(&siop, 0, sizeof(siop));

	siop.ses = ctx->sess.ses;
	if (ioctl(ctx->cfd, CIOCGSESSINFO, &siop)) {
		perror("ioctl(CIOCGSESSINFO)");
		return -1;
	}

	if (!(siop.flags & SIOP_FLAG_KERNEL_DRIVER_ONLY)) {
		// This is not an accelerated cipher
	}
	
	ctx->alignmask = siop.alignmask;
#endif
	return 0;
}

void aes_ctx_deinit(struct cryptodev_ctx* ctx) 
{
	/* Destroy the session specified by ctx */
	if (ioctl(ctx->cfd, CIOCFSESSION, &ctx->sess.ses)) {
		perror("ioctl(CIOCFSESSION)");
	}
}

int
aes_encrypt(struct cryptodev_ctx* ctx, const void* iv, const void* plaintext, void* ciphertext, size_t size)
{
	struct crypt_op cryp;
	void* p;
	
	/* check plaintext and ciphertext alignment */
	/* https://stackoverflow.com/questions/36630960/c-aligning-memory */
	/* We assume that the alignment mask is of the form 00...0011...11
	 * (zeros followed exclusively by ones). The next check
	 * (p =? (p+MASK) (AND) (NOT MASK)) verifies that the bits indicated
	 * by the ones in the mask are zero in p */
	if (ctx->alignmask) {
		p = (void*)(((unsigned long)plaintext + ctx->alignmask) & ~ctx->alignmask);
		if (plaintext != p) {
			// fprintf(stderr, "plaintext is not aligned\n");
			return -1;
		}

		p = (void*)(((unsigned long)ciphertext + ctx->alignmask) & ~ctx->alignmask);
		if (ciphertext != p) {
			// fprintf(stderr, "ciphertext is not aligned\n");
			return -1;
		}
	}

	memset(&cryp, 0, sizeof(cryp));

	/* Request a symmetric-key (or hash) operation */
	cryp.ses = ctx->sess.ses;
	/* The field cr_op-_len supplies the length of
	 * the input buffer */
	cryp.len = size;
	/* the fields	cr_op-_src, cr_op-_dst,
	 * cr_op-_iv supply the addresses of the input
	 * buffer, output buffer and initialization
	 * vector, respectively */
	cryp.src = (void*)plaintext;
	cryp.dst = ciphertext;
	cryp.iv = (void*)iv;
	/* To encrypt, set cr_op-_op to COP_ENCRYPT.  To decrypt,
	 * set cr_op-_op to COP_DECRYPT */
	cryp.op = COP_ENCRYPT;
	/* Perform the IOCTL command */
	if (ioctl(ctx->cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return -1;
	}

	return 0;
}

/* The used encryption method is symmetric, so the next function
 * is almost exactly the same as the above */
int
aes_decrypt(struct cryptodev_ctx* ctx, const void* iv, const void* ciphertext, void* plaintext, size_t size)
{
	struct crypt_op cryp;
	void* p;
	
	/* check plaintext and ciphertext alignment */
	if (ctx->alignmask) {
		p = (void*)(((unsigned long)plaintext + ctx->alignmask) & ~ctx->alignmask);
		if (plaintext != p) {
			// fprintf(stderr, "plaintext is not aligned\n");
			return -1;
		}

		p = (void*)(((unsigned long)ciphertext + ctx->alignmask) & ~ctx->alignmask);
		if (ciphertext != p) {
			// fprintf(stderr, "ciphertext is not aligned\n");
			return -1;
		}
	}

	memset(&cryp, 0, sizeof(cryp));

	/* Encrypt data.in to data.encrypted */
	cryp.ses = ctx->sess.ses;
	cryp.len = size;
	cryp.src = (void*)ciphertext;
	cryp.dst = plaintext;
	cryp.iv = (void*)iv;
	cryp.op = COP_DECRYPT;
	if (ioctl(ctx->cfd, CIOCCRYPT, &cryp)) {
		perror("ioctl(CIOCCRYPT)");
		return -1;
	}

	return 0;
}
