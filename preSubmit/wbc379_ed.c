#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <openssl/evp.h>
#include <openssl/pem.h>

#define OK       0
#define NO_INPUT 1
#define TOO_LONG 2

#define MAX_STRING      32
#define MAX_LINES       100

// Bogus key and IV: we'd normally set these from another source
//unsigned char key[] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";//{0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
unsigned char iv[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
int encrypted_count, decrypted_count, number_of_keys = 0;
char *str_to_send;

char *base64encode (const void *b64_encode_this, int encode_this_many_bytes);
char *base64decode (const void *b64_decode_this, int decode_this_many_bytes);
char *do_crypt(char* text, char *key);
char *do_decrypt(char* text, int x, unsigned char *try_this_key);
void prepend(char* s, const char* t);
char **arrayLines(FILE *fp1);
char *get_output();


// ########################## BASE 64 ENCODE AND DECODE #######################################
char *base64encode (const void *b64_encode_this, int encode_this_many_bytes){
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    BUF_MEM *mem_bio_mem_ptr;    //Pointer to a "memory BIO" structure holding our base64 data.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                           //Initialize our memory sink BIO.
    BIO_push(b64_bio, mem_bio);            //Link the BIOs by creating a filter-sink BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);  //No newlines every 64 characters or less.
    BIO_write(b64_bio, b64_encode_this, encode_this_many_bytes); //Records base64 encoded data.
    BIO_flush(b64_bio);   //Flush data.  Necessary for b64 encoding, because of pad characters.
    BIO_get_mem_ptr(mem_bio, &mem_bio_mem_ptr);  //Store address of mem_bio's memory structure.
    BIO_set_close(mem_bio, BIO_NOCLOSE);   //Permit access to mem_ptr after BIOs are destroyed.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    BUF_MEM_grow(mem_bio_mem_ptr, (*mem_bio_mem_ptr).length + 1);   //Makes space for end null.
    (*mem_bio_mem_ptr).data[(*mem_bio_mem_ptr).length] = '\0';  //Adds null-terminator to tail.
    return (*mem_bio_mem_ptr).data; //Returns base-64 encoded data. (See: "buf_mem_st" struct).
}

char *base64decode (const void *b64_decode_this, int decode_this_many_bytes){
    BIO *b64_bio, *mem_bio;      //Declares two OpenSSL BIOs: a base64 filter and a memory BIO.
    char *base64_decoded = calloc( (decode_this_many_bytes*3)/4+1, sizeof(char) ); //+1 = null.
    b64_bio = BIO_new(BIO_f_base64());                      //Initialize our base64 filter BIO.
    mem_bio = BIO_new(BIO_s_mem());                         //Initialize our memory source BIO.
    BIO_write(mem_bio, b64_decode_this, decode_this_many_bytes); //Base64 data saved in source.
    BIO_push(b64_bio, mem_bio);          //Link the BIOs by creating a filter-source BIO chain.
    BIO_set_flags(b64_bio, BIO_FLAGS_BASE64_NO_NL);          //Don't require trailing newlines.
    int decoded_byte_index = 0;   //Index where the next base64_decoded byte should be written.
    while ( 0 < BIO_read(b64_bio, base64_decoded+decoded_byte_index, 1) ){ //Read byte-by-byte.
        decoded_byte_index++; //Increment the index until read of BIO decoded data is complete.
    } //Once we're done reading decoded data, BIO_read returns -1 even though there's no error.
    BIO_free_all(b64_bio);  //Destroys all BIOs in chain, starting with b64 (i.e. the 1st one).
    return base64_decoded;        //Returns base-64 decoded data with trailing null terminator.
}

// ########################## AES 256 ENCODE #######################################
char *do_crypt(char* text, char *key){
    unsigned char outbuf[1024];
    int outlen, tmplen, i;
    char *encrypt_text = text;
    char *stringx;
    
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_EncryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, key, iv);

    if(!EVP_EncryptUpdate(&ctx, outbuf, &outlen, (unsigned char *) encrypt_text, strlen(encrypt_text)))
    {
        /* Error */
        return NULL;
    }
    if(!EVP_EncryptFinal_ex(&ctx, outbuf + outlen, &tmplen))
    {
        /* Error */
        return NULL;
    }
    outlen += tmplen;
    EVP_CIPHER_CTX_cleanup(&ctx);

    stringx = malloc(outlen);

    for (i=0; i<=(outlen-1); i++)
    {
        stringx[i] = (char)outbuf[i];
    }
    encrypted_count = outlen;
    return stringx;
}

// ########################## AES 256 DECODE #######################################
char *do_decrypt(char* text, int x, unsigned char *try_this_key){
    unsigned char debuf[1024];
    int delen, remainingBytes, i;
    char *decrypt_text = text;
    char *string;
    
    EVP_CIPHER_CTX ctx;
    EVP_CIPHER_CTX_init(&ctx);
    EVP_DecryptInit_ex(&ctx, EVP_aes_256_cbc(), NULL, try_this_key, iv);

    if(!EVP_DecryptUpdate(&ctx, debuf, &delen, (unsigned char *) decrypt_text, x))
    {
        /* Error */
        return NULL;
    }
    if(!EVP_DecryptFinal_ex(&ctx, debuf + delen, &remainingBytes))
    {
        /* Error */
        return NULL;
    }
    delen += remainingBytes;
    EVP_CIPHER_CTX_cleanup(&ctx);
    string = malloc(delen);
    for (i=0; i<=(delen-1); i++)
    {
        string[i] = (char)debuf[i];
    }
    decrypted_count = delen;
    return string;
}

// ############### PREPEND FUNCTION ####################
void prepend(char* s, const char* t){
    size_t len = strlen(t);
    size_t i;

    memmove(s + len, s, strlen(s) + 1);

    for (i = 0; i < len; ++i)
    {
        s[i] = t[i];
    }
}

// ############### PUTS KEYS INTO ARRAY ####################
char **arrayLines(FILE *fp1)
{
    int i = 0;
    char **line_array = malloc(sizeof (char *) * MAX_STRING);
    char line[MAX_LINES];

    while(fgets(line, sizeof(line), fp1) !=NULL)
    {
        line_array[i] = malloc(MAX_STRING);
        if (!line_array[i]){
            free (line_array);
            return NULL;
        }
        line_array[i] = strdup(line);
        i++;
    }
    number_of_keys = i;
    return line_array;
}

// ########################## MAIN FUNCTION #######################################
int do_func(char*input_text, int switch_int, char *in_file_str){

    FILE *fp1;
    char *e_key;
    char *text;
    char *detext;
    char **arrayLine_loc;
    char **arrayLine_loc_enc;
    char output_text[1024];
    char *collected_key;
    char *base64_encoded;
    char *base64_encoded_1;
    int bytes_to_decode = 0;

    fp1 = fopen(in_file_str,"r");
    //memset(base64_encoded, 0, sizeof(base64_encoded));

    // printf("input_text: %s\n", input_text);
    
    if (switch_int == 1){
        arrayLine_loc_enc = arrayLines(fp1);
        //printf("arrayLine_loc_enc: %s, len: %d\n", arrayLine_loc_enc[0], strlen(arrayLine_loc_enc[0]));
        e_key = base64decode(arrayLine_loc_enc[0], strlen(arrayLine_loc_enc[0]));
        //printf("ekey: %s\n", e_key);

        text = do_crypt(input_text, e_key);
        //printf("User Input: %s\n", input_text);
        //printf("Encryption: %s\n", text);
        char *data_to_encode;
        data_to_encode = text;

        int bytes_to_encode = encrypted_count; //Number of bytes in string to base64 encode.
        base64_encoded = base64encode(data_to_encode, bytes_to_encode);   //Base-64 encoding.
        bytes_to_decode = strlen(base64_encoded);
        //printf("base64_encoded: %s, %d\n", base64_encoded, encrypted_count);
        // printf("Base-64 encoded string is: %s\n", base64_encoded);  //Prints base64 encoded string.
        str_to_send = malloc(bytes_to_decode);
        memset(str_to_send, 0, bytes_to_decode);
        //printf("correct: %s, len bytes_to_decode %d\n", base64_encoded, bytes_to_decode);
        strcpy(str_to_send, base64_encoded);
       // printf("what happened?: %s, %d\n", str_to_send, bytes_to_decode);
        //free(base64_encoded);                //Frees up the memory holding our base64 encoded data.
    }
    else if (switch_int == 2){
        memset(base64_encoded, "\0", sizeof(base64_encoded));
        printf("info: %s, %d\n", input_text, strlen(input_text));
       // printf("len of input_text %d and base_64encoed %d\n",strlen(input_text), strlen(base64_encoded));
        strcpy(base64_encoded, input_text);
        //printf("base64decode: %s, %d\n", base64_encoded);

        bytes_to_decode = strlen(base64_encoded); //Number of bytes in string to base64 decode.
        printf("check 1a: %s, %d\n", base64_encoded, bytes_to_decode);
        char *base64_decoded = base64decode(base64_encoded, bytes_to_decode);   //Base-64 decoding.
        printf("Base-64 decoded string is: %s\n", base64_decoded);  //Prints base64 decoded string.


        //p1 = fopen("keys.txt","r");
        if(fp1 == NULL)
        {
            str_to_send = malloc(26);
            decrypted_count = 26;
            char* err_msg_1 = malloc(26);
            strcpy(err_msg_1, "Could not find key file.\n");
            strncpy(output_text, err_msg_1, 26);
            output_text[decrypted_count] = 0;
            //printf("str_to_send: %s", str_to_send);
            bzero(output_text, 26);
            free(err_msg_1);
            //printf("\nError reading file\n");
            return (void *) 0;
        }
        arrayLine_loc = arrayLines(fp1);
        //fclose(fp1);

        int i = 0;
        while (i < number_of_keys){
            collected_key = base64decode(arrayLine_loc[i], strlen(arrayLine_loc[i]));
            printf("collected_key: %s\n", collected_key);
            detext = do_decrypt(base64_decoded, encrypted_count, (unsigned char *)collected_key);
            if (detext != NULL){
                strncpy(output_text, detext, decrypted_count);
                output_text[decrypted_count] = 0;
                
                bzero(detext, decrypted_count);
                str_to_send = malloc(decrypted_count);
                break;
            }
            i++;
        }
        if (i == number_of_keys && i != 1){
            //printf("number_of_keys: %d\n", number_of_keys);
            //printf("should not\n");
            str_to_send = malloc(54);
            decrypted_count = 54;
            char* err_msg_1 = malloc(54);
            strcpy(err_msg_1, "Content could not be decrypted. Please use valid key\n");
            strncpy(output_text, err_msg_1, 54);
            output_text[decrypted_count] = 0;
            bzero(output_text, 54);
            free(err_msg_1);
        }
        
        
        memcpy(str_to_send, output_text, decrypted_count);
        free(base64_decoded);                //Frees up the memory holding our base64 decoded data.
        bzero(output_text, decrypted_count);
    }
    //printf("str_to_send %s\n", str_to_send);

    return 0;
}

char *get_output(){
    return str_to_send;
}
