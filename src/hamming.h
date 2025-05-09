#pragma once
#include <stdint.h>
#include <stdlib.h>

size_t hamming_calculate_encoded_size(size_t input_size);
size_t hamming_calculate_decoded_size(size_t encoded_size);
int hamming_encode(uint8_t* input, size_t input_size, uint8_t* output, size_t* output_size);
int hamming_decode(uint8_t* input, size_t input_size, uint8_t* output, size_t* output_size);
