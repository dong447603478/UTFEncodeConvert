#include <stdbool.h>
#include "UTFEncodeConvert.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ������ز���
EncodeType g_srcEncode;                 // �����������
bool g_setSrcEncode = false;            // �����������������
EndianType g_srcEndian;                 // �����ֽ���
bool g_setSrcEndian = false;            // �����������ֽ���
char *g_srcFilePath = NULL;             // �����ļ�·��
FILE *g_srcFile = NULL;                 // �����ļ�

// �����ز���
EncodeType g_destEncode;                // �����������
bool g_setDestEncode = false;           // �����������������
EndianType g_destEndian;                // ����ֽ���
bool g_setDestEndian = false;           // ����������ֽ���
char *g_destFilePath = NULL;            // ����ļ�·��
char *g_destTempFilePath = NULL;        // �����ʱ�ļ�·��
FILE* g_destFile = NULL;                // �����ļ�

// ��ӡ˵��
static void PrintHelp()
{
    printf("�����ʽ: UTFEncodeConvert <ѡ��> �����ļ� ����ļ�\n"
        "ѡ��˵��:\n"
        "-f         �����ʶ�����������ֵĲ���Ϊ�����ļ��Ĳ���\n"
        "-t         �����ʶ�����������ֵĲ���Ϊ����ļ��Ĳ���\n"
        "utf-8      �ַ�����Ϊutf-8\n"
        "utf-16     �ַ�����Ϊutf-16\n"
        "big        ����ֽ���\n"
        "little     С���ֽ���\n"
        "bom        ����ļ�д��BOM���ò�����������ļ���Ч��������д������ļ�����BOM\n"
        "��ǰ��û�г������������ʶ����Ĭ�ϵ�ǰ����Ϊ�������\n"
        "�������ļ���BOM���������ļ��ַ�������ֽ�����ʵ�������ļ�Ϊ׼\n");
}

// ���ļ�·���в����ļ���
static const char* FindName(const char *filePath)
{
    int pos = -1;
    for (int i = 0, size = strlen(filePath); i < size; i++)
    {
        if (filePath[i] == '/' || filePath[i] == '\\')
        {
            pos = i;
        }
    }
    return filePath + pos + 1;
}

// ���������ļ���BOM
static void DealSrcFileBOM()
{
    unsigned char buf[3] = { 0 };
    fread(buf, 1, 2, g_srcFile);
    EndianType endianType = LITTLE;
    if (IsUtf16BOM(buf, &endianType))
    {
        g_srcEncode = UTF_16;
        g_srcEndian = endianType;
        g_setSrcEncode = true;
        g_setSrcEndian = true;
        return;
    }

    fread(buf + 2, 1, 1, g_srcFile);
    if (IsUtf8BOM(buf))
    {
        g_srcEncode = UTF_8;
        g_setSrcEncode = true;
        return;
    }

    fseek(g_srcFile, 0, SEEK_SET);
}

// ��������ļ���BOM
static void DealDestFileBOM()
{
    if (g_destEncode == UTF_8)
    {
        fwrite(UTF_8_BOM, 1, 3, g_destFile);
    }
    else
    {
        unsigned char bom[2] = { 0 };
        memcpy(bom, &UTF_16_BOM, 2);
        if (GetSystemEndian() != g_destEndian)
        {
            unsigned char temp = bom[0];
            bom[0] = bom[1];
            bom[1] = temp;
        }
        fwrite(bom, 1, 2, g_destFile);
    }
}

// ��ʼ������
static bool InitializeContext(int argc, char *argv[])
{
    bool destParam = true;
    bool destBOM = false;
    for (int i = 1; i < argc; i++)
    {
        if (0 == strcmp(argv[i], "-f"))
        {
            destParam = false;
        }
        else if (0 == strcmp(argv[i], "-t"))
        {
            destParam = true;
        }
        else if (0 == strcmp(argv[i], "utf-8") || 0 == strcmp(argv[i], "utf-16"))
        {
            EncodeType *encode = destParam ? &g_destEncode : &g_srcEncode;
            bool* setEncode = destParam ? &g_setDestEncode : &g_setSrcEncode;
            *encode = (0 == strcmp(argv[i], "utf-8") ? UTF_8 : UTF_16);
            *setEncode = true;
        }
        else if (0 == strcmp(argv[i], "big") || 0 == strcmp(argv[i], "little"))
        {
            EndianType *endian = destParam ? &g_destEndian : &g_srcEndian;
            bool *setEndian = destParam ? &g_setDestEndian : &g_setSrcEndian;
            *endian = (0 == strcmp(argv[i], "big") ? BIG : LITTLE);
            *setEndian = true;
        }
        else if (0 == strcmp(argv[i], "bom"))
        {
            destBOM = true;
        }
        else
        {
            if (NULL == g_srcFilePath)
            {
                g_srcFilePath = malloc(strlen(argv[i]) + 1);
                if (NULL == g_srcFilePath)
                {
                    return false;
                }
                strcpy(g_srcFilePath, argv[i]);
            }
            else if (NULL == g_destFilePath)
            {
                g_destFilePath = malloc(strlen(argv[i]) + 1);
                if (NULL == g_destFilePath)
                {
                    return false;
                }
                strcpy(g_destFilePath, argv[i]);

                g_destTempFilePath = malloc(strlen(g_destFilePath) + 5);
                if (NULL == g_destTempFilePath)
                {
                    return false;
                }
                strcpy(g_destTempFilePath, g_destFilePath);
                if (0 == strcmp(FindName(g_destFilePath), FindName(g_srcFilePath)))
                {
                    strcpy(g_destTempFilePath + strlen(g_destTempFilePath), ".tmp");
                }
            }
            else
            {
                printf("%s���ǺϷ���ѡ��\n", argv[i]);
                return false;
            }
        }
    }

    if (NULL == g_srcFilePath)
    {
        printf("δָ�������ļ�\n");
        return false;
    }

    if (NULL == g_destFilePath)
    {
        printf("δָ������ļ�\n");
        return false;
    }

    if (!g_setDestEncode)
    {
        printf("δָ������ļ��ַ�����\n");
        return false;
    }

    if (g_destEncode == UTF_16 && !g_setDestEndian)
    {
        printf("δָ������ļ��ֽ���\n");
        return false;
    }

    g_srcFile = fopen(g_srcFilePath, "rb");
    if (NULL == g_srcFile)
    {
        printf("�����ļ�%s��ʧ�ܣ�ʧ��ԭ��: %s\n", g_srcFilePath, strerror(errno));
        return false;
    }

    DealSrcFileBOM();

    if (!g_setSrcEncode)
    {
        printf("δָ�������ļ��ַ�����\n");
        return false;
    }

    if (g_srcEncode == UTF_16 && !g_setSrcEndian)
    {
        printf("δָ�������ļ��ֽ���\n");
        return false;
    }

    g_destFile = fopen(g_destTempFilePath, "wb");
    if (NULL == g_destFile)
    {
        printf("����ļ�%s��ʧ�ܣ�ʧ��ԭ��: %s\n", g_destTempFilePath, strerror(errno));
        return false;
    }

    if (destBOM)
    {
        DealDestFileBOM();
    }

    return true;
}

// ���ٻ���
static void DestroyContext()
{
    free(g_srcFilePath);
    g_srcFilePath = NULL;

    if (g_srcFile != NULL)
    {
        fclose(g_srcFile);
        g_srcFile = NULL;
    }

    free(g_destFilePath);
    g_destFilePath = NULL;

    free(g_destTempFilePath);
    g_destTempFilePath = NULL;

    if (g_destFile != NULL)
    {
        fclose(g_destFile);
        g_destFile = NULL;
    }
}

// �޸�Ŀ���ļ�������ʱ�ļ����޸�Ϊ��ʽ�ļ���
static void ChangeDestFileName()
{
    if (0 == strcmp(g_destTempFilePath, g_destFilePath))
    {
        return;
    }

    char* command = malloc(strlen(g_destTempFilePath) + strlen(g_destFilePath) + 6);
    if (command == NULL)
    {
        return;
    }
#ifdef _WIN32
    sprintf(command, "move %s %s", g_destTempFilePath, g_destFilePath);
#else
    sprintf(command, "mv %s %s", g_destTempFilePath, g_destFilePath);
#endif
    system(command);
    free(command);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        PrintHelp();
        return -1;
    }

    if (!InitializeContext(argc, argv))
    {
        DestroyContext();
        return -1;
    }

    size_t srcBufSize = 1024;
    void* srcBuf = malloc(srcBufSize);
    if (srcBuf == NULL)
    {
        DestroyContext();
        return -1;
    }

    size_t destBufSize = 1024;
    void* destBuf = malloc(destBufSize);
    if (destBuf == NULL)
    {
        free(srcBuf);
        DestroyContext();
        return -1;
    }

    int ret = 0;
    while (true)
    {
        size_t srcRestSize = 0;
        if (!ReadUtfFromFile(srcBuf, srcBufSize, &srcRestSize, g_srcFile, g_srcEncode, g_srcEndian))
        {
            ret = -1;
            break;
        }
        if (srcRestSize == 0)
        {
            break;
        }

        size_t srcConvertedSize = 0;
        while (srcConvertedSize < srcRestSize)
        {
            size_t srcConvertedSizeCur = 0;
            size_t destConvertSize = 0;
            if (!UtfEncodeConvert((unsigned char*)srcBuf + srcConvertedSize, srcRestSize - srcConvertedSize,
                g_srcEncode, &srcConvertedSizeCur, destBuf, destBufSize, g_destEncode, &destConvertSize))
            {
                ret = -1;
                break;
            }

            if (!WriteUtfToFile(destBuf, destConvertSize, g_destFile, g_destEncode, g_destEndian))
            {
                ret = -1;
                break;
            }

            srcConvertedSize += srcConvertedSizeCur;
        }

        if (ret == -1)
        {
            break;
        }
    }

    if (ret != -1)
    {
        // ����������ļ�
        fclose(g_destFile);
        g_destFile = NULL;
        ChangeDestFileName();
    }

    free(srcBuf);
    free(destBuf);
    DestroyContext();
    return ret;
}
