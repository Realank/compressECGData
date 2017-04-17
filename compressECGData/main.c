//
//  main.c
//  compressECGData
//
//  Created by Realank on 2017/4/14.
//  Copyright © 2017年 Realank. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include "compressECGData.h"

int main(int argc, const char * argv[]) {
    if (argc != 4) {
        printf("please input files path as  arguments\n");
        return -1;
    }
    
    const char * readFilePath = argv[1];
    printf("read file : %s\n",readFilePath);
    const char * compressFilePath = argv[2];
    printf("compress file : %s\n",compressFilePath);
    const char * decompressFilePath = argv[3];
    printf("decompress file : %s\n",decompressFilePath);
    compressFile4bit(readFilePath,compressFilePath);
    decompressFile4bit(compressFilePath, decompressFilePath);
    return 0;
}
