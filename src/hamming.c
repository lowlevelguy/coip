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
    
    // Place data bits in positions 3, 5, 6, 7 (indices 2, 4, 5, 6)
    encoded |= ((data & 0x1) << 2);         // d1 to position 3
    encoded |= ((data & 0x2) << 3);         // d2 to position 5
    encoded |= ((data & 0x4) << 3);         // d3 to position 6
    encoded |= ((data & 0x8) << 3);         // d4 to position 7
    
    // Calculate parity bits
    uint8_t p1 = calculate_parity(encoded, 0x1);  // positions 1, 3, 5, 7
    uint8_t p2 = calculate_parity(encoded, 0x2);  // positions 2, 3, 6, 7
    uint8_t p4 = calculate_parity(encoded, 0x4);  // positions 4, 5, 6, 7
    
    // Place parity bits
    encoded |= p1;                // p1 at position 1
    encoded |= (p2 << 1);         // p2 at position 2
    encoded |= (p4 << 3);         // p4 at position 4
    
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
 * Detect and correct errors in a 7-bit Hamming code
 * 
 * @param encoded 7-bit encoded data
 * @return Corrected 7-bit data
 */
uint8_t hamming_correct(uint8_t encoded) {
    // Calculate syndrome by checking each parity bit
    uint8_t syndrome = 0;
    
    // Create a copy of encoded data with parity bits cleared for checking
    uint8_t data_for_check = encoded;
    
    // Clear original parity bits for checking
    uint8_t p1_actual = encoded & 0x1;         // Position 1
    uint8_t p2_actual = (encoded >> 1) & 0x1;  // Position 2
    uint8_t p4_actual = (encoded >> 3) & 0x1;  // Position 4
    
    // Clear the parity bits to calculate expected values
    data_for_check &= ~0x1;  // Clear p1
    data_for_check &= ~0x2;  // Clear p2
    data_for_check &= ~0x8;  // Clear p4
    
    // Calculate expected parity values
    uint8_t p1_expected = calculate_parity(data_for_check, 0x1);
    uint8_t p2_expected = calculate_parity(data_for_check, 0x2);
    uint8_t p4_expected = calculate_parity(data_for_check, 0x4);
    
    // Check each parity bit
    if (p1_actual != p1_expected) {
        syndrome |= 0x1;  // p1 error
    }
    if (p2_actual != p2_expected) {
        syndrome |= 0x2;  // p2 error
    }
    if (p4_actual != p4_expected) {
        syndrome |= 0x4;  // p4 error
    }
    
    // If syndrome is non-zero, there's an error at bit position 'syndrome'
    if (syndrome != 0) {
        // Flip the bit at position syndrome
        encoded ^= (1 << (syndrome - 1));
    }
    
    return encoded;
}

/**
 * Decode 7-bit Hamming code to 4-bit data
 * 
 * @param encoded 7-bit encoded data
 * @return 4-bit decoded data
 */
uint8_t hamming_decode_7bits(uint8_t encoded) {
    // Correct any single-bit errors
    uint8_t corrected = hamming_correct(encoded);
    
    // Extract data bits from positions 3, 5, 6, 7 (indices 2, 4, 5, 6)
    uint8_t data = 0;
    data |= ((corrected >> 2) & 0x1);       // bit 3 -> bit 0
    data |= ((corrected >> 4) & 0x1) << 1;  // bit 5 -> bit 1
    data |= ((corrected >> 5) & 0x1) << 2;  // bit 6 -> bit 2
    data |= ((corrected >> 6) & 0x1) << 3;  // bit 7 -> bit 3
    
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
        low_nibble = hamming_decode_7bits(input[i]);
        
        // If we have a second 7-bit code in this pair
        if (i + 1 < input_size) {
            high_nibble = hamming_decode_7bits(input[i + 1]);
            
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
