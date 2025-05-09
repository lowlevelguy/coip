#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hamming.h"

/**
 * Calculates the required size of encoded data in bytes
 * 
 * @param input_size Size of the input data in bytes
 * @return Size needed for the encoded data in bytes
 */
size_t hamming_calculate_encoded_size(size_t input_size) {
    // For Hamming(7,4), each 4 bits of input becomes 7 bits of output
    // We process in bytes (8 bits), so each byte becomes 2 encoded 7-bit chunks (14 bits)
    // This means each byte becomes 2 bytes in the worst case (with padding)
    return (input_size * 2);
}

/**
 * Calculates the maximum size of decoded data based on encoded data size
 * 
 * @param encoded_size Size of the encoded data in bytes
 * @return Maximum possible size of the decoded data in bytes
 */
size_t hamming_calculate_decoded_size(size_t encoded_size) {
    // Each byte of decoded data comes from 2 encoded bytes (approximately)
    // This gives us a safe upper bound
    return (encoded_size / 2 + 1);
}

/**
 * Calculate parity bit for a given position in the data
 * 
 * @param data The data to calculate parity for
 * @param position Position of the parity bit
 * @return The parity bit value (0 or 1)
 */
uint8_t calculate_parity(uint8_t data, int position) {
    uint8_t parity = 0;
    
    // Calculate which bits this parity bit covers
    for (int i = 0; i < 7; i++) {
        // Check if this position should be covered by this parity bit
        // A position is covered if the binary representation of its position has a 1 in the parity bit's position
        if ((i & position) == position) {
            // This bit is covered by this parity bit
            parity ^= ((data >> i) & 1);
        }
    }
    
    return parity;
}

/**
 * Encode 4 bits of data into 7 bits using Hamming(7,4)
 * 
 * @param data 4-bit data to encode
 * @return 7-bit encoded data
 */
uint8_t hamming_encode_nibble(uint8_t data) {
    uint8_t encoded = 0;

    // Extract individual data bits
    uint8_t d3 = (data >> 0) & 1;  // Lowest bit
    uint8_t d5 = (data >> 1) & 1;
    uint8_t d6 = (data >> 2) & 1;
    uint8_t d7 = (data >> 3) & 1;  // Highest bit

    // Place data bits in their positions
    encoded |= (d3 << 2);  // Position 3
    encoded |= (d5 << 4);  // Position 5
    encoded |= (d6 << 5);  // Position 6
    encoded |= (d7 << 6);  // Position 7

    // Calculate parity bits
    uint8_t p1 = d3 ^ d5 ^ d7;
    uint8_t p2 = d3 ^ d6 ^ d7;
    uint8_t p4 = d5 ^ d6 ^ d7;

    // Place parity bits in their positions
    encoded |= (p1 << 0);  // Position 1
    encoded |= (p2 << 1);  // Position 2
    encoded |= (p4 << 3);  // Position 4

    return encoded;
}

/**
 * Encode data using Hamming(7,4)
 * 
 * @param input Pointer to input data
 * @param input_size Size of the input data in bytes
 * @param output Pointer to output buffer (must be pre-allocated)
 * @param output_size Pointer to store the actual output size
 * @return 0 on success, non-zero on error
 */
int hamming_encode(uint8_t* input, size_t input_size, uint8_t* output, size_t* output_size) {
    if (input == NULL || output == NULL || output_size == NULL) {
        return -1; // Invalid parameters
    }
    
    size_t output_idx = 0;
    
    for (size_t i = 0; i < input_size; i++) {
        // Process each byte of input as two 4-bit chunks
        uint8_t low_nibble = input[i] & 0x0F;         // Lower 4 bits
        uint8_t high_nibble = (input[i] >> 4) & 0x0F; // Upper 4 bits
        
        // Encode each nibble to 7 bits
        uint8_t encoded_low = hamming_encode_nibble(low_nibble);
        uint8_t encoded_high = hamming_encode_nibble(high_nibble);
        
        // Store the encoded nibbles
        output[output_idx++] = encoded_low;
        output[output_idx++] = encoded_high;
    }
    
    *output_size = output_idx;
    return 0; // Success
}

/**
 * Decode 7-bit Hamming code to 4-bit data
 * 
 * @param encoded 7-bit encoded data
 * @return 4-bit decoded data
 */
uint8_t hamming_decode_nibble(uint8_t encoded) {
    // Extract individual bits
    uint8_t p1 = (encoded >> 0) & 1;  // Position 1
    uint8_t p2 = (encoded >> 1) & 1;  // Position 2
    uint8_t d3 = (encoded >> 2) & 1;  // Position 3
    uint8_t p4 = (encoded >> 3) & 1;  // Position 4
    uint8_t d5 = (encoded >> 4) & 1;  // Position 5
    uint8_t d6 = (encoded >> 5) & 1;  // Position 6
    uint8_t d7 = (encoded >> 6) & 1;  // Position 7

    // Calculate syndrome (error location)
    uint8_t s1 = p1 ^ d3 ^ d5 ^ d7;
    uint8_t s2 = p2 ^ d3 ^ d6 ^ d7;
    uint8_t s4 = p4 ^ d5 ^ d6 ^ d7;

    uint8_t syndrome = (s4 << 2) | (s2 << 1) | s1;

    // Correct error if syndrome is non-zero
    if (syndrome != 0) {
        // Flip the bit at position indicated by syndrome
        printf("Hamming: Error detected at bit position %d. Fixing...\n", syndrome-1);
        encoded ^= (1 << (syndrome - 1));

        // Re-extract data bits after correction
        d3 = (encoded >> 2) & 1;
        d5 = (encoded >> 4) & 1;
        d6 = (encoded >> 5) & 1;
        d7 = (encoded >> 6) & 1;
    }

    // Reconstruct data
    uint8_t data = (d7 << 3) | (d6 << 2) | (d5 << 1) | d3;
    return data;
}

/**
 * Decode Hamming encoded data
 * 
 * @param input Pointer to encoded input data
 * @param input_size Size of the encoded data in bytes
 * @param output Pointer to output buffer (must be pre-allocated)
 * @param output_size Pointer to store the actual output size
 * @return 0 on success, non-zero on error
 */
int hamming_decode(uint8_t* input, size_t input_size, uint8_t* output, size_t* output_size) {
    if (input == NULL || output == NULL || output_size == NULL) {
        return -1; // Invalid parameters
    }
    
    size_t output_idx = 0;
    
    // Process pairs of 7-bit codes
    for (size_t i = 0; i < input_size; i += 2) {
        uint8_t low_nibble, high_nibble;
        
        // Decode first 7-bit code to 4 bits
        low_nibble = hamming_decode_nibble(input[i]);
        
        // If we have a second 7-bit code in this pair
        if (i + 1 < input_size) {
            high_nibble = hamming_decode_nibble(input[i + 1]);
            
            // Combine into one byte
            output[output_idx++] = (high_nibble << 4) | low_nibble;
        } else {
            // Last 7-bit code, we have only a partial byte
            output[output_idx++] = low_nibble;
        }
    }
    
    *output_size = output_idx;
    return 0; // Success
}
