#include "../header/header.h"

// 여러 개의 파일을 스레드에 한 개씩 넘겨줘야함
typedef struct{
    char* fileName;
    bool showLineNumber;
} CatArgs;

void* catThread(void* arg){
    CatArgs* args = (CatArgs*)arg;
    if (args == NULL || args->fileName == NULL){
        if (args) free(args);
        return NULL;
    }

    char filePath[256];
    snprintf(filePath, sizeof(filePath), "../file/%s", args->fileName);

    FILE* file = fopen(filePath, "r");
    if (file == NULL) {
        printf("cat: %s: No such file or directory\n", args->fileName);
        free(args->fileName);
        free(args);
        return NULL;
    }

    Directory* dirEntry = findRoute(args->fileName);
    if (dirEntry == NULL) {
        printf("cat: %s: Not found in directory structure\n", args->fileName);
        fclose(file);
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
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "../file/%s", fileName);

    FILE* file = fopen(filePath, "w");
    if (file == NULL){
        printf("cat: %s: Cannot create file\n", fileName);
        return;
    }

    char line[MAX_BUFFER];
    while(fgets(line, sizeof(line), stdin)){
        fprintf(file, "%s", line);
    }
    // 남은 stdin 버퍼 비우기
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF);

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);

    // 읽기쓰기 권한
    char mode[4] = "644";

    MkdirArgs* args = (MkdirArgs*)malloc(sizeof(MkdirArgs));
    if (!args) {
        printf("Memory allocation failed\n");
        return;
    }
    strncpy(args->path, fileName, MAX_ROUTE - 1);
    args->path[MAX_ROUTE - 1] = '\0';
    strncpy(args->mode, mode, 3);
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
}

// 파일 입력추가 (>> 옵션)
void appendFile(const char* fileName){
    char filePath[256];
    snprintf(filePath, sizeof(filePath), "../file/%s", fileName);

    FILE* file = fopen(filePath, "a");
    if (file == NULL){
        printf("cat: %s: Cannot open file\n", fileName);
        return;
    }

    char line[MAX_BUFFER];
    while (fgets(line, sizeof(line), stdin)){
        fprintf(file, "%s", line);
    }

    fseek(file, 0, SEEK_END);
    long int size = ftell(file);
    fclose(file);

    Directory* fileDir = findRoute(fileName);
    if (fileDir != NULL){
        fileDir->size = size;
        updateDirectoryFile();
    }
}
