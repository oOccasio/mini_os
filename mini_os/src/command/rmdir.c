#include "../../header/Header.h"

// 디렉토리 및 하위 노드 메모리 재귀 해제
void freeDirectory(Directory* dir) {
    if (dir == NULL) return;
    freeDirectory(dir->leftChild);
    freeDirectory(dir->rightSibling);
    free(dir);
}

// 부모의 자식 연결 리스트에서 디렉토리 제거 후 메모리 해제
void removeDirectory(Directory* dir) {
    if (dir == NULL) return;
    Directory* parent = dir->parent;
    if (parent == NULL) return;  // 루트이거나 부모 없으면 제거 불가

    // 삭제 전 디렉토리 정보 백업
    char dirName[MAX_NAME];
    strncpy(dirName, dir->name, MAX_NAME - 1);
    dirName[MAX_NAME - 1] = '\0';

    // 부모의 leftChild가 삭제 대상이면 바로 연결 변경
    if (parent->leftChild == dir) {
        parent->leftChild = dir->rightSibling;
    } else {
        // 부모의 자식 리스트 순회하며 대상 노드 찾기
        Directory* sibling = parent->leftChild;
        while (sibling != NULL && sibling->rightSibling != dir) {
            sibling = sibling->rightSibling;
        }
        if (sibling != NULL) {
            sibling->rightSibling = dir->rightSibling;
        }
    }

    // 삭제 대상 노드의 연결 해제
    dir->parent = NULL;
    dir->rightSibling = NULL;
    dir->leftChild = NULL;

    // 삭제 대상 노드 메모리 해제
    free(dir);

    printf("rmdir: successfully removed directory '%s'\n", dirName);
}

void removeDirectoryRecursive(Directory* dir) {
    if (dir == NULL) return;

    Directory* child = dir->leftChild;
    while (child != NULL) {
        Directory* next = child->rightSibling;
        if (child->type == 'd') {
            removeDirectoryRecursive(child);
        } else {
            // 파일 노드면 바로 메모리 해제
            freeDirectory(child);
        }
        child = next;
    }
    // 자기 자신도 삭제
    removeDirectory(dir);
}


// 다중 디렉토리 삭제 스레드 실행
void removeDirectoryThread(char* dirPaths[], int dirCount, bool recursive) {
    if (dirCount > MAX_THREAD) {
        printf("rmdir: Too many directories\n");
        return;
    }

    pthread_t threads[MAX_THREAD];
    int threadCount = 0;
    bool success = false;

    for (int i = 0; i < dirCount; i++) {
        if (dirPaths[i] == NULL || strlen(dirPaths[i]) == 0) {
            fprintf(stderr, "rmdir: Invalid directory path\n");
            continue;
        }

        RmdirArgs* args = malloc(sizeof(RmdirArgs));
        if (!args) {
            fprintf(stderr, "Memory allocation failed\n");
            continue;
        }

        args->dirPath = strdup(dirPaths[i]);
        if (!args->dirPath) {
            fprintf(stderr, "Memory allocation failed\n");
            free(args);
            continue;
        }
        args->recursive = recursive;

        if (pthread_create(&threads[threadCount], NULL, rmdirThread, (void*)args) == 0) {
            threadCount++;
            success = true;
        } else {
            fprintf(stderr, "Failed to create thread for '%s'\n", dirPaths[i]);
            free(args->dirPath);
            free(args);
        }
    }

    // 모든 스레드 완료 대기
    for (int i = 0; i < threadCount; i++) {
        pthread_join(threads[i], NULL);
    }

    // 하나라도 성공적으로 삭제되었다면 디렉토리 파일 업데이트
    if (success) {
        updateDirectoryFile();
    }
}


// rmdir 스레드 함수
void* rmdirThread(void* arg) {
    if (arg == NULL) {
        printf("rmdir: Invalid arguments\n");
        return NULL;
    }

    RmdirArgs* args = (RmdirArgs*)arg;
    if (args->dirPath == NULL) {
        printf("rmdir: Invalid path\n");
        free(args);
        return NULL;
    }

    Directory* targetDir = findRoute(args->dirPath);
    if (targetDir == NULL) {
        printf("rmdir: failed to remove '%s': No such directory\n", args->dirPath);
        free(args->dirPath);
        free(args);
        return NULL;
    }

    if (targetDir->type != 'd') {
        printf("rmdir: failed to remove '%s': Not a directory\n", args->dirPath);
        free(args->dirPath);
        free(args);
        return NULL;
    }

    if (targetDir == dirTree->root) {
        printf("rmdir: failed to remove '%s': Cannot remove root directory\n", args->dirPath);
        free(args->dirPath);
        free(args);
        return NULL;
    }

    if (!args->recursive) {
        if (targetDir->leftChild != NULL) {
            printf("rmdir: failed to remove '%s': Directory not empty\n", args->dirPath);
            free(args->dirPath);
            free(args);
            return NULL;
        }
        removeDirectory(targetDir);
    } else {
        removeDirectoryRecursive(targetDir);
    }

    free(args->dirPath);
    free(args);
    return NULL;
}