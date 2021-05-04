
#include "stegano.h"


void Usage() {
    puts("./encoder <PATH_TO_IMG> <PATH_TO_SECRET> <OUTPUT_FILENAME>");
}

bool is_valid_png(int image_fd) {
    char buf[8];
    lseek(image_fd, 0, SEEK_SET);
    read(image_fd, buf, sizeof(buf));
    return memcmp(buf, PNG_MAGIC_NUMBER, 8) == 0;
}

void encode_byte_into_container(uint8_t secret_byte, uint8_t *container, uint8_t *result) {
    // Control variable for how many spaces to shift the secret by
    // Gonna start with a resolution of 2. Rn idk how to parameterize it
    int shift_counter = 6;
    for(int i = 0; i < 4; i++, shift_counter -= 2) {
        // shift the secret down to 2 bits
        uint8_t shifted = secret_byte >> shift_counter;
        // This is probably pointless, but we want the only two set bits to 
        // be the 2 lowest order bits
        uint8_t to_be_encoded = shifted & 0x3;
        // clear the 2 lowest order bits in the container
        uint8_t buffer_container = container[i] & 0xFC;
        // And OR them together to set the lowest order bits with the secret
        uint8_t result_buffer = buffer_container | to_be_encoded;
        result[i] = result_buffer;
    }
}

bool encode_secret_to_png(int image_fd, int secret_fd, int output_fd, int resolution) {
        
    if(!is_valid_png(image_fd)) {
        puts("Image supplied was not a valid png file.");
        return false;
    }

    int secret_nbytes = lseek(secret_fd, 0, SEEK_END);
    int image_nbytes = lseek(image_fd, 0, SEEK_END);
    
    int offset[INT16_MAX]; // TODO Do better than this.
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
            num_offsets++;
            total_dat_len += chunk_hdr.length;
        }

        lseek(output_fd, (chunk_hdr.length + 4), SEEK_CUR);
        cur_offset = lseek(image_fd, (chunk_hdr.length + 4), SEEK_CUR);

    } while(cur_offset < image_nbytes);

    char output_buffer[image_nbytes];
    lseek(secret_fd, 0, SEEK_SET);
    lseek(image_fd, 0, SEEK_SET);
    lseek(output_fd, 0, SEEK_SET);

    puts("Creating output image...");

    if(read(image_fd, &output_buffer, image_nbytes) < 0) {
        perror("Error reading image to buffer");
        exit(1);
    }
   
    if(write(output_fd, &output_buffer, image_nbytes) < 0) {
        perror("Error writing image to tmp result file from buffer");
        exit(1);
    }

    puts("Encoding secret into output image...");
    // Now that we have made a copy of the input image lets encode the secret
    char secret_buffer; 
    png_chunk_hdr_t chunk_hdr;
    for(int i = 0; i < num_offsets; i++) {

        // seek to the beginning of each offset
        lseek(image_fd, offset[i], SEEK_SET);
        // and the same spot in the output image
        lseek(output_fd, offset[i], SEEK_SET);

        // fill the chunk header struct
        read(image_fd, &chunk_hdr.length, sizeof(int32_t));
        off_t offset = read(image_fd, chunk_hdr.chunk_type, sizeof(uint32_t));
        // read forward to maintain the proper location
        lseek(output_fd, offset, SEEK_SET);

        chunk_hdr.length = ntohl(chunk_hdr.length);

        // We need to know how many bytes we can encode in this chunk. 2-bit resolution, so 
        int chunk_capacity = chunk_hdr.length / 4;
        // be careful not to go over here. 
        for(int k = 0; k < chunk_capacity; k++) {
            // read a byte from the secret
            read(secret_fd, &secret_buffer, sizeof(secret_buffer));
            // read 4 bytes from the chunk
            uint8_t container[4];
            uint8_t result_buffer[4];
            read(image_fd, container, 4);
            encode_byte_into_container(secret_buffer, container, result_buffer);
            // write(output_fd, result_buffer, 4);
        }
    }

    // EVERYTHING SHALL BE DOUBLES!!! (This is a comment that I look back on 3 weeks later and think "What the hell?")
    used_capacity = (((double)secret_nbytes * (double)resolution) / (double)total_dat_len) * 100;


    puts("");
    printf("Data chunks:\t\t\t%d\n", num_offsets);
    printf("Encodable bytes:\t\t%d\n", total_dat_len);
    printf("Bytes to encode:\t\t%d\n", secret_nbytes);
    printf("PNG Capacity Used:\t\t %f percent\n", used_capacity);

    return true;
}


int main(int argc, char *argv[]) {
    // User must submit an IMG and Secret to program
    if(argc < 4) {
        Usage();
        exit(1);
    }
    
    int image_fd;
    int secret_fd;
    int output_fd;

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

    output_fd = open(argv[3], O_CREAT | O_RDWR, (S_IRUSR | S_IWUSR));
    if(output_fd < 0) {
        perror("Failed to create output file");
        close(image_fd);
        close(secret_fd);
        exit(1);
    }


    if(!encode_secret_to_png(image_fd, secret_fd, output_fd, 2)) {
        close(image_fd);
        close(secret_fd);
        close(output_fd);
        exit(1);
    }

    close(image_fd);
    close(secret_fd);
    close(output_fd);
    exit(0);
}

