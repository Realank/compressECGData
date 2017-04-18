//
//  compressECGData.c
//  compressECGData
//
//  Created by Realank on 2017/4/17.
//  Copyright © 2017年 Realank. All rights reserved.
//

#include "compressECGData.h"

#define BufferSize 1024*16
uint8_t queue = 0;
int position = 0;

int pushToQueue(uint8_t data, uint8_t* popData){
    
    if (position == 0) {
        queue = (data&0x0f);
        position++;
        return 0;
    }else{
        queue <<= 4;
        queue += (data&0x0f);
        *popData = queue;
        queue = 0;
        position = 0;
        return 1;
    }
}

int forcePopQueue(uint8_t* popData){
    if (position == 0) {
        //noting
        //reset
        queue = 0;
        position = 0;
        return 0;
        
    }else{
        queue <<= 4;
        *popData = queue;
        //reset
        queue = 0;
        position = 0;
        return 1;
    }
}

int compressFile4bit(const char* readFilePath,const char* writeFilePath){
    int ret = 1;
    
    FILE *readFp = fopen(readFilePath, "r");
    FILE *writeFp = fopen(writeFilePath, "w");
    if (!readFp || !writeFp) {
        printf("Can't open file\n");
        ret = 0;
        goto CLOSEFILE;
    }
    size_t fileLength = 0;
    size_t onceReadLength = 0;
    uint8_t* readBuffer = malloc(sizeof(uint8_t) * BufferSize);
    uint8_t* writeBuffer = malloc(sizeof(uint8_t) * BufferSize*2);
    if (readBuffer == NULL || writeBuffer == NULL) {
        printf("Memmory Alloc Fail\n");
        ret = 0;
        goto FREEMEM;
    }
    
    //header
    writeBuffer[0] = 0xf1;
    writeBuffer[1] = 0x04;
    writeBuffer[2] = 0x00;//sum high
    writeBuffer[3] = 0x00;//sum low
    writeBuffer[4] = 0x00;//TBD
    writeBuffer[5] = 0x00;//TBD
    if (fwrite(writeBuffer, sizeof(uint8_t), 6, writeFp) != 6) {
        printf("Write to file failed\n");
        ret = 0;
        goto FREEMEM;
    }
    
    uint16_t previousValue = 0;
    long largeMarginCount = 0;
    uint16_t sum = 0;
    long deltaCount = 0;//count the delta datas after full size data
    while ((onceReadLength = fread(readBuffer, sizeof(uint8_t), BufferSize, readFp))) {
        size_t writeLength = 0;
        
        for (int i = 0; i < onceReadLength; i+=2) {
            uint16_t ecgValue = (readBuffer[i] << 8) + readBuffer[i+1];
            int16_t delta = ecgValue - previousValue;
            
            if ((delta > 7 || delta < -7) || deltaCount >= BufferSize) {
                //pop full size data
                deltaCount = 0;
                largeMarginCount++;
                uint8_t dataToPush = 0;
                if (pushToQueue(0x8, &dataToPush)) {
                    writeBuffer[writeLength] = dataToPush;
                    writeLength ++;
                }
                if (forcePopQueue(&dataToPush)) {
                    writeBuffer[writeLength] = dataToPush;
                    writeLength ++;
                }
                
                writeBuffer[writeLength] = readBuffer[i];
                writeBuffer[writeLength+1] = readBuffer[i+1];
                writeLength += 2;
                
            }else{
                deltaCount++;
                uint8_t dataToPush = 0;
                if (pushToQueue(delta, &dataToPush)) {
                    writeBuffer[writeLength] = dataToPush;
                    writeLength += 1;
                }
                
            }
            previousValue = ecgValue;
            sum += ecgValue;
        }
        fileLength += onceReadLength;
        size_t writtenLength = fwrite(writeBuffer, sizeof(uint8_t), writeLength, writeFp);
        if (writtenLength != writeLength) {
            printf("Write to file failed\n");
            ret = 0;
            goto FREEMEM;
        }
    }
    
    uint8_t dataToPush = 0;
    size_t writeLength = 0;
    // end data
    if (pushToQueue(0x8, &dataToPush)) {
        writeBuffer[writeLength] = dataToPush;
        writeLength ++;
    }
    if (forcePopQueue(&dataToPush)) {
        writeBuffer[writeLength] = dataToPush;
        writeLength ++;
    }
    if (fwrite(writeBuffer, sizeof(uint8_t), writeLength, writeFp) != writeLength) {
        printf("Write to file failed\n");
        ret = 0;
        goto FREEMEM;
    }
    
    //write sum
    if(fseek(writeFp, 2, SEEK_SET) != 0){
        printf("Seek file failed\n");
        ret = 0;
        goto FREEMEM;
    }
    writeBuffer[0] = sum / 0x100;
    writeBuffer[1] = sum % 0x100;
    if (fwrite(writeBuffer, sizeof(uint8_t), 2, writeFp) != 2) {
        printf("Write to file failed\n");
        ret = 0;
        goto FREEMEM;
    }
FREEMEM:
    free(readBuffer);
    free(writeBuffer);
    printf("file length: %ld\n",fileLength);
    printf("large margin count: %ld\n",largeMarginCount);
CLOSEFILE:
    fclose(readFp);
    fclose(writeFp);
    return ret;
}


int decompressFile4bit(const char* readFilePath,const char* writeFilePath){
    int ret = 1;
    
    FILE *readFp = fopen(readFilePath, "r");
    FILE *writeFp = fopen(writeFilePath, "w");
    if (!readFp || !writeFp) {
        printf("Can't open file\n");
        ret = 0;
        goto CLOSEFILE;
    }
    size_t fileLength = 0;
    size_t onceReadLength = 0;
    uint8_t* readBuffer = malloc(sizeof(uint8_t) * BufferSize);
    uint8_t* writeBuffer = malloc(sizeof(uint8_t) * BufferSize * 4);
    if (readBuffer == NULL || writeBuffer == NULL) {
        printf("Memmory Alloc Fail\n");
        ret = 0;
        goto FREEMEM;
    }
    
    //header
    
    onceReadLength = fread(readBuffer, sizeof(uint8_t), 6, readFp);
    if (onceReadLength != 6) {
        printf("Read failed\n");
        ret = 0;
        goto FREEMEM;
    }
    if (readBuffer[0] != 0xf1 || readBuffer[1] != 0x04) {
        printf("Data Format Error\n");
        ret = 0;
        goto FREEMEM;
    }
    uint16_t checkSum = (readBuffer[2] << 8) + readBuffer[3];
    
    uint16_t previousValue = 0;
    uint16_t sum = 0;
    int nextIsFullSizeData = 0;
    int shouldUsePreviousBlockData = 0;
    uint8_t previousBlockData = 0x00;

    while ((onceReadLength = fread(readBuffer, sizeof(uint8_t), BufferSize, readFp))) {
        size_t writeLength = 0;
        
        for (int i = 0; i < onceReadLength; i++) {
            
            uint8_t data = readBuffer[i];
            
            if (nextIsFullSizeData) {
                //full size data
                if (!shouldUsePreviousBlockData) {
                    shouldUsePreviousBlockData = 1;
                    previousBlockData = data;
                }else{
                    writeBuffer[writeLength] = previousBlockData;
                    writeBuffer[writeLength+1] = data;
                    writeLength += 2;
                    uint16_t ecgValue = (previousBlockData << 8) + data;
                    previousValue = ecgValue;
                    sum += ecgValue;
                    
                    //reset flag
                    previousBlockData = 0x00;
                    shouldUsePreviousBlockData = 0;
                    nextIsFullSizeData = 0;
                }
                continue;
            }
            
            uint8_t high4Bit = (data&0xf0) >> 4;
            uint8_t low4Bit = (data&0x0f);
            
            if (high4Bit == 0x8) {
                nextIsFullSizeData = 1;
                continue;
            }

            int16_t delta = (high4Bit > 7 ? high4Bit - 0x10 : high4Bit);
            uint16_t ecgValue = (previousValue + delta);
            previousValue = ecgValue;
            sum += ecgValue;
            writeBuffer[writeLength] = ecgValue / 0x100;
            writeBuffer[writeLength + 1] = ecgValue % 0x100;
            writeLength += 2;
            
            if (low4Bit == 0x8) {
                nextIsFullSizeData = 1;
                continue;
            }
            
            delta = (low4Bit > 7 ? low4Bit - 0x10 : low4Bit);
            ecgValue = (previousValue + delta);
            previousValue = ecgValue;
            sum += ecgValue;
            writeBuffer[writeLength] = ecgValue / 0x100;
            writeBuffer[writeLength + 1] = ecgValue % 0x100;
            writeLength += 2;
            
            
        }
        fileLength += writeLength;
        size_t writtenLength = fwrite(writeBuffer, sizeof(uint8_t), writeLength, writeFp);
        if (writtenLength != writeLength) {
            printf("Write to file failed\n");
            ret = 0;
            goto FREEMEM;
        }
    }
    
    if (checkSum != sum) {
        printf("Sum Check Failed\n");
        ret = 0;
        goto FREEMEM;
    }
FREEMEM:
    free(readBuffer);
    free(writeBuffer);
    printf("file length: %ld\n",fileLength);
CLOSEFILE:
    fclose(readFp);
    fclose(writeFp);
    return ret;
}


//int compressFile8bit(const char* readFilePath,const char* writeFilePath){
//    int ret = 1;
//    FILE *readFp = fopen(readFilePath, "r");
//    FILE *writeFp = fopen(writeFilePath, "w");
//    if (!readFp || !writeFp) {
//        printf("Can't open file\n");
//        ret = 0;
//        goto CLOSEFILE;
//    }
//    size_t fileLength = 0;
//    size_t onceReadLength = 0;
//    uint8_t* readBuffer = malloc(sizeof(uint8_t) * BufferSize);
//    uint8_t* writeBuffer = malloc(sizeof(uint8_t) * BufferSize);
//    if (!readBuffer || !writeBuffer) {
//        printf("Memmory Alloc Fail\n");
//        ret = 0;
//        goto FREEMEM;
//    }
//    
//    //header
//    writeBuffer[0] = 0xf1;
//    writeBuffer[1] = 0x08;
//    if (fwrite(writeBuffer, sizeof(uint8_t), 2, writeFp) != 2) {
//        printf("Write to file failed\n");
//        ret = 0;
//        goto FREEMEM;
//    }
//    
//    uint16_t previousValue = 0;
//    long largeMarginCount = 0;
//    uint16_t sum = 0;
//    while ((onceReadLength = fread(readBuffer, sizeof(uint8_t), BufferSize, readFp))) {
//        size_t writeLength = 0;
//        
//        for (int i = 0; i < onceReadLength; i+=2) {
//            uint16_t ecgValue = (readBuffer[i] << 8) + readBuffer[i+1];
//            int16_t delta = ecgValue - previousValue;
//            
//            if (delta > 127 || delta < -127) {
//                largeMarginCount++;
//                writeBuffer[writeLength] = 0x80;
//                writeLength+=1;
//                if (writeLength%2 == 1) {
//                    //need to fill a 0x00 to align
//                    writeBuffer[writeLength] = 0x00;
//                    writeLength+=1;
//                }
//                writeBuffer[writeLength] = readBuffer[i*2];
//                writeBuffer[writeLength+1] = readBuffer[i*2+1];
//                writeLength += 2;
//            }else{
//                writeBuffer[writeLength] = delta;
//                writeLength += 1;
//            }
//            previousValue = ecgValue;
//            sum += ecgValue;
//        }
//        fileLength += onceReadLength;
//        size_t writtenLength = fwrite(writeBuffer, sizeof(uint8_t), writeLength, writeFp);
//        if (writtenLength != writeLength) {
//            printf("Write to file failed\n");
//        }
//    }
//    writeBuffer[0] = sum / 0x100;
//    writeBuffer[1] = sum % 0x100;
//    if (fwrite(writeBuffer, sizeof(uint8_t), 2, writeFp) != 2) {
//        printf("Write to file failed\n");
//        ret = 0;
//        goto FREEMEM;
//    }
//FREEMEM:
//    free(readBuffer);
//    free(writeBuffer);
//    printf("\n file length: %ld\n",fileLength);
//    printf("large margin count: %ld\n",largeMarginCount);
//CLOSEFILE:
//    fclose(readFp);
//    fclose(writeFp);
//    return ret;
//}
