#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdint.h>


/*
    PNG magic number

    89	        Has the high bit set to detect transmission systems that do not support 8-bit data and to reduce the 
                    chance that a text file is mistakenly interpreted as a PNG, or vice versa.
    50 4E 47	In ASCII, the letters PNG, allowing a person to identify the format easily if it is viewed in a text editor.
    0D 0A	    A DOS-style line ending (CRLF) to detect DOS-Unix line ending conversion of the data.
    1A	        A byte that stops display of the file under DOS when the command type has been used—the end-of-file character.
    0A	        A Unix-style line ending (LF) to detect Unix-DOS line ending conversion.
*/ 
char PNG_MAGIC_NUMBER[8] = {0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A};

/*
    Should implement some kind of magic number, with an encoding resolution of 2, always
    Resolution: the number of low-order bits we overwrite in the encoding process, min 1 max 8. default 2
*/
typedef struct {
    short resolution;
} stegano_hdr_t;

/*
    length:     (4 bytes, big-endian) length of the following chunk
    chunk_type: Chunk types are given a four-letter case sensitive ASCII type/name. The case of the different letters in the 
                name (bit 5 of the numeric value of the character) is a bit field that provides the decoder with some information on 
                the nature of chunks it does not recognize.
*/
typedef struct
{
    int32_t length;
    char chunk_type[5];
} png_chunk_hdr_t;


/*
    Validates the input PNG by checking for the existance and correct formatting of the PNG magic number
*/
bool is_valid_png(int image_fd);

bool encode_secret_to_png(int image_fd, int secret_fd, int output_fd, int resolution);
bool decode_secret_from_png(int image_fd, int output_fd);