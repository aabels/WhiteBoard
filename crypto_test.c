#include <openssl/evp.h>
#include <string.h>
#include <stdlib.h>
int main(){
do_crypt();
return 0;
}

int do_crypt()
        {
        unsigned char outbuf[1024];
        int outlen, tmplen, i;
        /* Bogus key and IV: we'd normally set these from
         * another source.
         * key is the symmetric key to use
         * iv is the IV to use (if necessary),
         * the actual number of bytes used for the key and IV depends on thecipher.
         */
        unsigned char key[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        unsigned char iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
        char intext[] = "some Crypto Text";

        /*cipher context */
        EVP_CIPHER_CTX ctx;

        /*Initilizes cipher context as ctx*/
        EVP_CIPHER_CTX_init(&ctx);

        /*sets up cipher context ctx for encryption with
        * cipher type from ENGINE impl.
        *impl is NULL then the default implementation is used. 
        int EVP_EncryptInit_ex(EVP_CIPHER_CTX *ctx, const EVP_CIPHER *type, ENGINE *impl, unsigned char *key, unsigned char *iv);*/
        EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv);

        /* Do we have to worry about padding?
         * PKCS padding works by adding n padding bytes of value n to make the
           total length of the encrypted data a multiple of the block size.
           Padding is always added so if the data is already a multiple of the
           block size n will equal the block size. For example if the block size
           is 8 and 11 bytes are to be encrypted then 5 padding bytes of value 5
           will be added.
         */

        /* encrypts inl (intext) bytes from the buffer in and writes
         * the encrypted version to out (outbuf).
         * This function can be called multiple times to encrypt successive blocks of data.
         * zero bytes to (inl + cipher_block_size - 1) so outl should contain sufficient room. The
         * actual number of bytes written is placed in outl (outlen).
         * int EVP_EncryptUpdate(EVP_CIPHER_CTX *ctx, unsigned char *out, int *outl, unsigned char *in, int inl);*/
        if(!EVP_EncryptUpdate(&ctx, outbuf, &outlen, intext, strlen(intext)))
                {
                /* Error */
                return 0;
                }
        /* Buffer passed to EVP_EncryptFinal() must be after data just
         * encrypted to avoid overwriting it.
         * Encrypt_Final_ex encrypts the "final" data, that is any data that remains in a partial block.
         * final data is written to out (outbuf + outlen), number of bytes wirrten is (tmplen)
         * int EVP_EncryptFinal_ex(EVP_CIPHER_CTX *ctx, unsigned char *out,int *outl); */
        if(!EVP_EncryptFinal_ex(&ctx, (outbuf + outlen), &tmplen))
                {
                /* Error */
                return 0;
                }
        outlen += tmplen;
        EVP_CIPHER_CTX_cleanup(&ctx);
        
        for (i=0; i<=(outlen-1); i++){
		printf("%C", outbuf[i]);
	   }
	   printf("\n");
        return 1;
        }
