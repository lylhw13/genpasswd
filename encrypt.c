#include "encrypt.h"

static EVP_CIPHER_CTX *d_ctx;
static EVP_CIPHER_CTX *e_ctx;

static void handleErrors(void)
{
    ERR_print_errors_fp(stderr);
    abort();
}

static void _enc_ctx_init(EVP_CIPHER_CTX **ctx, unsigned char *key, unsigned char *iv, int enc)
{
    if(!(*ctx = EVP_CIPHER_CTX_new()))
        handleErrors();

    if (1 != EVP_CipherInit_ex(*ctx, EVP_aes_256_cbc(), NULL, key, iv, enc))
        handleErrors();
}

void enc_ctx_init(unsigned char *key, unsigned char *iv)
{
    _enc_ctx_init(&e_ctx, key, iv, 1);    /* 1 for encryption */
    _enc_ctx_init(&d_ctx, key, iv, 0);    /* 0 for decryption */
}

static int encry_decry_ctx(unsigned char *inbuf, int in_len, EVP_CIPHER_CTX *ctx, unsigned char *outbuf)
{
    int len, out_len;

    if (ctx != NULL) {
        if (1 != EVP_CipherUpdate(ctx, outbuf, &len, inbuf, in_len))
            handleErrors();
        out_len = len;

        if (1 != EVP_CipherFinal_ex(ctx, outbuf + len, &len))
            handleErrors();
        out_len += len;

        return out_len;
    }
    return 0;
}

int encry(unsigned char *plaintext, int plaintext_len, unsigned char *ciphertext)
{
    return encry_decry_ctx(plaintext, plaintext_len, e_ctx, ciphertext);
}

int decry(unsigned char *ciphertext, int ciphertext_len, unsigned char *plaintext)
{
    return encry_decry_ctx(ciphertext, ciphertext_len, d_ctx, plaintext);
}

void cleanup(void)
{
    EVP_CIPHER_CTX_free(e_ctx);
    EVP_CIPHER_CTX_free(d_ctx);
}

#define KEY_LEN 32
#define IV_LEN 16

// int main()
// {
//     /* A 256 bit key */
//     // unsigned char *key = (unsigned char *)"6ccf2517b89369695182fc1d0581662c";

//     // /* A 128 bit IV */
//     // unsigned char *iv = (unsigned char *)"b5fb2c86683cfe3e";
//     unsigned char *key  = (unsigned char *)"01234567890123456789012345678901";
//     unsigned char *key2 = (unsigned char *)"01234567890123456789012345111111";

//     /* A 128 bit IV */
//     unsigned char *iv = (unsigned char *)"0123456789012345";

 

//     enc_ctx_init(key2, iv);
//     // cleanup();

//     enc_ctx_init(key,iv);

//     unsigned char *plaintext = (unsigned char *)"The quick brown fox jumps over the lazy dog";

//     unsigned char ciphertext[128];
//     unsigned char decryptedtext[128];

//     int decryptedtext_len, ciphertext_len;
    
//     printf("before\n");
//     if (e_ctx == NULL){
//         puts("e_ctx is NULL");
//     }
//     ciphertext_len = encry(plaintext, strlen((char *)plaintext), ciphertext);
//     // ciphertext_len = encry(plaintext, strlen((char *)plaintext), ciphertext);


//     printf("Ciphertext is:\n");
//     BIO_dump_fp (stdout, (const char *)ciphertext, ciphertext_len);

//     decryptedtext_len = decry(ciphertext, ciphertext_len, decryptedtext);

//     // decryptedtext_len = encry_decry_ctx(ciphertext, ciphertext_len, d_ctx, decryptedtext);

//     // /* Add a NULL terminator. We are expecting printable text */
//     decryptedtext[decryptedtext_len] = '\0';

//     // /* Show the decrypted text */
//     printf("Decrypted text is:\n");
//     printf("%s\n", decryptedtext);

//     cleanup();

//     return 0;
// }