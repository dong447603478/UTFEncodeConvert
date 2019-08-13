#include "UTFEncodeConvert.h"

// ��ȡcode�Ĵ�begin��ʼ��bitLen��λ
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

// �任�ֽ���
static void ChangeEndian(unsigned short *code)
{
    unsigned char *p = (unsigned char *)code;
    unsigned char temp = p[0];
    p[0] = p[1];
    p[1] = temp;
}

// UTF-8��BOM
const unsigned char UTF_8_BOM[3] = { 0xEF, 0xBB, 0xBF };

// UTF-16��BOM
const unsigned short UTF_16_BOM = 0xFEFF;

/*****************************************
** �Ƿ�UTF-8��BOM
******************************************/
bool IsUtf8BOM(unsigned char* src)
{
    return src[0] == UTF_8_BOM[0] && src[1] == UTF_8_BOM[1] && src[2] == UTF_8_BOM[2];
}

/*****************************************
** �Ƿ�UTF-16��BOM
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
** ��ȡϵͳ�ֽ���
******************************************/
EndianType GetSystemEndian()
{
    unsigned int a = 1;
    unsigned char *p = (unsigned char *)&a;
    return (*p == 1 ? LITTLE : BIG);
}

/*****************************************
** �����붨��
******************************************/
const int SUCCESS = 0;               // ����
const int PARAM_NULL_PORINTER = -1;  // ����Ϊ��ָ��
const int BUF_SIZE_ZERO = -2;        // ��������СΪ0
const int INVALID_CHARACTER = -3;    // �Ƿ��ַ�
const int IMPERFECT_CHARACTER = -4;  // �������ַ�

/*****************************************
** ��ȡϵͳ�ֽ���
** unsigned char *buf       ���ڴ�Ŵ�ת����UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int *codePoint  ���ת�����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز�����ű�ת����UTF-8�ַ��ı��뵥Ԫ��
**                          ��������ΪINVALID_CHARACTERʱ���ز�����ŷǷ��ַ����뵥Ԫ���buf��ƫ����
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
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
** ת��Unicode��ԪΪUTF-8�ַ�
** unsigned char *buf       ���ڴ��ת�����UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int codePoint   Ҫת����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز������ת�����UTF-8�ַ��ı��뵥Ԫ��
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
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
** ת��UTF-16�ַ�ΪUnicode��Ԫ
** unsigned short *buf      ���ڴ�Ŵ�ת����UTF-16���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int *codePoint  ���ת�����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز�����ű�ת����UTF-16�ַ��ı��뵥Ԫ��
**                          ��������ΪINVALID_CHARACTERʱ���ز�����ŷǷ��ַ����뵥Ԫ���buf��ƫ����
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
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
** ת��Unicode��ԪΪUTF-16�ַ�
** unsigned short *buf      ���ڴ��ת�����UTF-16���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** unsigned int codePoint   Ҫת����Unicode��Ԫ
** unsigned int *retParam   ���ز���
**                          ��������ΪSUCCESSʱ���ز������ת�����UTF-16�ַ��ı��뵥Ԫ��
**                          ��������ΪIMPERFECT_CHARACTERʱ���ز������ȱ�ٵ��ַ����뵥Ԫ��
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
** ���ļ��ж�ȡUTF-8�ַ�
** unsigned char *buf       ���ڴ�Ŷ�ȡ��UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** size_t *readLen          ��ȡ��UTF-8���뵥Ԫ��
** FILE *file               ��ȡ���ļ�
******************************************/
bool ReadUtf8FromFile(unsigned char *buf, size_t bufSize, size_t *readLen, FILE *file)
{
    if (NULL == buf)
    {
        printf("ReadUtf8FromFile ����Ļ�����bufΪ��ָ��\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("ReadUtf8FromFile ����Ļ�����buf�Ĵ�СΪ0\n");
        return false;
    }

    if (readLen == NULL)
    {
        printf("ReadUtf8FromFile ������������readLenΪ��ָ��\n");
        return false;
    }

    if (file == NULL)
    {
        printf("ReadUtf8FromFile ������ļ�ָ��fileΪ��ָ��\n");
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
            printf("�����Ƿ���UTF-8�ַ�����: %x\n", (unsigned int)*(buf + usedLen + retParam));
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
                printf("�ļ�file������������UTF-8�ַ�������ȱ��%u�����뵥Ԫ\n", (unsigned int)(retParam - readCurSize));
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
** ���ļ��ж�ȡUTF-16�ַ�
** unsigned short *buf      ���ڴ�Ŷ�ȡ��UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** size_t *readLen          ��ȡ��UTF-16���뵥Ԫ��
** FILE *file               ��ȡ���ļ�
** EndianType endianType    �ֽ���
******************************************/
bool ReadUtf16FromFile(unsigned short *buf, size_t bufSize, size_t *readLen, FILE *file, EndianType endianType)
{
    if (NULL == buf)
    {
        printf("ReadUtf16FromFile ����Ļ�����bufΪ��ָ��\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("ReadUtf16FromFile ����Ļ�����buf�Ĵ�СΪ0\n");
        return false;
    }

    if (readLen == NULL)
    {
        printf("ReadUtf16FromFile ������������readLenΪ��ָ��\n");
        return false;
    }

    if (file == NULL)
    {
        printf("ReadUtf16FromFile ������ļ�ָ��fileΪ��ָ��\n");
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
            printf("�����Ƿ���UTF-16�ַ�����: %x\n", (unsigned int)*(buf + usedLen + retParam));
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
                printf("�ļ�file������������UTF-16�ַ�������ȱ��%u�����뵥Ԫ\n", (unsigned int)(retParam - readCurSize));
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
** ���ļ��ж�ȡUTF�ַ�
** void *buf                ���ڴ�Ŷ�ȡ��UTF���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** size_t *readLen          ��ȡ��UTF���뵥Ԫ��
** FILE *file               ��ȡ���ļ�
** EncodeType encodeType    UTF��������
** EndianType endianType    �ֽ���
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
    void *destBuf, size_t destBufSize, EncodeType destEncodeType, size_t *convertDestSize)
{
    if (srcBuf == NULL)
    {
        printf("UtfEncodeConvert ���뻺����srcBufΪ��ָ��\n");
        return false;
    }

    if (srcBufSize == 0)
    {
        printf("UtfEncodeConvert ���뻺����srcBuf�Ĵ�СΪ0\n");
        return false;
    }

    if (convertSrcSize == NULL)
    {
        printf("UtfEncodeConvert ����convertSrcSizeΪ��ָ��\n");
        return false;
    }

    if (destBuf == NULL)
    {
        printf("UtfEncodeConvert ���������destBufΪ��ָ��\n");
        return false;
    }

    if (destBufSize == 0)
    {
        printf("UtfEncodeConvert ���������destBufSize�Ĵ�СΪ0\n");
        return false;
    }

    if (convertDestSize == NULL)
    {
        printf("UtfEncodeConvert ����convertDestSizeΪ��ָ��\n");
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
                printf("�����Ƿ���UTF-8�ַ�����: %x\n", (unsigned int)*((unsigned char *)srcBuf + convertSrcSizeTemp + retParam));
                return false;
            }
            else if (retcode == IMPERFECT_CHARACTER)
            {
                printf("������������UTF-8�ַ���ȱ��%u�����뵥Ԫ\n", retParam);
                return false;
            }
        }
        else
        {
            retcode = Utf16ToUnicode((unsigned short*)srcBuf + convertSrcSizeTemp,
                srcBufSize - convertSrcSizeTemp, &unicode, &retParam);
            if (retcode == INVALID_CHARACTER)
            {
                printf("�����Ƿ���UTF-16�ַ�����: %x\n", (unsigned int)*((unsigned short *)srcBuf + convertSrcSizeTemp + retParam));
                return false;
            }
            else if (retcode == IMPERFECT_CHARACTER)
            {
                printf("������������UTF-16�ַ���ȱ��%u�����뵥Ԫ\n", retParam);
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
            printf("�����Ƿ���Unicode��Ԫ: %x\n", unicode);
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
** ��UTF-8�ַ�д�뵽�ļ�
** unsigned char *buf       ���ڴ��Ҫд���UTF-8���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** FILE *file               д����ļ�
******************************************/
bool WriteUtf8ToFile(unsigned char *buf, size_t bufSize, FILE *file)
{
    if (NULL == buf)
    {
        printf("WriteUtf8ToFile ����Ļ�����bufΪ��ָ��\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("WriteUtf8ToFile ����Ļ�����buf�Ĵ�СΪ0\n");
        return false;
    }

    if (file == NULL)
    {
        printf("WriteUtf8ToFile ������ļ�ָ��fileΪ��ָ��\n");
        return false;
    }

    size_t writeSize = fwrite(buf, 1, bufSize, file);
    if (writeSize < bufSize)
    {
        printf("д��UTF-8�ַ����ļ������쳣��Ҫд��ı��뵥Ԫ����%u��ʵ��д������%u\n", bufSize, writeSize);
        return false;
    }

    return true;
}

/*****************************************
** ��UTF-16�ַ�д�뵽�ļ�
** unsigned char *buf       ���ڴ��Ҫд���UTF-16���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** FILE *file               д����ļ�
** EndianType endianType    �ֽ���
******************************************/
bool WriteUtf16ToFile(unsigned short *buf, size_t bufSize, FILE *file, EndianType endianType)
{
    if (NULL == buf)
    {
        printf("WriteUtf16ToFile ����Ļ�����bufΪ��ָ��\n");
        return false;
    }

    if (bufSize == 0)
    {
        printf("WriteUtf16ToFile ����Ļ�����buf�Ĵ�СΪ0\n");
        return false;
    }

    if (file == NULL)
    {
        printf("WriteUtf16ToFile ������ļ�ָ��fileΪ��ָ��\n");
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
            printf("д��UTF-16�ַ����ļ������쳣��Ҫд��ı��뵥Ԫ����%u��ʵ��д������%u\n", bufSize, writeSize);
            return false;
        }
        writeSize++;
    }

    return true;
}

/*****************************************
** ��UTF�ַ�д�뵽�ļ�
** unsigned char *buf       ���ڴ��Ҫд���UTF���뵥Ԫ
** size_t bufSize           buf�Ĵ�С
** FILE *file               д����ļ�
** EncodeType encodeType    UTF��������
** EndianType endianType    �ֽ���
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
