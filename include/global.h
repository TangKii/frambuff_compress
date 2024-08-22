#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

uint32_t decompressArray(uint8_t* compressedArr, uint32_t compressedLength, uint8_t* buff);
uint32_t compressArray(uint8_t* arr, uint32_t length, uint8_t* buff);

#endif