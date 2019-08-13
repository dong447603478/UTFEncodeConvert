#include <stdbool.h>
#include "UTFEncodeConvert.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 输入相关参数
EncodeType g_srcEncode;                 // 输入编码类型
bool g_setSrcEncode = false;            // 已设置输入编码类型
EndianType g_srcEndian;                 // 输入字节序
bool g_setSrcEndian = false;            // 已设置输入字节序
char *g_srcFilePath = NULL;             // 输入文件路径
FILE *g_srcFile = NULL;                 // 输入文件

// 输出相关参数
EncodeType g_destEncode;                // 输出编码类型
bool g_setDestEncode = false;           // 已设置输出编码类型
EndianType g_destEndian;                // 输出字节序
bool g_setDestEndian = false;           // 已设置输出字节序
char *g_destFilePath = NULL;            // 输出文件路径
char *g_destTempFilePath = NULL;        // 输出临时文件路径
FILE* g_destFile = NULL;                // 输入文件

// 打印说明
static void PrintHelp()
{
    printf("命令格式: UTFEncodeConvert <选项> 输入文件 输出文件\n"
        "选项说明:\n"
        "-f         输入标识，代表后面出现的参数为输入文件的参数\n"
        "-t         输出标识，代表后面出现的参数为输出文件的参数\n"
        "utf-8      字符编码为utf-8\n"
        "utf-16     字符编码为utf-16\n"
        "big        大端字节序\n"
        "little     小端字节序\n"
        "bom        输出文件写入BOM，该参数仅对输出文件有效，若不填写则输出文件不带BOM\n"
        "若前面没有出现输入输出标识，则默认当前参数为输出参数\n"
        "若输入文件有BOM，则输入文件字符编码和字节序以实际输入文件为准\n");
}

// 从文件路径中查找文件名
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

// 处理输入文件的BOM
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

// 处理输出文件的BOM
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

// 初始化环境
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
                printf("%s不是合法的选项\n", argv[i]);
                return false;
            }
        }
    }

    if (NULL == g_srcFilePath)
    {
        printf("未指定输入文件\n");
        return false;
    }

    if (NULL == g_destFilePath)
    {
        printf("未指定输出文件\n");
        return false;
    }

    if (!g_setDestEncode)
    {
        printf("未指定输出文件字符编码\n");
        return false;
    }

    if (g_destEncode == UTF_16 && !g_setDestEndian)
    {
        printf("未指定输出文件字节序\n");
        return false;
    }

    g_srcFile = fopen(g_srcFilePath, "rb");
    if (NULL == g_srcFile)
    {
        printf("输入文件%s打开失败，失败原因: %s\n", g_srcFilePath, strerror(errno));
        return false;
    }

    DealSrcFileBOM();

    if (!g_setSrcEncode)
    {
        printf("未指定输入文件字符编码\n");
        return false;
    }

    if (g_srcEncode == UTF_16 && !g_setSrcEndian)
    {
        printf("未指定输入文件字节序\n");
        return false;
    }

    g_destFile = fopen(g_destTempFilePath, "wb");
    if (NULL == g_destFile)
    {
        printf("输出文件%s打开失败，失败原因: %s\n", g_destTempFilePath, strerror(errno));
        return false;
    }

    if (destBOM)
    {
        DealDestFileBOM();
    }

    return true;
}

// 销毁环境
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

// 修改目标文件名从临时文件名修改为正式文件名
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
        // 重命名输出文件
        fclose(g_destFile);
        g_destFile = NULL;
        ChangeDestFileName();
    }

    free(srcBuf);
    free(destBuf);
    DestroyContext();
    return ret;
}
