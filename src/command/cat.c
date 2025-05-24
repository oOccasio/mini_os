#include "../../header/Header.h"

// 여러 개의 파일을 스레드에 한 개씩 넘겨줘야함

// 파일 접근 권한 검사
static bool checkFilePermission(Directory* file, bool needWrite) {
    if (file == NULL) return false;
    
    // 소유자인 경우
    if (loginUser->UID == file->UID) {
        if (needWrite) return file->permission[1]; // 쓰기 권한
        return file->permission[0]; // 읽기 권한
    }
    // 그룹 멤버인 경우
    else if (loginUser->GID == file->GID) {
        if (needWrite) return file->permission[4]; // 쓰기 권한
        return file->permission[3]; // 읽기 권한
    }
    // 기타 사용자인 경우
    else {
        if (needWrite) return file->permission[7]; // 쓰기 권한
        return file->permission[6]; // 읽기 권한
    }
}

void* catThread(void* arg){
    CatArgs* args = (CatArgs*)arg;
    if (args == NULL || args->fileName == NULL){
        if (args) free(args);
        return NULL;
    }

    Directory* dirEntry = findRoute(args->fileName);
    if (dirEntry == NULL) {
        printf("cat: %s: Not found in directory structure\n", args->fileName);
        free(args->fileName);
        free(args);
        return NULL;
    }

    // 읽기 권한 검사
    if (!checkFilePermission(dirEntry, false)) {
        printf("cat: %s: Permission denied\n", args->fileName);
        free(args->fileName);
        free(args);
        return NULL;
    }

    char filePath[256];
    snprintf(filePath, sizeof(filePath), "information/resources/file/%s", args->fileName);

    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        printf("cat: %s: No such file or directory\n", args->fileName);
        free(args->fileName);
        free(args);
        return NULL;
    }

    char line[MAX_BUFFER];
    int lineNumber = 1;
    while (fgets(line, sizeof(line), file) != NULL){
        if (args->showLineNumber){
            printf("%6d\t%s", lineNumber++, line);
        } else {
            printf("%s", line);
        }
    }
    fclose(file);
    free(args->fileName);
    free(args);
    return NULL;
}

// 멀티스레딩으로 파일 출력(-n 출력/기본출력 옵션)
void catFilesThread(char* fileNames[], int fileCount, bool showLineNumber){
    if (fileCount > MAX_THREAD) {
        printf("cat: Too many files. Max supported: %d\n", MAX_THREAD);
        return;
    }

    pthread_t threads[MAX_THREAD];

    for (int i = 0; i < fileCount; i++){
        CatArgs* args = (CatArgs*)malloc(sizeof(CatArgs));
        args->fileName = strdup(fileNames[i]);
        args->showLineNumber = showLineNumber;

        pthread_create(&threads[i], NULL, catThread, (void*)args);
    }

    for (int i = 0; i < fileCount; i++){
        pthread_join(threads[i], NULL);
    }
}

//파일 생성 (> 옵션)
void createFile(const char* fileName){
    Directory* existingFile = findRoute(fileName);
    if (existingFile != NULL) {
        // 파일이 존재하는 경우 쓰기 권한 검사
        if (!checkFilePermission(existingFile, true)) {
            printf("cat: %s: Permission denied\n", fileName);
            return;
        }
    }

    char filePath[256];
    snprintf(filePath, sizeof(filePath), "information/resources/file/%s", fileName);

    FILE* file = fopen(filePath, "w");
    if (file == NULL){
        printf("cat: %s: Cannot create file\n", fileName);
        return;
    }

    char line[MAX_BUFFER];
    while(fgets(line, sizeof(line), stdin) != NULL){
        if(line[0] == '\x04') break;  // Ctrl+D 입력 확인
        fprintf(file, "%s", line);
    }

    // EOF나 에러 발생 시 stdin 초기화
    clearerr(stdin);

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);

    if (existingFile == NULL) {
        // 새 파일 생성
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

        pthread_t thread;
        void* threadResult;

        pthread_create(&thread, NULL, makeDirectory, (void*)args);
        pthread_join(thread, &threadResult);

        Directory* newDir = (Directory*)threadResult;
        if (newDir != NULL) {
            newDir->type = '-';
            newDir->size = size;
            updateDirectoryFile();
        }
    } else {
        // 기존 파일 크기 업데이트
        existingFile->size = size;
        updateDirectoryFile();
    }
}

// 파일 입력추가 (>> 옵션)
void appendFile(const char* fileName){
    Directory* existingFile = findRoute(fileName);
    if (existingFile != NULL) {
        // 파일이 존재하는 경우 쓰기 권한 검사
        if (!checkFilePermission(existingFile, true)) {
            printf("cat: %s: Permission denied\n", fileName);
            return;
        }
    }

    char filePath[256];
    snprintf(filePath, sizeof(filePath), "information/resources/file/%s", fileName);

    FILE* file = fopen(filePath, "a");
    if (file == NULL){
        printf("cat: %s: Cannot open file\n", fileName);
        return;
    }

    char line[MAX_BUFFER];
    while(fgets(line, sizeof(line), stdin) != NULL){
        if(line[0] == '\x04') break;  // Ctrl+D 입력 확인
        fprintf(file, "%s", line);
    }

    // EOF나 에러 발생 시 stdin 초기화
    clearerr(stdin);

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);

    if (existingFile != NULL) {
        existingFile->size = size;
        updateDirectoryFile();
    }
}
