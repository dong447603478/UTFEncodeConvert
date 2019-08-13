#include "UTFEncodeConvert.h"

// 截取code的从begin开始的bitLen个位
static unsigned int CutBit(unsigned int code, unsigned int begin, unsigned int bitLen)
{
    code >>= begin;
    unsigned int maskBit = 0;
    for (unsigned int i = 0; i < bitLen; i++)
    {
        maskBit <<= 1;
        maskBit++;
    }
    code &= maskBit;
    return code;
}

// 变换字节序
static void ChangeEndian(unsigned short *code)
{
    unsigned char *p = (unsigned char *)code;
    unsigned char temp = p[0];
    p[0] = p[1];
    p[1] = temp;
}

// UTF-8的BOM
const unsigned char UTF_8_BOM[3] = { 0xEF, 0xBB, 0xBF };

// UTF-16的BOM
const unsigned short UTF_16_BOM = 0xFEFF;

/*****************************************
** 是否UTF-8的BOM
******************************************/
bool IsUtf8BOM(unsigned char* src)
{
    return src[0] == UTF_8_BOM[0] && src[1] == UTF_8_BOM[1] && src[2] == UTF_8_BOM[2];
}

/*****************************************
** 是否UTF-16的BOM
******************************************/
bool IsUtf16BOM(unsigned char *src, EndianType *endianType)
{
    if (src[0] == 0xFE && src[1] == 0xFF)
    {
        *endianType = BIG;
        return true;
    }
    else if (src[0] == 0xFF && src[1] == 0xFE)
    {
        *endianType = LITTLE;
        return true;
    }

    return false;
}

/*****************************************
** 获取系统字节序
******************************************/
EndianType GetSystemEndian()
{
    unsigned int a = 1;
    unsigned char *p = (unsigned char *)&a;
    return (*p == 1 ? LITTLE : BIG);
}

/*****************************************
** 返回码定义
******************************************/
const int SUCCESS = 0;               // 参数
const int PARAM_NULL_PORINTER = -1;  // 参数为空指针
const int BUF_SIZE_ZERO = -2;        // 缓冲区大小为0
const int INVALID_CHARACTER = -3;    // 非法字符
const int IMPERFECT_CHARACTER = -4;  // 不完整字符

/*****************************************
** 获取系统字节序
** unsigned char *buf       用于存放待转换的UTF-8编码单元
** size_t bufSize           buf的大小
** unsigned int *codePoint  存放转换后的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放被转换的UTF-8字符的编码单元数
**                          当返回码为INVALID_CHARACTER时返回参数存放非法字符编码单元相对buf的偏移量
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int Utf8ToUnicode(unsigned char *buf, size_t bufSize, unsigned int *codePoint, unsigned int *retParam)
{
    if (buf == NULL || codePoint == NULL || retParam == NULL)
    {
        return PARAM_NULL_PORINTER;
    }

    if (bufSize == 0)
    {
        return BUF_SIZE_ZERO;
    }

    unsigned char firstCode = buf[0];
    if (firstCode <= 0x7F)
    {
        *retParam = 1;
        *codePoint = firstCode;
    }
    else
    {
        size_t codeCount = 0;
        while ((firstCode & 0x80) != 0)
        {
            codeCount++;
            firstCode <<= 1;
        }

        if (codeCount > 6)
        {
            *retParam = 0;
            return INVALID_CHARACTER;
        }

        unsigned int codePointTmp = CutBit(buf[0], 0, 7 - codeCount);
        size_t realCount = bufSize < codeCount ? bufSize : codeCount;
        for (size_t i = 1; i < realCount; i++)
        {
            if ((buf[i] & 0xC0) != 0x80)
            {
                *retParam = i;
                return INVALID_CHARACTER;
            }
            codePointTmp <<= 6;
            codePointTmp += CutBit(buf[i], 0, 6);
        }

        if (bufSize < codeCount)
        {
            *retParam = codeCount - bufSize;
            return IMPERFECT_CHARACTER;
        }

        *retParam = codeCount;
        *codePoint = codePointTmp;
    }

    return SUCCESS;
}

/*****************************************
** 转换Unicode码元为UTF-8字符
** unsigned char *buf       用于存放转换后的UTF-8编码单元
** size_t bufSize           buf的大小
** unsigned int codePoint   要转换的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放转换后的UTF-8字符的编码单元数
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int UnicodeToUtf8(unsigned char *buf, size_t bufSize, unsigned int codePoint, unsigned int *retParam)
{
    if (buf == NULL || retParam == NULL)
    {
        return PARAM_NULL_PORINTER;
    }

    if (bufSize == 0)
    {
        return BUF_SIZE_ZERO;
    }

    if ((codePoint >= 0xD800 && codePoint <= 0xDFFF) || codePoint > 0x10FFFF)
    {
        return INVALID_CHARACTER;
    }

    size_t codeCount = 0;
    if (codePoint <= 0x7F)
    {
        codeCount = 1;
    }
    else if (codePoint <= 0x7FF)
    {
        codeCount = 2;
    }
    else if (codePoint <= 0xFFFF)
    {
        codeCount = 3;
    }
    else
    {
        codeCount = 4;
    }

    if (codeCount == 1)
    {
        *retParam = 1;
        buf[0] = codePoint;
    }
    else
    {
        buf[0] = 0;
        for (size_t i = 0; i < codeCount; i++)
        {
            buf[0] = (buf[0] >> 1) + 0x80;
        }
        buf[0] += CutBit(codePoint, 6 * (codeCount - 1), 7 - codeCount);
        size_t realCount = bufSize < codeCount ? bufSize : codeCount;
        for (size_t i = 1; i < realCount; i++)
        {
            buf[i] = 0x80 + CutBit(codePoint, 6 * (codeCount - 1 - i), 6);
        }

        if (bufSize < codeCount)
        {
            *retParam = codeCount - bufSize;
            return IMPERFECT_CHARACTER;
        }

        *retParam = codeCount;
    }

    return SUCCESS;
}

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
int Utf16ToUnicode(unsigned short *buf, size_t bufSize, unsigned int *codePoint, unsigned int *retParam)
{
    if (buf == NULL || codePoint == NULL || retParam == NULL)
    {
        return PARAM_NULL_PORINTER;
    }

    if (bufSize == 0)
    {
        return BUF_SIZE_ZERO;
    }

    if ((buf[0] & 0xFC00) == 0xD800)
    {
        if (bufSize < 2)
        {
            *retParam = 1;
            return IMPERFECT_CHARACTER;
        }
        if ((buf[1] & 0xFC00) != 0xDC00)
        {
            *retParam = 1;
            return INVALID_CHARACTER;
        }
        *codePoint = (((buf[0] & 0x3FF) << 10) | (buf[1] & 0x3FF)) + 0x10000;
        *retParam = 2;
    }
    else if ((buf[0] & 0xFC00) == 0xDC00)
    {
        *retParam = 0;
        return INVALID_CHARACTER;
    }
    else
    {
        *codePoint = buf[0];
        *retParam = 1;
    }

    return SUCCESS;
}

/*****************************************
** 转换Unicode码元为UTF-16字符
** unsigned short *buf      用于存放转换后的UTF-16编码单元
** size_t bufSize           buf的大小
** unsigned int codePoint   要转换的Unicode码元
** unsigned int *retParam   返回参数
**                          当返回码为SUCCESS时返回参数存放转换后的UTF-16字符的编码单元数
**                          当返回码为IMPERFECT_CHARACTER时返回参数存放缺少的字符编码单元数
******************************************/
int UnicodeToUtf16(unsigned short *buf, size_t bufSize, unsigned int codePoint, unsigned int *retParam)
{
    if (buf == NULL || retParam == NULL)
    {
        return PARAM_NULL_PORINTER;
    }

    if (bufSize == 0)
    {
        return BUF_SIZE_ZERO;
    }

    if ((codePoint >= 0xD800 && codePoint <= 0xDFFF) || codePoint > 0x10FFFF)
    {
        return INVALID_CHARACTER;
    }

    if (codePoint <= 0xFFFF)
    {
        buf[0] = codePoint;
        *retParam = 1;
    }
    else
    {
        codePoint -= 0x10000;
        buf[0] = (codePoint >> 10) + 0xD800;
        if (bufSize < 2)
        {
            *retParam = 1;
            return IMPERFECT_CHARACTER;
        }
        buf[1] = (codePoint & 0x3FF) + 0xDC00;
        *retParam = 2;
    }

    return SUCCESS;
}

/*****************************************
** 从文件中读取UTF-8字符
** unsigned char *buf       用于存放读取的UTF-8编码单元
** size_t bufSize           buf的大小
** size_t *readLen          读取到UTF-8编码单元数
** FILE *file               读取的文件
******************************************/
bool ReadUtf8FromFile(unsigned char *buf, size_t bufSize, size_t *readLen, FILE *file)
{
    if (NULL == buf)
    {
        printf("ReadUtf8FromFile 传入的缓冲区buf为空指针\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("ReadUtf8FromFile 传入的缓冲区buf的大小为0\n");
        return false;
    }

    if (readLen == NULL)
    {
        printf("ReadUtf8FromFile 传入的输出参数readLen为空指针\n");
        return false;
    }

    if (file == NULL)
    {
        printf("ReadUtf8FromFile 传入的文件指针file为空指针\n");
        return false;
    }

    size_t usedLen = 0;
    while (usedLen < bufSize)
    {
        size_t readCurSize = fread(buf + usedLen, 1, 1, file);
        if (readCurSize < 1)
        {
            break;
        }

        unsigned int codePoint = 0;
        unsigned int retParam = 0;
        int retCode = Utf8ToUnicode(buf + usedLen, 1, &codePoint, &retParam);

        if (retCode == INVALID_CHARACTER)
        {
            printf("遇到非法的UTF-8字符编码: %x\n", (unsigned int)*(buf + usedLen + retParam));
            return false;
        }
        else if (retCode == IMPERFECT_CHARACTER)
        {
            if (retParam > (bufSize - usedLen - 1))
            {
                fseek(file, -1, SEEK_CUR);
                break;
            }

            readCurSize = fread(buf + usedLen + 1, 1, retParam, file);
            if (readCurSize < retParam)
            {
                printf("文件file不是以完整的UTF-8字符结束，缺少%u个编码单元\n", (unsigned int)(retParam - readCurSize));
                return false;
            }
            usedLen += (readCurSize + 1);
        }
        else
        {
            usedLen++;
        }
    }

    *readLen = usedLen;
    return true;
}

/*****************************************
** 从文件中读取UTF-16字符
** unsigned short *buf      用于存放读取的UTF-8编码单元
** size_t bufSize           buf的大小
** size_t *readLen          读取到UTF-16编码单元数
** FILE *file               读取的文件
** EndianType endianType    字节序
******************************************/
bool ReadUtf16FromFile(unsigned short *buf, size_t bufSize, size_t *readLen, FILE *file, EndianType endianType)
{
    if (NULL == buf)
    {
        printf("ReadUtf16FromFile 传入的缓冲区buf为空指针\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("ReadUtf16FromFile 传入的缓冲区buf的大小为0\n");
        return false;
    }

    if (readLen == NULL)
    {
        printf("ReadUtf16FromFile 传入的输出参数readLen为空指针\n");
        return false;
    }

    if (file == NULL)
    {
        printf("ReadUtf16FromFile 传入的文件指针file为空指针\n");
        return false;
    }

    size_t usedLen = 0;
    while (usedLen < bufSize)
    {
        size_t readCurSize = fread(buf + usedLen, 2, 1, file);
        if (readCurSize < 1)
        {
            break;
        }
        if (GetSystemEndian() != endianType)
        {
            ChangeEndian(buf + usedLen);
        }

        unsigned int codePoint = 0;
        unsigned int retParam = 0;
        int retCode = Utf16ToUnicode(buf + usedLen, 1, &codePoint, &retParam);

        if (retCode == INVALID_CHARACTER)
        {
            printf("遇到非法的UTF-16字符编码: %x\n", (unsigned int)*(buf + usedLen + retParam));
            return false;
        }
        else if (retCode == IMPERFECT_CHARACTER)
        {
            if (retParam > (bufSize - usedLen - 1))
            {
                fseek(file, -2, SEEK_CUR);
                break;
            }

            readCurSize = fread(buf + usedLen + 1, 2, retParam, file);
            if (readCurSize < retParam)
            {
                printf("文件file不是以完整的UTF-16字符结束，缺少%u个编码单元\n", (unsigned int)(retParam - readCurSize));
                return false;
            }
            if (GetSystemEndian() != endianType)
            {
                for (size_t i = 0; i < readCurSize; i++)
                {
                    ChangeEndian(buf + usedLen + 1 + i);
                }
            }
            usedLen += (readCurSize + 1);
        }
        else
        {
            usedLen++;
        }
    }

    *readLen = usedLen;
    return true;
}

/*****************************************
** 从文件中读取UTF字符
** void *buf                用于存放读取的UTF编码单元
** size_t bufSize           buf的大小
** size_t *readLen          读取到UTF编码单元数
** FILE *file               读取的文件
** EncodeType encodeType    UTF编码类型
** EndianType endianType    字节序
******************************************/
bool ReadUtfFromFile(void *buf, size_t bufSize, size_t *readLen, FILE *file, EncodeType encodeType, EndianType endianType)
{
    bool ret = true;
    if (encodeType == UTF_8)
    {
        ret = ReadUtf8FromFile(buf, bufSize, readLen, file);
    }
    else
    {
        bufSize /= 2;
        ret = ReadUtf16FromFile(buf, bufSize, readLen, file, endianType);
        if (ret)
        {
            *readLen = (*readLen) * 2;
        }
    }

    return ret;
}

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
    void *destBuf, size_t destBufSize, EncodeType destEncodeType, size_t *convertDestSize)
{
    if (srcBuf == NULL)
    {
        printf("UtfEncodeConvert 输入缓冲区srcBuf为空指针\n");
        return false;
    }

    if (srcBufSize == 0)
    {
        printf("UtfEncodeConvert 输入缓冲区srcBuf的大小为0\n");
        return false;
    }

    if (convertSrcSize == NULL)
    {
        printf("UtfEncodeConvert 参数convertSrcSize为空指针\n");
        return false;
    }

    if (destBuf == NULL)
    {
        printf("UtfEncodeConvert 输出缓冲区destBuf为空指针\n");
        return false;
    }

    if (destBufSize == 0)
    {
        printf("UtfEncodeConvert 输出缓冲区destBufSize的大小为0\n");
        return false;
    }

    if (convertDestSize == NULL)
    {
        printf("UtfEncodeConvert 参数convertDestSize为空指针\n");
        return false;
    }

    if (srcEncodeType == UTF_16)
    {
        srcBufSize /= 2;
    }

    if (destEncodeType == UTF_16)
    {
        destBufSize /= 2;
    }

    size_t convertSrcSizeTemp = 0;
    size_t convertDestSizeTemp = 0;
    while (convertSrcSizeTemp < srcBufSize && convertDestSizeTemp < destBufSize)
    {
        unsigned int unicode = 0;
        unsigned int retParam = 0;
        int retcode = 0;
        if (srcEncodeType == UTF_8)
        {
            retcode = Utf8ToUnicode((unsigned char *)srcBuf + convertSrcSizeTemp,
                srcBufSize - convertSrcSizeTemp, &unicode, &retParam);
            if (retcode == INVALID_CHARACTER)
            {
                printf("遇到非法的UTF-8字符编码: %x\n", (unsigned int)*((unsigned char *)srcBuf + convertSrcSizeTemp + retParam));
                return false;
            }
            else if (retcode == IMPERFECT_CHARACTER)
            {
                printf("遇到不完整的UTF-8字符，缺少%u个编码单元\n", retParam);
                return false;
            }
        }
        else
        {
            retcode = Utf16ToUnicode((unsigned short*)srcBuf + convertSrcSizeTemp,
                srcBufSize - convertSrcSizeTemp, &unicode, &retParam);
            if (retcode == INVALID_CHARACTER)
            {
                printf("遇到非法的UTF-16字符编码: %x\n", (unsigned int)*((unsigned short *)srcBuf + convertSrcSizeTemp + retParam));
                return false;
            }
            else if (retcode == IMPERFECT_CHARACTER)
            {
                printf("遇到不完整的UTF-16字符，缺少%u个编码单元\n", retParam);
                return false;
            }
        }
        unsigned int curConvertSrcSize = retParam;

        if (destEncodeType == UTF_8)
        {
            retcode = UnicodeToUtf8((unsigned char *)destBuf + convertDestSizeTemp,
                destBufSize - convertDestSizeTemp, unicode, &retParam);
        }
        else
        {
            retcode = UnicodeToUtf16((unsigned short *)destBuf + convertDestSizeTemp,
                destBufSize - convertDestSizeTemp, unicode, &retParam);
        }

        if (retcode == IMPERFECT_CHARACTER)
        {
            break;
        }
        else if (retcode == INVALID_CHARACTER)
        {
            printf("遇到非法的Unicode码元: %x\n", unicode);
            return false;
        }

        convertSrcSizeTemp += curConvertSrcSize;
        convertDestSizeTemp += retParam;
    }

    *convertSrcSize = (srcEncodeType == UTF_16 ? convertSrcSizeTemp * 2 : convertSrcSizeTemp);
    *convertDestSize = (destEncodeType == UTF_16 ? convertDestSizeTemp * 2 : convertDestSizeTemp);
    return true;
}

/*****************************************
** 把UTF-8字符写入到文件
** unsigned char *buf       用于存放要写入的UTF-8编码单元
** size_t bufSize           buf的大小
** FILE *file               写入的文件
******************************************/
bool WriteUtf8ToFile(unsigned char *buf, size_t bufSize, FILE *file)
{
    if (NULL == buf)
    {
        printf("WriteUtf8ToFile 传入的缓冲区buf为空指针\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("WriteUtf8ToFile 传入的缓冲区buf的大小为0\n");
        return false;
    }

    if (file == NULL)
    {
        printf("WriteUtf8ToFile 传入的文件指针file为空指针\n");
        return false;
    }

    size_t writeSize = fwrite(buf, 1, bufSize, file);
    if (writeSize < bufSize)
    {
        printf("写入UTF-8字符到文件出现异常，要写入的编码单元数是%u，实际写入数是%u\n", bufSize, writeSize);
        return false;
    }

    return true;
}

/*****************************************
** 把UTF-16字符写入到文件
** unsigned char *buf       用于存放要写入的UTF-16编码单元
** size_t bufSize           buf的大小
** FILE *file               写入的文件
** EndianType endianType    字节序
******************************************/
bool WriteUtf16ToFile(unsigned short *buf, size_t bufSize, FILE *file, EndianType endianType)
{
    if (NULL == buf)
    {
        printf("WriteUtf16ToFile 传入的缓冲区buf为空指针\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("WriteUtf16ToFile 传入的缓冲区buf的大小为0\n");
        return false;
    }

    if (file == NULL)
    {
        printf("WriteUtf16ToFile 传入的文件指针file为空指针\n");
        return false;
    }

    size_t writeSize = 0;
    while (writeSize < bufSize)
    {
        unsigned short code = buf[writeSize];
        if (GetSystemEndian() != endianType)
        {
            ChangeEndian(&code);
        }

        if (fwrite(&code, 2, 1, file) < 1)
        {
            printf("写入UTF-16字符到文件出现异常，要写入的编码单元数是%u，实际写入数是%u\n", bufSize, writeSize);
            return false;
        }
        writeSize++;
    }

    return true;
}

/*****************************************
** 把UTF字符写入到文件
** unsigned char *buf       用于存放要写入的UTF编码单元
** size_t bufSize           buf的大小
** FILE *file               写入的文件
** EncodeType encodeType    UTF编码类型
** EndianType endianType    字节序
******************************************/
bool WriteUtfToFile(void *buf, size_t bufSize, FILE *file, EncodeType encodeType, EndianType endianType)
{
    bool ret = true;
    if (encodeType == UTF_8)
    {
        ret = WriteUtf8ToFile(buf, bufSize, file);
    }
    else
    {
        bufSize /= 2;
        ret = WriteUtf16ToFile(buf, bufSize, file, endianType);
    }

    return ret;
}
