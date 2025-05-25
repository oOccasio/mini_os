#include "../header/Header.h"

/*
 새 디렉토리 노드 생성
 name: 디렉토리 이름
 mode: 권한 문자열
 return: 할당된 Directory 구조체 포인터
 */
Directory* createNewDirectory(char* name, const char* mode) {
    Directory* newDir = (Directory*)malloc(sizeof(Directory));
    strncpy(newDir->name, name, MAX_NAME); // 이름 복사
    newDir->type = 'd';                    // 디렉토리 타입
    newDir->visible = true;                // 숨김 여부 기본값
    setPermission(newDir, mode);           // 권한 설정
    newDir->UID = loginUser->UID;          // 소유자
    newDir->GID = loginUser->GID;          // 그룹
    newDir->size = 4096;                   // 디렉토리 기본 크기
    setDirectoryTime(newDir);              // 생성 시간 설정
    newDir->parent = NULL;                 // 상위 디렉토리(초기화)
    newDir->leftChild = NULL;              // 하위 디렉토리(초기화)
    newDir->rightSibling = NULL;           // 이웃 디렉토리(초기화)
    return newDir;
}

/*
 경로(route) 필드 채우기
 newDir: 경로를 설정할 디렉토리
 parent: 상위 디렉토리
 dirName: 현재 디렉토리 이름
 */
void addDirectoryRoute(Directory* newDir, Directory* parent, char* dirName) {
    char name[MAX_ROUTE];
    if(strcmp(parent->route, "/") == 0) { // 루트 디렉토리 바로 아래면
        strcpy(name, parent->route);
        strcat(name, dirName);
        strncpy(newDir->route, name, MAX_ROUTE);
    } else { // 그 외의 경우
        strcpy(name, parent->route);
        strcat(name, "/");
        strcat(name, dirName);
        strncpy(newDir->route, name, MAX_ROUTE);
    }
}

/*
 부모-자식 연결 함수
 parent: 부모 디렉토리
 newDir: 새로 만든 디렉토리
 */
static void linkNewDirectory(Directory* parent, Directory* newDir) {
    newDir->parent = parent;
    if(parent->leftChild == NULL) {
        parent->leftChild = newDir;
    } else {
        Directory* sib = parent->leftChild;
        while(sib->rightSibling) sib = sib->rightSibling;
        sib->rightSibling = newDir;
    }
}

/*
 경로를 '/' 기준으로 분할해서 큐에 저장 (빈 세그먼트 무시)
 path: 분할할 경로 문자열
 */
static void splitPathToQueue(Queue* queue, char* path) {
    char* token = strtok(path, "/");
    while(token != NULL) {
        if(strlen(token) > 0) enqueue(queue, token); // ❌ strdup 없이 원본 포인터 enqueue
        token = strtok(NULL, "/");
    }
}






/*
 mkdir 명령어의 스레드 함수
 arg: MkdirArgs 구조체 포인터
 return: 생성된 Directory 포인터 (실패 시 NULL)
 */
void* makeDirectory(void* arg) {
    MkdirArgs* args = (MkdirArgs*)arg;
    char* path = args->path;              // 생성할 경로
    const char* mode = args->mode;        // 권한 문자열
    bool createParents = args->createParents; // -p 옵션 여부

    // 경로 복사 (strtok로 분할 시 원본 보존)
    char pathCopy[MAX_ROUTE];
    strncpy(pathCopy, path, MAX_ROUTE-1);
    pathCopy[MAX_ROUTE-1] = '\0';

    // 이미 동일한 경로가 존재하면 에러 출력 후 종료
    if(findRoute(pathCopy) != NULL) {
    printf("mkdir: cannot create directory '%s': File exists\n", path);
    free(args);
    return NULL;
    }
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    initQueue(queue);

    Directory* currentDirectory;
    if(createParents) {
        // -p 옵션: 중간 경로가 없으면 자동 생성
        currentDirectory = (path[0] == '/') ? dirTree->root : dirTree->current;
        Queue* queue = (Queue*)malloc(sizeof(Queue));
        initQueue(queue);
        splitPathToQueue(queue, pathCopy);

        char* dirName = NULL; // 마지막 세그먼트 저장용
        while(!isEmpty(queue)) {
            char* segment = dequeue(queue);
            dirName = segment; // 마지막 세그먼트가 최종 디렉토리 이름
            Directory* tmp = currentDirectory->leftChild;
            // 현재 디렉토리의 하위에서 이름 일치하는 디렉토리 탐색
            while(tmp != NULL && strcasecmp(tmp->name, segment) != 0)
                tmp = tmp->rightSibling;
            if(tmp == NULL) {
                // 없으면 새로 생성
                Directory* newDir = createNewDirectory(segment, mode);
                linkNewDirectory(currentDirectory, newDir);
                addDirectoryRoute(newDir, currentDirectory, segment);
                currentDirectory = newDir;
            } else {
                // 이미 있으면 그 디렉토리로 이동
                currentDirectory = tmp;
            }
            free(segment);
        }
        if(dirName) free(dirName); // 마지막 세그먼트 메모리 해제
        freeQueue(queue);
        free(queue);
        updateDirectoryFile(); // 파일에 반영
        free(args);
        return (void*)currentDirectory;
    } else {
        // -p 옵션 없음: 중간 경로가 없으면 에러
        currentDirectory = (path[0] == '/') ? dirTree->root : dirTree->current;
        Queue* queue = (Queue*)malloc(sizeof(Queue));
        initQueue(queue);
        splitPathToQueue(queue, pathCopy);

        char* dirName = NULL;
        while(!isEmpty(queue)) {
            char* segment = dequeue(queue);
            dirName = segment;
            Directory* tmp = currentDirectory->leftChild;
            // 현재 디렉토리의 하위에서 이름 일치하는 디렉토리 탐색
            while(tmp != NULL && strcasecmp(tmp->name, segment) != 0)
                tmp = tmp->rightSibling;
            if(isEmpty(queue)) {
                // 마지막 세그먼트: 생성 시도
                if(tmp != NULL) {
                    printf("mkdir: cannot create directory '%s': File exists\n", path);
                    free(segment);
                    freeQueue(queue);
                    free(queue);
                    free(args);
                    return NULL;
                }
                Directory* newDir = createNewDirectory(segment, mode);
                linkNewDirectory(currentDirectory, newDir);
                addDirectoryRoute(newDir, currentDirectory, segment);
                updateDirectoryFile();
                free(segment);
                freeQueue(queue);
                free(queue);
                free(args);
                return (void*)newDir;
            }
            if(tmp == NULL) {
                // 중간 경로가 없으면 에러
                printf("mkdir: %s: No such file or directory.\n", path);
                free(segment);
                freeQueue(queue);
                free(queue);
                free(args);
                return NULL;
            }
            currentDirectory = tmp;
            free(segment);
        }
        freeQueue(queue);
        free(queue);
        free(args);
        return (void*)currentDirectory;
    }
}
