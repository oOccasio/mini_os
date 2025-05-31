#include "../../header/Header.h"

// 파일 접근 권한 검사 (읽기/쓰기)
static bool checkFilePermission(Directory* file, bool needWrite) {
    if (file == NULL) return false;

    if (loginUser->UID == file->UID) {
        return needWrite ? file->permission[1] : file->permission[0];
    } else if (loginUser->GID == file->GID) {
        return needWrite ? file->permission[4] : file->permission[3];
    } else {
        return needWrite ? file->permission[7] : file->permission[6];
    }
}

// 여러 파일을 순차적으로 출력 (단일 스레드)
// showLineNumber가 true면 각 라인 앞에 번호를 붙임
void catFiles(char* fileNames[], int fileCount, bool showLineNumber) {
    for (int i = 0; i < fileCount; i++) {
        Directory* dirEntry = findRoute(fileNames[i]);
        if (dirEntry == NULL) {
            printf("cat: %s: No such file or directory\n", fileNames[i]);
            continue;
        }

        if (!checkFilePermission(dirEntry, false)) {
            printf("cat: %s: Permission denied\n", fileNames[i]);
            continue;
        }

        char filePath[256];
        const char *relativePath = dirEntry->route;
        if (relativePath[0] == '/') {
            relativePath++; // 맨 앞 '/' 제거
        }

        if (strlen(relativePath) == 0) {
            snprintf(filePath, sizeof(filePath), "information/resources/file/%s", dirEntry->name);
        } else {
            snprintf(filePath, sizeof(filePath), "information/resources/file/%s", dirEntry->name);
        }
        FILE* file = fopen(filePath, "r");
        if (file == NULL) {
            printf("cat: %s: Cannot open file\n", fileNames[i]);
            continue;
        }

        char line[MAX_BUFFER];
        int lineNumber = 1;
        while (fgets(line, sizeof(line), file) != NULL) {
            if (showLineNumber) {
                printf("%6d\t%s", lineNumber++, line);
            } else {
                printf("%s", line);
            }
        }
        fclose(file);
    }
}

// 파일 생성 함수 (> 리다이렉션용)
void createFile(const char* fileName) {
    Directory* existingFile = findRoute(fileName);
    char filePath[256];

    if (existingFile == NULL) {
        snprintf(filePath, sizeof(filePath), "information/resources/file/%s", fileName);
    } else {
        snprintf(filePath, sizeof(filePath), "information/resources/file/%s", existingFile->name);
    }

    FILE* file = fopen(filePath, "w");
    if (file == NULL) {
        printf("cat: %s: Cannot create file\n", fileName);
        return;
    }

    char line[MAX_BUFFER];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (line[0] == '\x04') break;  // Ctrl+D 입력 시 종료
        fprintf(file, "%s", line);
    }

    clearerr(stdin);
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);

    if (existingFile == NULL) {
        MkdirArgs* args = (MkdirArgs*)malloc(sizeof(MkdirArgs));
        if (!args) {
            printf("Memory allocation failed\n");
            return;
        }
        strncpy(args->path, fileName, MAX_ROUTE - 1);
        args->path[MAX_ROUTE - 1] = '\0';
        strncpy(args->mode, "644", 3);
        args->mode[3] = '\0';
        args->createParents = false;

        Directory* newDir = (Directory*)makeDirectory((void*)args);
        if (newDir != NULL) {
            newDir->type = '-';
            newDir->size = size;
            updateDirectoryFile();
        }
    } else {
        existingFile->size = size;
        updateDirectoryFile();
    }
}

// 파일 추가 입력 함수 (>> 리다이렉션용)
void appendFile(const char* fileName) {
    Directory* existingFile = findRoute(fileName);
    if (existingFile != NULL && !checkFilePermission(existingFile, true)) {
        printf("cat: %s: Permission denied\n", fileName);
        return;
    }

    char filePath[256];
    const char *relativePath = existingFile->route;
    if (relativePath[0] == '/') {
        relativePath++; // 맨 앞 '/' 제거
    }

    if (strlen(relativePath) == 0) {
        snprintf(filePath, sizeof(filePath), "information/resources/file/%s", existingFile->name);
    } else {
        snprintf(filePath, sizeof(filePath), "information/resources/file/%s", existingFile->name);
    }

    FILE* file = fopen(filePath, "a");
    if (file == NULL) {
        printf("cat: %s: Cannot open file\n", fileName);
        return;
    }

    char line[MAX_BUFFER];
    while (fgets(line, sizeof(line), stdin) != NULL) {
        if (line[0] == '\x04') break;
        fprintf(file, "%s", line);
    }

    clearerr(stdin);
    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);

    if (existingFile != NULL) {
        existingFile->size = size;
        updateDirectoryFile();
    }
}
