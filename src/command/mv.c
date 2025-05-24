#include "../header/Header.h"

// 권한 복사 함수
void copyPermissions(Directory* dest, Directory* src) {
    for (int i = 0; i < 9; i++) {
        dest->permission[i] = src->permission[i];
    }
}

// 자식 노드 추가 함수
void addChildToDirectory(Directory* parent, Directory* child) {
    if (parent->leftChild == NULL) {
        parent->leftChild = child;
    } else {
        Directory* sibling = parent->leftChild;
        
        while (sibling->rightSibling != NULL) {
            sibling = sibling->rightSibling;
        }        
        sibling->rightSibling = child;
    }
}


// 부모 노드에서 자식 제거 함수
void removeChildFromParent(Directory* parent, Directory* target) {
    if (!parent || !target || !parent->leftChild) return;

    if (parent->leftChild == target) {
        parent->leftChild = target->rightSibling;
    } else {
        Directory* sibling = parent->leftChild;
        while (sibling->rightSibling && sibling->rightSibling != target) {
            sibling = sibling->rightSibling;
        }
        if (sibling->rightSibling == target) {
            sibling->rightSibling = target->rightSibling;
        }
    }
}



// 파일 이동 함수
void moveFile(Directory* source, Directory* destination, char* newName) {
    Directory* newFile = createNewDirectory(newName, "644");
    newFile->type = '-';
    newFile->size = source->size;
    newFile->parent = destination;

    copyPermissions(newFile, source);
    addDirectoryRoute(source, destination, newName);
    addChildToDirectory(destination, newFile);
    removeChildFromParent(source->parent, source);

    updateDirectoryFile();
}

// 디렉토리 이동 함수
void moveDirectory(Directory* source, Directory* destination, char* newName, bool recursive) {
    Directory* newDir = createNewDirectory(newName, "755"); // 기본 권한 755
    newDir->parent = destination;

    copyPermissions(newDir, source);
    addDirectoryRoute(newDir, destination, newName);
    addChildToDirectory(destination, newDir);

    if (recursive) {
        Directory* child = source->leftChild;
        while (child != NULL) {
            if (child->type == 'd') {
                moveDirectory(child, newDir, child->name, true);
            } else {
                moveFile(child, newDir, child->name);
            }
            child = child->rightSibling;
        }
    }

    removeChildFromParent(source->parent, source);
    updateDirectoryFile();
}
