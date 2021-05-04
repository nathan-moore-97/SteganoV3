#include "stegano.h"

void bin(unsigned n)
{
    printf("0x%02x\t", n);
    unsigned i;
    int counter = 0;
    for (i = 1 << 7; i > 0; i = i / 2) {
        (n & i) ? printf("1") : printf("0");
        if(++counter % 4 == 0) {
            printf(" ");
        }
    }
    puts("");
}


uint8_t decode_result_from_container(uint8_t *container) {
    puts("Decoding...");
    printf("iter\tcount\tcon_byte\tsecret\tbuffer\tbuffer_bin\n");
    uint8_t buffer = 0;
    int shift_counter = 0;

    for (int i = 0; i < 4; i++) {
        // isolate the two bytes we care about
        uint8_t secret = container[i] & 0x3;
        buffer <<= 2;
        buffer |= secret;

        printf("%d\t", i); // Iterations
        printf("%d\t", shift_counter); // number of bits to shift by
        printf("0x%02x\t\t", container[i]); // container byte (pre-decoded)
        printf("0x%02x\t", secret); // isolated secret bytes
        bin(buffer);
    }
    
    return buffer;
}

void encode_byte_into_container(uint8_t secret_byte, uint8_t *container, uint8_t *result) {
    // Control variable for how many spaces to shift the secret by
    // Gonna start with a resolution of 2. Rn idk how to parameterize it
    int shift_counter = 6;
    puts("Encoding...");
    printf("iter\tcount\tsecret\tcon_byte\tshift_sec\tto_be_enc\tbuf\tres\n");
    for(int i = 0; i < 4; i++, shift_counter -= 2) {
        // shift the secret down to 2 bits
        uint8_t shifted = secret_byte >> shift_counter;
        // This is probably pointless, but we want the only two set bits to 
        // be the 2 lowest order bits
        uint8_t to_be_encoded = shifted & 0x3;
        // clear the 2 lowest order bits in the container
        uint8_t buffer_container = container[i] & 0xFC;
        // And OR them together to set the lowest order bits with the secret
        uint8_t resultBuffer = buffer_container | to_be_encoded;

        printf("%d\t%d\t0x%02x\t0x%02x\t\t0x%02x\t\t0x%02x\t\t0x%02x\t0x%02x"
            , i, shift_counter, secret_byte, container[i], shifted, to_be_encoded
            , buffer_container, resultBuffer);
        puts("");

        result[i] = resultBuffer;
    }
    puts("");
}

int main() {
    // Lets simulate reading one byte at a time, and what we do with that byte. 
    // First we need a byte of encodable data...
    uint8_t secret = 0x7d;
    // We will need resolution 8 / resolution bytes of container data to encode. 
    // Lets add them in sequence to make it really easy how to see the data changes
    uint8_t container[4] = {60, 61, 62, 63}; 
    uint8_t result[4];

    encode_byte_into_container(secret, container, result);
    uint8_t decoded = decode_result_from_container(result);
    puts(""); // Drum roll please....

    printf("Secret:\n");
    bin(secret);
    printf("\nResults:\n");
    for(int i = 0; i < 4; i++) {
        bin(result[i]);
    }


    printf("\nDecoding result:\t0x%02x\n", decoded);
    return 0;
}