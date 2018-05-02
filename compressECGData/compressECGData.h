//
//  compressECGData.h
//  compressECGData
//
//  Created by Realank on 2017/4/17.
//  Copyright © 2017年 Realank. All rights reserved.
//

#ifndef compressECGData_h
#define compressECGData_h

#include <stdio.h>
#include <stdlib.h>

int compressFile4bit(const char* readFilePath,const char* writeFilePath);

int decompressFile4bit(const char* readFilePath,const char* writeFilePath);

//int compressFile8bit(const char* readFilePath,const char* writeFilePath);
#endif /* compressECGData_h */
