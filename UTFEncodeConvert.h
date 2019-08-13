#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

// ��������
typedef enum
{
    UTF_8,
    UTF_16
} EncodeType;

// �ֽ�������
typedef enum
{
    LITTLE,
    BIG
} EndianType;

// UTF-8��BOM
extern const unsigned char UTF_8_BOM[3];

// UTF-16��BOM
extern const unsigned short UTF_16_BOM;

/*****************************************
** �Ƿ�UTF-8��BOM
******************************************/
bool IsUtf8BOM(unsigned char *src);

/*****************************************
** �Ƿ�UTF-16��BOM
******************************************/
bool IsUtf16BOM(unsigned char *src, EndianType *endianType);

/*****************************************
** ��ȡϵͳ�ֽ���
******************************************/
EndianType GetSystemEndian();

/*****************************************
** �����붨��
******************************************/
extern const int SUCCESS;               // ����
extern const int PARAM_NULL_PORINTER;  // ����Ϊ��ָ��
extern const int BUF_SIZE_ZERO;        // ��������СΪ0
extern const int INVALID_CHARACTER;    // �Ƿ��ַ�
extern const int IMPERFECT_CHARACTER;  // �������ַ�

/*****************************************
** ת��UTF-8�ַ�ΪUnicode��Ԫ
** unsigned char *buf       ���ڴ�Ŵ�ת����UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int *codePoint  ���ת�����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز�����ű�ת����UTF-8�ַ��ı��뵥Ԫ��
**                          ��������ΪINVALID_CHARACTERʱ���ز�����ŷǷ��ַ����뵥Ԫ���buf��ƫ����
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
******************************************/
int Utf8ToUnicode(unsigned char *buf, size_t bufSize, unsigned int *codePoint, unsigned int *retParam);

/*****************************************
** ת��Unicode��ԪΪUTF-8�ַ�
** unsigned char *buf       ���ڴ��ת�����UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int codePoint   Ҫת����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز������ת�����UTF-8�ַ��ı��뵥Ԫ��
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
******************************************/
int UnicodeToUtf8(unsigned char *buf, size_t bufSize, unsigned int codePoint, unsigned int *retParam);

/*****************************************
** ת��UTF-16�ַ�ΪUnicode��Ԫ
** unsigned short *buf      ���ڴ�Ŵ�ת����UTF-16���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int *codePoint  ���ת�����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز�����ű�ת����UTF-16�ַ��ı��뵥Ԫ��
**                          ��������ΪINVALID_CHARACTERʱ���ز�����ŷǷ��ַ����뵥Ԫ���buf��ƫ����
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
******************************************/
int Utf16ToUnicode(unsigned short *buf, size_t bufSize, unsigned int *codePoint, unsigned int *retParam);

/*****************************************
** ת��Unicode��ԪΪUTF-16�ַ�
** unsigned short *buf      ���ڴ��ת�����UTF-16���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int codePoint   Ҫת����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز������ת�����UTF-16�ַ��ı��뵥Ԫ��
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
******************************************/
int UnicodeToUtf16(unsigned short *buf, size_t bufSize, unsigned int codePoint, unsigned int *retParam);

/*****************************************
** ���ļ��ж�ȡUTF-8�ַ�
** unsigned char *buf       ���ڴ�Ŷ�ȡ��UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** size_t *readLen          ��ȡ��UTF-8���뵥Ԫ��
** FILE *file               ��ȡ���ļ�
******************************************/
bool ReadUtf8FromFile(unsigned char *buf, size_t bufSize, size_t *readLen, FILE *file);

/*****************************************
** ���ļ��ж�ȡUTF-16�ַ�
** unsigned short *buf      ���ڴ�Ŷ�ȡ��UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** size_t *readLen          ��ȡ��UTF-16���뵥Ԫ��
** FILE *file               ��ȡ���ļ�
** EndianType endianType    �ֽ���
******************************************/
bool ReadUtf16FromFile(unsigned short *buf, size_t bufSize, size_t *readLen, FILE *file, EndianType endianType);

/*****************************************
** ���ļ��ж�ȡUTF�ַ�
** void *buf                ���ڴ�Ŷ�ȡ��UTF���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** size_t *readLen          ��ȡ��UTF���뵥Ԫ��
** FILE *file               ��ȡ���ļ�
** EncodeType encodeType    UTF��������
** EndianType endianType    �ֽ���
******************************************/
bool ReadUtfFromFile(void *buf, size_t bufSize, size_t *readLen, FILE *file, EncodeType encodeType, EndianType endianType);

/*****************************************
** UTF����ת��
** void *srcBuf                 ���뻺����
** size_t srcBufSize            ���뻺�����Ĵ�С
** EncodeType srcEncodeType     ����Դ��������
** size_t *convertSrcSize       ���뻺������ת���Ĵ�С
** void *destBuf                ���������
** size_t destBufSize           ����������Ĵ�С
** EncodeType destEncodeType    �����������
** size_t *convertDestSize      ת��������������Ĵ�С
******************************************/
bool UtfEncodeConvert(void *srcBuf, size_t srcBufSize, EncodeType srcEncodeType, size_t *convertSrcSize,
    void *destBuf, size_t destBufSize, EncodeType destEncodeType, size_t *convertDestSize);

/*****************************************
** ��UTF-8�ַ�д�뵽�ļ�
** unsigned char *buf       ���ڴ��Ҫд���UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** FILE *file               д����ļ�
******************************************/
bool WriteUtf8ToFile(unsigned char *buf, size_t bufSize, FILE *file);

/*****************************************
** ��UTF-16�ַ�д�뵽�ļ�
** unsigned char *buf       ���ڴ��Ҫд���UTF-16���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** FILE *file               д����ļ�
** EndianType endianType    �ֽ���
******************************************/
bool WriteUtf16ToFile(unsigned short *buf, size_t bufSize, FILE *file, EndianType endianType);

/*****************************************
** ��UTF�ַ�д�뵽�ļ�
** unsigned char *buf       ���ڴ��Ҫд���UTF���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** FILE *file               д����ļ�
** EncodeType encodeType    UTF��������
** EndianType endianType    �ֽ���
******************************************/
bool WriteUtfToFile(void *buf, size_t bufSize, FILE *file, EncodeType encodeType, EndianType endianType);
