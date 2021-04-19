#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include "stegano.h"


void Usage() {
    puts("./encoder IMG SECRET [OUTPUT_FILENAME] ");
}

bool is_valid_png(int image_fd) {
    char buf[8];
    lseek(image_fd, 0, SEEK_SET);
    read(image_fd, buf, sizeof(buf));
    return memcmp(buf, PNG_MAGIC_NUMBER, 8) == 0;
}

int main(int argc, char *argv[]) {
    // User must submit an IMG and Secret to program
    if(argc < 3) {
        Usage();
        exit(1);
    }
    
    int image_fd;
    int secret_fd;

    image_fd = open(argv[1], O_RDONLY);
    if(image_fd < 0) {
        perror("Image opening failed: ");
        exit(1);
    }

    secret_fd = open(argv[2], O_RDONLY);
    if(secret_fd < 0) {
        perror("Secret opening failed: ");
        close(image_fd);
        exit(1);
    }

    if(!encode_secret_to_png(image_fd, secret_fd, -1, 2)) {
        close(image_fd);
        close(secret_fd);
        exit(1);
    }

    close(image_fd);
    close(secret_fd);
    exit(0);
}


bool encode_secret_to_png(int image_fd, int secret_fd, int output_fd, int resolution) {
        
    if(!is_valid_png(image_fd)) {
        puts("Image supplied was not a valid png file.");
        return false;
    }

    int secret_nbytes = lseek(secret_fd, 0, SEEK_END);
    int image_nbytes = lseek(image_fd, 0, SEEK_END);
    
    int offset[INT16_MAX]; // TODO Do better than this.
    int length_of[INT16_MAX]; // For real though
    int num_offsets = 0; 
    int total_dat_len = 0;
    int cur_offset;
    double used_capacity = 0.0;
    // Image has been validated, now time to read the chunks
    lseek(image_fd, sizeof(PNG_MAGIC_NUMBER), SEEK_SET);
    puts("Parsing image...");

    do {
        png_chunk_hdr_t chunk_hdr;
        read(image_fd, &chunk_hdr.length, sizeof(int32_t));
        read(image_fd, chunk_hdr.chunk_type, sizeof(uint32_t));
        chunk_hdr.length = ntohl(chunk_hdr.length);
        
        // Check for data section 
        if (strcmp("IDAT", chunk_hdr.chunk_type) == 0) {
            offset[num_offsets] = cur_offset;
            length_of[num_offsets] = chunk_hdr.length;
            num_offsets++;
            total_dat_len += chunk_hdr.length;
        }

        lseek(output_fd, (chunk_hdr.length + 4), SEEK_CUR);
        cur_offset = lseek(image_fd, (chunk_hdr.length + 4), SEEK_CUR);

    } while(cur_offset < image_nbytes);

    // EVERYTHING SHALL BE DOUBLES!!!
    used_capacity = (((double)secret_nbytes * (double)resolution) / (double)total_dat_len) * 100;


    puts("");
    printf("Data chunks:\t\t\t%d\n", num_offsets);
    printf("Encodable bytes:\t\t%d\n", total_dat_len);
    printf("Bytes to encode:\t\t%d\n", secret_nbytes);
    printf("PNG Capacity Used:\t\t %f percent\n", used_capacity);



    return true;
}
