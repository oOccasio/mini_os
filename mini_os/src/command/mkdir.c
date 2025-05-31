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
// splitPathToQueue 수정
static void splitPathToQueue(Queue* queue, char* path) {
    char* token = strtok(path, "/");
    while(token != NULL) {
        if(strlen(token) > 0) {
            char* copied = strdup(token);  // ✔ strdup을 통해 heap에 복사
            enqueue(queue, copied);        // 안전하게 enqueue
        }
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
    char* path = args->path;
    const char* mode = args->mode;
    bool createParents = args->createParents;

    char pathCopy[MAX_ROUTE];
    strncpy(pathCopy, path, MAX_ROUTE - 1);
    pathCopy[MAX_ROUTE - 1] = '\0';

    if (findRoute(pathCopy) != NULL) {
        printf("mkdir: cannot create directory '%s': File exists\n", path);
        free(args);
        return NULL;
    }

    Queue* queue = (Queue*)malloc(sizeof(Queue));
    initQueue(queue);
    splitPathToQueue(queue, pathCopy);

    Directory* currentDirectory = (path[0] == '/') ? dirTree->root : dirTree->current;

    while (!isEmpty(queue)) {
        char* segment = dequeue(queue);

        Directory* tmp = currentDirectory->leftChild;
        while (tmp != NULL && strcasecmp(tmp->name, segment) != 0)
            tmp = tmp->rightSibling;

        if (tmp == NULL) {
            if (!createParents && !isEmpty(queue)) {
                printf("mkdir: %s: No such file or directory.\n", path);
                free(segment);
                break;
            }
            Directory* newDir = createNewDirectory(segment, mode);
            linkNewDirectory(currentDirectory, newDir);
            addDirectoryRoute(newDir, currentDirectory, segment);
            currentDirectory = newDir;
        } else {
            if (isEmpty(queue) && !createParents) {
                printf("mkdir: cannot create directory '%s': File exists\n", path);
                free(segment);
                break;
            }
            currentDirectory = tmp;
        }

        free(segment);
    }

    freeQueue(queue);
    free(queue);
    free(args);
    updateDirectoryFile();
    return (void*)currentDirectory;
}
