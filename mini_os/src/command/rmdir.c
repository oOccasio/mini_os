#include "../../header/Header.h"

// 실제 파일/디렉토리 삭제 함수 추가
static void deletePhysicalFileOrDir(Directory* dir) {
    if (dir == NULL) return;

    char filePath[512];
    const char* relativePath = dir->route;
    if (relativePath[0] == '/') relativePath++;

    if (strlen(relativePath) == 0) {
        snprintf(filePath, sizeof(filePath), "information/resources/file/%s", dir->name);
    } else {
        snprintf(filePath, sizeof(filePath), "information/resources/file/%s", dir->name);
    }

    int ret;
    if (dir->type == 'd') {
        // 디렉토리는 rmdir() 호출
        ret = rmdir(filePath);
    } else {
        // 파일은 remove() 호출
        ret = remove(filePath);
        printf("Deleted physical: %s\n", filePath);
    }
}

void freeDirectory(Directory* dir) {
    if (dir == NULL) return;
    free(dir);
}

void removeDirectory(Directory* dir) {
    if (dir == NULL) return;

    // 1. 실제 파일 또는 디렉토리 삭제
    deletePhysicalFileOrDir(dir);

    Directory* parent = dir->parent;
    if (parent == NULL) return;  // 루트 또는 부모 없으면 삭제 불가

    char dirName[MAX_NAME];
    strncpy(dirName, dir->name, MAX_NAME - 1);
    dirName[MAX_NAME - 1] = '\0';

    if (parent->leftChild == dir) {
        parent->leftChild = dir->rightSibling;
    } else {
        Directory* sibling = parent->leftChild;
        while (sibling != NULL && sibling->rightSibling != dir) {
            sibling = sibling->rightSibling;
        }
        if (sibling != NULL) {
            sibling->rightSibling = dir->rightSibling;
        }
    }

    dir->parent = NULL;
    dir->rightSibling = NULL;
    dir->leftChild = NULL;

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
            deletePhysicalFileOrDir(child);  // 실제 파일 삭제
            freeDirectory(child);
        }
        child = next;
    }

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