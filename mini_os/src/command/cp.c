#include "../header/Header.h"

// 파일 복사: 파일 데이터만 복사
void copyFile(Directory* source, Directory* destination) {
    char srcPath[256], destPath[256];
    
    // route 기반 경로 사용
    snprintf(srcPath, sizeof(srcPath), "information/resources/file%s", source->route);
    snprintf(destPath, sizeof(destPath), "information/resources/file%s", destination->route);

    // 상위 디렉토리가 없으면 생성
    char tempPath[256];
    strncpy(tempPath, destPath, sizeof(tempPath));
    char* lastSlash = strrchr(tempPath, '/');
    if (lastSlash) {
        *lastSlash = '\0';
        char mkdirCmd[300];
        snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p %s", tempPath);
        system(mkdirCmd);
    }

    FILE *src = fopen(srcPath, "rb");
    if (!src) {
        perror("copyFile: source open failed");
        return;
    }

    FILE *dest = fopen(destPath, "wb");
    if (!dest) {
        perror("copyFile: destination open failed");
        fclose(src);
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src)) > 0)
        fwrite(buffer, 1, bytes, dest);

    fclose(src);
    fclose(dest);
}


// 디렉토리 복사: 구조(포인터) 완전성 보장
void copyDirectory(Directory* source, Directory* destParent, bool recursive, char* folderName) {
    if (!recursive) {
        printf("cp: -r 옵션 없이 디렉토리는 복사할 수 없습니다.\n");
        return;
    }

    // 새 디렉토리 생성
    Directory* newDir = createNewDirectory(folderName, "755");
    newDir->type = 'd';
    newDir->parent = destParent;
    newDir->leftChild = NULL;
    newDir->rightSibling = NULL;
    addDirectoryRoute(newDir, destParent, folderName);

    // ✅ 반드시 부모 트리에 연결
    if (destParent->leftChild == NULL) {
        destParent->leftChild = newDir;
    } else {
        Directory* sibling = destParent->leftChild;
        while (sibling->rightSibling)
            sibling = sibling->rightSibling;
        sibling->rightSibling = newDir;
    }

    // 자식 노드 순회 및 재귀 복사
    Directory* prevChild = NULL;
    Directory* child = source->leftChild;
    while (child) {
        if (child->type == 'd') {
            Directory* copied = createNewDirectory(child->name, "755");
            copied->type = 'd';
            copied->parent = newDir;
            copied->leftChild = NULL;
            copied->rightSibling = NULL;
            addDirectoryRoute(copied, newDir, child->name);

            if (!newDir->leftChild)
                newDir->leftChild = copied;
            else
                prevChild->rightSibling = copied;
            prevChild = copied;

            // 재귀 호출
            copyDirectory(child, copied, true, child->name);
        } else if (child->type == '-') {
            Directory* copied = createNewDirectory(child->name, "644");
            copied->type = '-';
            copied->size = child->size;
            copied->parent = newDir;
            copied->leftChild = NULL;
            copied->rightSibling = NULL;
            addDirectoryRoute(copied, newDir, child->name);

            if (!newDir->leftChild)
                newDir->leftChild = copied;
            else
                prevChild->rightSibling = copied;
            prevChild = copied;

            copyFile(child, copied);
        }
        child = child->rightSibling;
    }
}
