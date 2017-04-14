//
//  main.c
//  compressECGData
//
//  Created by Realank on 2017/4/14.
//  Copyright © 2017年 Realank. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#define BufferSize 1024*4

void compressFile4bit(const char* readFilePath,const char* writeFilePath){
    FILE *readFp = fopen(readFilePath, "r");
    FILE *writeFp = fopen(writeFilePath, "w");
    if (!readFp || !writeFp) {
        printf("Can't open file\n");
        exit(1);
    }
    size_t fileLength = 0;
    size_t onceReadLength = 0;
    uint8_t* readBuffer = malloc(sizeof(uint8_t) * BufferSize);
    uint8_t* writeBuffer = malloc(sizeof(uint8_t) * BufferSize);
    if (!readBuffer || !writeBuffer) {
        printf("Memmory Alloc Fail\n");
        exit(1);
    }
    uint16_t previousValue = 0;
    long largeMarginCount = 0;
    uint16_t sum = 0;
    while ((onceReadLength = fread(readBuffer, sizeof(uint8_t), BufferSize, readFp))) {
        size_t writeLength = 0;
        
        for (int i = 0; i < onceReadLength; i+=2) {
            uint16_t ecgValue = (readBuffer[i] << 8) + readBuffer[i+1];
            int16_t delta = ecgValue - previousValue;
            
            if (delta > 127 || delta < -127) {
                largeMarginCount++;
                writeBuffer[writeLength] = 0x80;
                writeLength+=1;
                if (writeLength%2 == 1) {
                    //need to fill a 0x00 to align
                    writeBuffer[writeLength] = 0x00;
                    writeLength+=1;
                }
                writeBuffer[writeLength] = readBuffer[i*2];
                writeBuffer[writeLength+1] = readBuffer[i*2+1];
                writeLength += 2;
            }else{
                writeBuffer[writeLength] = delta;
                writeLength += 1;
            }
            previousValue = ecgValue;
            sum += ecgValue;
        }
        fileLength += onceReadLength;
        size_t writtenLength = fwrite(writeBuffer, sizeof(uint8_t), writeLength, writeFp);
        if (writtenLength != writtenLength) {
            printf("Write to file failed\n");
        }
    }
    writeBuffer[0] = sum / 0x100;
    writeBuffer[1] = sum % 0x100;
    if (fwrite(writeBuffer, sizeof(uint8_t), 2, writeFp) != 2) {
        printf("Write to file failed\n");
    }
    free(readBuffer);
    free(writeBuffer);
    printf("\n file length: %ld\n",fileLength);
    printf("large margin count: %ld\n",largeMarginCount);
    fclose(readFp);
    fclose(writeFp);
}


void compressFile8bit(const char* readFilePath,const char* writeFilePath){
    FILE *readFp = fopen(readFilePath, "r");
    FILE *writeFp = fopen(writeFilePath, "w");
    if (!readFp || !writeFp) {
        printf("Can't open file\n");
        exit(1);
    }
    size_t fileLength = 0;
    size_t onceReadLength = 0;
    uint8_t* readBuffer = malloc(sizeof(uint8_t) * BufferSize);
    uint8_t* writeBuffer = malloc(sizeof(uint8_t) * BufferSize);
    if (!readBuffer || !writeBuffer) {
        printf("Memmory Alloc Fail\n");
        exit(1);
    }
    uint16_t previousValue = 0;
    long largeMarginCount = 0;
    uint16_t sum = 0;
    while ((onceReadLength = fread(readBuffer, sizeof(uint8_t), BufferSize, readFp))) {
        size_t writeLength = 0;
        
        for (int i = 0; i < onceReadLength; i+=2) {
            uint16_t ecgValue = (readBuffer[i] << 8) + readBuffer[i+1];
            int16_t delta = ecgValue - previousValue;
            
            if (delta > 127 || delta < -127) {
                largeMarginCount++;
                writeBuffer[writeLength] = 0x80;
                writeLength+=1;
                if (writeLength%2 == 1) {
                    //need to fill a 0x00 to align
                    writeBuffer[writeLength] = 0x00;
                    writeLength+=1;
                }
                writeBuffer[writeLength] = readBuffer[i*2];
                writeBuffer[writeLength+1] = readBuffer[i*2+1];
                writeLength += 2;
            }else{
                writeBuffer[writeLength] = delta;
                writeLength += 1;
            }
            previousValue = ecgValue;
            sum += ecgValue;
        }
        fileLength += onceReadLength;
        size_t writtenLength = fwrite(writeBuffer, sizeof(uint8_t), writeLength, writeFp);
        if (writtenLength != writtenLength) {
            printf("Write to file failed\n");
        }
    }
    writeBuffer[0] = sum / 0x100;
    writeBuffer[1] = sum % 0x100;
    if (fwrite(writeBuffer, sizeof(uint8_t), 2, writeFp) != 2) {
        printf("Write to file failed\n");
    }
    free(readBuffer);
    free(writeBuffer);
    printf("\n file length: %ld\n",fileLength);
    printf("large margin count: %ld\n",largeMarginCount);
    fclose(readFp);
    fclose(writeFp);
}

int main(int argc, const char * argv[]) {
    if (argc != 3) {
        printf("please input files path as  arguments\n");
        return -1;
    }
    
    const char * readFilePath = argv[1];
    printf("read file : %s\n",readFilePath);
    const char * writeFilePath = argv[2];
    printf("write file : %s\n",writeFilePath);
    compressFile8bit(readFilePath,writeFilePath);
    return 0;
}
