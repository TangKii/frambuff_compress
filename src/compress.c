#include "global.h"

uint32_t decompressArray(uint8_t* compressedArr, uint32_t compressedLength, uint8_t* buff) 
{
    uint32_t i = 0, decode_index = 0;

    while (i < compressedLength) {
        if (compressedArr[i] != 0) {
            // 非零元素，直接输出
            //printf("[%d]=0x%02x ", i, compressedArr[i]);
            buff[decode_index++] = compressedArr[i];
            i++;
        } else {
            // 遇到0，处理紧随其后的计数值
            uint16_t zeroCount = compressedArr[i+1] | (compressedArr[i+2] << 8);
            //printf("zero=%d ", zeroCount);
            for (uint16_t j = 0; j < zeroCount; j++) {;
                buff[decode_index++] = 0;
            }
            i+=3;
        }
    }

    return decode_index;
}

uint32_t compressArray(uint8_t* arr, uint32_t length, uint8_t* buff)
{
    uint32_t i = 0, encode_index = 0;
    
    while (i < length) {
        if (arr[i] != 0) {
            // 非零元素，直接输出
            //printf("0x%02x, ", arr[i]);
            buff[encode_index++] = arr[i];
            i++;
        } else {
            // 遇到0，计算连续的0的数量
            int zeroCount = 0;
            while (i < length && arr[i] == 0) {
                zeroCount++;
                i++;
            }
            // 输出零的数量
            //printf("0 count=%d %d %d %d %d %d\n", zeroCount, i, length,arr[i-1],arr[i], arr[i+1]);
            buff[encode_index++] = 0;
            buff[encode_index++] = zeroCount & 0xff;
            buff[encode_index++] = (zeroCount & 0xff00) >> 8;
        }
    }
    return encode_index;
}