#include "decoding.h"

// Declares necessary variables for decoding
uint32_t char1;
uint32_t char2;
uint32_t char3;
uint8_t base1;
uint8_t base2;
uint8_t base3;
uint8_t base4;

// Uses math operations described in studio 12 to convert base64 character to 6 bits
uint8_t base64(char c) {
    if      (c >= 'A' && c <= 'Z') return c - 'A' + 0;
    else if (c >= 'a' && c <= 'z') return c - 'a' + 26;
    else if (c >= '0' && c <= '9') return c - '0' + 52;
    else if (c == '+') return 62;
    else if (c == '/') return 63;
    else return 255; // For debugging purposes
}

// Converts 4 chars to 3 bytes using bitmasks and shifts, returns uint32_t (3 bytes).
uint32_t decode(uint32_t string) {
    base1 = base64(string & 0xFF); // LSB 
    base2 = base64((string >> 8) & 0xFF); 
    base3 = base64((string >> 16) & 0xFF);
    base4 = base64((string >> 24) & 0xFF); // MSB
    char1 = ((base1 << 2) & 0b11111100) | ((base2 >> 4) & 0b00000011); // byte 0 
    char2 = ((base2 << 4) & 0b11110000) | ((base3 >> 2) & 0b00001111); // byte 1
    char3 = ((base3 << 6) & 0b11000000) | ((base4) & 0b00111111); // byte 2
    return (char1 << 16) | (char2 << 8) | (char3); // combines and returns uint32_t
}

uint8_t char_to_hex(char c) {
    if (c >= 'a' && c <= 'f') return c - 'a' + 10; // a - z
    else if (c >= '0' && c <= '9') return c - '0'; // 0 - 9
    else return 255;
}