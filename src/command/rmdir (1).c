#include "../header/header.h"

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

    // 삭제 대상 노드 및 하위 메모리 해제
    freeDirectory(dir);
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

    for (int i = 0; i < dirCount; i++) {
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

        if (pthread_create(&threads[threadCount], NULL, rmdirThread, (void*)args) == 0){
            threadCount++;
        }
        else {
            fprintf(stderr, "Failed to create thread for '%s'\n", dirPaths[i]);
            free(args->dirPath);
            free(args);
        }
    }

    for (int i = 0; i < dirCount; i++) {
        pthread_join(threads[i], NULL);
    }

    updateDirectoryFile();  // 변경된 디렉토리 정보를 파일에 반영
}

// 구조체를 기반으로 Directory.txt 파일에 파일 정보를 작성
void writeDirectoryToFile(FILE* file, Directory* directory) {
    if(directory == NULL) {
        return;
    }

    if(strcmp(directory->name, "/") == 0) {
        fprintf(file, "%s %c %d %d%d%d %d %d %d %d %d %d %d\n",
            directory->name,
            directory->type,
            directory->visible,
            (directory->permission[0] << 2) | (directory->permission[1] << 1) | directory->permission[2],
            (directory->permission[3] << 2) | (directory->permission[4] << 1) | directory->permission[5],
            (directory->permission[6] << 2) | (directory->permission[7] << 1) | directory->permission[8],
            directory->UID,
            directory->GID,
            directory->size,
            directory->month,
            directory->day,
            directory->hour,
            directory->minute
        );
    } else {
        fprintf(file, "%s %c %d %d%d%d %d %d %d %d %d %d %d %s\n",
            directory->name,
            directory->type,
            directory->visible,
            (directory->permission[0] << 2) | (directory->permission[1] << 1) | directory->permission[2],
            (directory->permission[3] << 2) | (directory->permission[4] << 1) | directory->permission[5],
            (directory->permission[6] << 2) | (directory->permission[7] << 1) | directory->permission[8],
            directory->UID,
            directory->GID,
            directory->size,
            directory->month,
            directory->day,
            directory->hour,
            directory->minute,
            directory->parent->route
        );
    }

    writeDirectoryToFile(file, directory->rightSibling);
    writeDirectoryToFile(file, directory->leftChild);
}

// rmdir 스레드 함수
void* rmdirThread(void* arg) {
    RmdirArgs* args = (RmdirArgs*)arg;

    Directory* targetDir = findRoute(args->dirPath);
    if (targetDir == NULL) {
        printf("rmdir: failed to remove '%s': No such directory\n", args->dirPath);
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