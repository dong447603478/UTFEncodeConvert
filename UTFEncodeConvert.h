#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// 编码类型
typedef enum
{
    UTF_8,
    UTF_16
} EncodeType;

// 字节序类型
typedef enum
{
    LITTLE,
    BIG
} EndianType;

// UTF-8的BOM
extern const unsigned char UTF_8_BOM[3];

// UTF-16的BOM
extern const unsigned short UTF_16_BOM;

/*****************************************
** 是否UTF-8的BOM
******************************************/
bool IsUtf8BOM(unsigned char *src);

/*****************************************
** 是否UTF-16的BOM
******************************************/
bool IsUtf16BOM(unsigned char *src, EndianType *endianType);

/*****************************************
** 获取系统字节序
******************************************/
EndianType GetSystemEndian();

/*****************************************
** 返回码定义
******************************************/
extern const int SUCCESS;               // 参数
extern const int PARAM_NULL_PORINTER;  // 参数为空指针
extern const int BUF_SIZE_ZERO;        // 缓冲区大小为0
extern const int INVALID_CHARACTER;    // 非法字符
extern const int IMPERFECT_CHARACTER;  // 不完整字符

/*****************************************
** 转换UTF-8字符为Unicode码元
** unsigned char *buf       用于存放待转换的UTF-8编码单元
** size_t bufSize           buf的大小
** unsigned int *codePoint  存放转换后的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放被转换的UTF-8字符的编码单元数
**                          当返回码为INVALID_CHARACTER时返回参数存放非法字符编码单元相对buf的偏移量
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int Utf8ToUnicode(unsigned char *buf, size_t bufSize, unsigned int *codePoint, unsigned int *retParam);

/*****************************************
** 转换Unicode码元为UTF-8字符
** unsigned char *buf       用于存放转换后的UTF-8编码单元
** size_t bufSize           buf的大小
** unsigned int codePoint   要转换的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放转换后的UTF-8字符的编码单元数
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int UnicodeToUtf8(unsigned char *buf, size_t bufSize, unsigned int codePoint, unsigned int *retParam);

/*****************************************
** 转换UTF-16字符为Unicode码元
** unsigned short *buf      用于存放待转换的UTF-16编码单元
** size_t bufSize           buf的大小
** unsigned int *codePoint  存放转换后的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放被转换的UTF-16字符的编码单元数
**                          当返回码为INVALID_CHARACTER时返回参数存放非法字符编码单元相对buf的偏移量
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int Utf16ToUnicode(unsigned short *buf, size_t bufSize, unsigned int *codePoint, unsigned int *retParam);

/*****************************************
** 转换Unicode码元为UTF-16字符
** unsigned short *buf      用于存放转换后的UTF-16编码单元
** size_t bufSize           buf的大小
** unsigned int codePoint   要转换的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放转换后的UTF-16字符的编码单元数
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int UnicodeToUtf16(unsigned short *buf, size_t bufSize, unsigned int codePoint, unsigned int *retParam);

/*****************************************
** 从文件中读取UTF-8字符
** unsigned char *buf       用于存放读取的UTF-8编码单元
** size_t bufSize           buf的大小
** size_t *readLen          读取到UTF-8编码单元数
** FILE *file               读取的文件
******************************************/
bool ReadUtf8FromFile(unsigned char *buf, size_t bufSize, size_t *readLen, FILE *file);

/*****************************************
** 从文件中读取UTF-16字符
** unsigned short *buf      用于存放读取的UTF-8编码单元
** size_t bufSize           buf的大小
** size_t *readLen          读取到UTF-16编码单元数
** FILE *file               读取的文件
** EndianType endianType    字节序
******************************************/
bool ReadUtf16FromFile(unsigned short *buf, size_t bufSize, size_t *readLen, FILE *file, EndianType endianType);

/*****************************************
** 从文件中读取UTF字符
** void *buf                用于存放读取的UTF编码单元
** size_t bufSize           buf的大小
** size_t *readLen          读取到UTF编码单元数
** FILE *file               读取的文件
** EncodeType encodeType    UTF编码类型
** EndianType endianType    字节序
******************************************/
bool ReadUtfFromFile(void *buf, size_t bufSize, size_t *readLen, FILE *file, EncodeType encodeType, EndianType endianType);

/*****************************************
** UTF编码转换
** void *srcBuf                 输入缓冲区
** size_t srcBufSize            输入缓冲区的大小
** EncodeType srcEncodeType     输入源编码类型
** size_t *convertSrcSize       输入缓冲区被转换的大小
** void *destBuf                输出缓冲区
** size_t destBufSize           输出缓冲区的大小
** EncodeType destEncodeType    输出编码类型
** size_t *convertDestSize      转换到输出缓冲区的大小
******************************************/
bool UtfEncodeConvert(void *srcBuf, size_t srcBufSize, EncodeType srcEncodeType, size_t *convertSrcSize,
    void *destBuf, size_t destBufSize, EncodeType destEncodeType, size_t *convertDestSize);

/*****************************************
** 把UTF-8字符写入到文件
** unsigned char *buf       用于存放要写入的UTF-8编码单元
** size_t bufSize           buf的大小
** FILE *file               写入的文件
******************************************/
bool WriteUtf8ToFile(unsigned char *buf, size_t bufSize, FILE *file);

/*****************************************
** 把UTF-16字符写入到文件
** unsigned char *buf       用于存放要写入的UTF-16编码单元
** size_t bufSize           buf的大小
** FILE *file               写入的文件
** EndianType endianType    字节序
******************************************/
bool WriteUtf16ToFile(unsigned short *buf, size_t bufSize, FILE *file, EndianType endianType);

/*****************************************
** 把UTF字符写入到文件
** unsigned char *buf       用于存放要写入的UTF编码单元
** size_t bufSize           buf的大小
** FILE *file               写入的文件
** EncodeType encodeType    UTF编码类型
** EndianType endianType    字节序
******************************************/
bool WriteUtfToFile(void *buf, size_t bufSize, FILE *file, EncodeType encodeType, EndianType endianType);
