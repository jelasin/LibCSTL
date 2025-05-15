#include "DJB2Hash.h"

// DJB2 hash function implementation
unsigned int DJB2Hash(const char *str) {
    unsigned int hash = 5381; // Initialize hash value
    int c;

    // Iterate through each character in the string
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }

    return hash; // Return the computed hash value
}