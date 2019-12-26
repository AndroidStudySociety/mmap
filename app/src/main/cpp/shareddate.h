//
// Created by Jesson on 2019-11-25.
//

#ifndef MMAP_SHAREDDATE_H
#define MMAP_SHAREDDATE_H

#include <jni.h>
#include <string>

#define TEXT_SZ 2048
typedef struct shared_use_st
{
    int written; // 非 0可读， 0表示 可写
    char text[TEXT_SZ]; // 记录 写入和 读取 的 文本
}shared_use_st;
#endif //MMAP_SHAREDDATE_H
