#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/**
 * 获取文件大小
 * @param filename 文件名称
 * @return 是否成功(1,-1)
 */
int initFileSize(char *filename, long *size) {
    struct stat buf;
    if (stat(filename, &buf) < 0) {
        printf("stat => %s,errno=%d,msg:%s\n", filename, errno, strerror(errno));
        return -1;
    }
    *size = buf.st_size;
    return 1;
}

/**
 * 获取文件名称
 * @param name 文件名称
 * @return 文件名称
 */
int initFileName(char *fileFullPath, char *fileName) {
    char *nameStart = NULL;
    int sep = '/';
    if (NULL == fileFullPath) {
        printf("the path name is NULL\n");
        return -1;
    }
    nameStart = strrchr(fileFullPath, sep);
    if (nameStart == NULL) {
        printf("can`t find / in this name [%s]", fileFullPath);
        sprintf(fileName, "%s", fileFullPath);
        return 1;
    }
    sprintf(fileName, "%s", nameStart + 1);
    return 1;
}

/**
 * 初始化文件路径
 * @param fileFullPath 文件全路径
 * @param filePath 需要初始化的文件路径
 * @return 是否成功（1,-1）
 */
int initFilePath(char *fileFullPath, char *filePath) {
    char *nameStart = NULL;
    int sep = '/';
    if (NULL == fileFullPath) {
        printf("the path name is NULL\n");
        return -1;
    }
    nameStart = strrchr(fileFullPath, sep);
    if (nameStart == NULL) {
        printf("can`t find / in this name [%s]", fileFullPath);
        sprintf(filePath, ".");
        return 1;
    }
    int pathLen = nameStart - fileFullPath;
    stpncpy(filePath, fileFullPath, pathLen);
    filePath[pathLen] = '\0';
    return 1;
}


int main() {
    printf("File Split!\n");

    // 文件名称
    char *fileFullPath = "D:/kali-linux-2019.1a-amd64.iso";

    // 文件名称
    char fileName[128];

    // 文件路径
    char filePath[1024];

    // 文件大小
    long fileSize;

    // 块大小
    long blockSize = 1024 * 1024 * 1024 * 20;

    int fileFd = open(fileFullPath, O_RDWR, S_IRUSR);
    if (fileFd == -1) {
        printf("openfile,errno=%d,msg:%s\n", errno, strerror(errno));
        exit(0);
    }

    if (initFileName(fileFullPath, fileName) < 0) {
        printf("error fileFullPath init fileName");
        exit(0);
    }

    if (initFilePath(fileFullPath, filePath) < 0) {
        printf("error fileFullPath init filePath");
        exit(0);
    }

    if (initFileSize(fileFullPath, &fileSize) < 0) {
        printf("error fileFullPath init fileSize");
        exit(0);
    }

    printf("fileFullPath => %s\n", fileFullPath);
    printf("filePath => %s\n", filePath);
    printf("fileName => %s\n", fileName);
    printf("fileLen => %ld\n", fileSize);

    char blockFileName[512];
    int blockNum = 0;

    int ioCacheLen = 4096;
    char ioCache[ioCacheLen];

    while (1) {
        long blockFileSize = fileSize > blockSize ? blockSize : fileSize;
        sprintf(blockFileName, "%s/%d_%s", filePath, blockNum, fileName);
        printf("file[%s] ==> block[name=%s,len=%ld,path=%s] \n", fileName, blockFileName, blockFileSize, filePath);
        fileSize -= blockFileSize;
        if (lseek(fileFd, fileSize, SEEK_SET) < 0) {
            printf("lseek,blockNum=%d,errno=%d,msg:%s\n", blockNum, errno, strerror(errno));
            break;
        }

        int blockFileFd = open(blockFileName, O_CREAT | O_RDWR, S_IRUSR);
        if (blockFileFd < 0) {
            printf("openfile[%s],errno=%d,msg:%s\n", blockFileName, errno, strerror(errno));
            break;
        }

        printf("%s => start\n", blockFileName);

        while (1) {
            int rLen = read(fileFd, ioCache, ioCacheLen);
            if (rLen == -1) {
                printf("read[%s],blockNum=%d,errno=%d,msg:%s\n", fileFullPath, blockNum, errno, strerror(errno));
                break;
            }
            int wLen = write(blockFileFd, ioCache, rLen);
            if (wLen == -1) {
                printf("write[%s],blockNum=%d,errno=%d,msg:%s\n", blockFileName, blockNum, errno, strerror(errno));
                break;
            }
            if (rLen != wLen) {
                printf("offset[%s,%s] rLen=%d,wLen=%d\n", fileFullPath, blockFileName, rLen, wLen);
            }
            blockFileSize -= wLen;
            if (blockFileSize == 0) {
                break;
            }
        }
        close(blockFileFd);

        printf("%s => finish\n", blockFileName);

        // 减小文件体积
        ftruncate(fileFd, fileSize);

        blockNum++;

        if (fileSize == 0) {
            break;
        }
    }

    close(fileFd);
    return 0;
}