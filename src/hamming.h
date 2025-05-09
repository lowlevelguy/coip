#pragma once
#include <stdint.h>
#include <stdlib.h>

/**
 * Calculates the required size of encoded data in bytes
 * 
 * @param input_size Size of the input data in bytes
 * @return Size needed for the encoded data in bytes
 */
size_t hamming_calculate_encoded_size(size_t input_size);

/**
 * Calculates the maximum size of decoded data based on encoded data size
 * 
 * @param encoded_size Size of the encoded data in bytes
 * @return Maximum possible size of the decoded data in bytes
 */
size_t hamming_calculate_decoded_size(size_t encoded_size);

/**
 * Encode data using Hamming(7,4)
 * 
 * @param input Pointer to input data
 * @param input_size Size of the input data in bytes
 * @param output Pointer to output buffer (must be pre-allocated)
 * @param output_size Pointer to store the actual output size
 * @return 0 on success, non-zero on error
 */
int hamming_encode(uint8_t* input, size_t input_size, uint8_t* output, size_t* output_size);

/**
 * Decode Hamming encoded data
 * 
 * @param input Pointer to encoded input data
 * @param input_size Size of the encoded data in bytes
 * @param output Pointer to output buffer (must be pre-allocated)
 * @param output_size Pointer to store the actual output size
 * @return 0 on success, non-zero on error
 */
int hamming_decode(uint8_t* input, size_t input_size, uint8_t* output, size_t* output_size);
