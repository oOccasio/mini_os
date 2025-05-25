#include "../header/Header.h"

void moveFile(Directory * source, Directory * destination, char * newName){

    Directory* newFile = createNewDirectory(newName, "644"); // 소유자 읽기 , 쓰기권한, 나머지 읽기 권한 
    newFile->type = '-';
    newFile->size = source->size;
    newFile->parent = destination;
    newFile->permission[0] = source->permission[0];
    newFile->permission[1] = source->permission[1];
    newFile->permission[2] = source->permission[2];
    newFile->permission[3] = source->permission[3];
    newFile->permission[4] = source->permission[4];
    newFile->permission[5] = source->permission[5];
    newFile->permission[6] = source->permission[6];
    newFile->permission[7] = source->permission[7];
    newFile->permission[8] = source->permission[8];
    addDirectoryRoute(source, destination, newName);

    if(destination->leftChild == NULL){
        destination->leftChild = newFile;
    }else{
        Directory * sibling = destination->leftChild;
        while(sibling->rightSibling != NULL){
            sibling = sibling->rightSibling;
        }
        sibling->rightSibling = newFile;
    }

    Directory*parent = source->parent;
    if (parent->leftChild == source){
        parent->leftChild = source->rightSibling;
    }else{
        Directory * sibling = parent->leftChild;
        while(sibling->rightSibling != source){
            sibling = sibling->rightSibling;
        }
        sibling->rightSibling = source->rightSibling;
    }

    updateDirectoryFile();
}




void moveDirectory(Directory * source, Directory * destination, char * newName, bool recursive){


    Directory* newDir = createNewDirectory(newName, "755");
    newDir->parent = destination;
    newDir->permission[0] = source->permission[0];
    newDir->permission[1] = source->permission[1];
    newDir->permission[2] = source->permission[2];
    newDir->permission[3] = source->permission[3];
    newDir->permission[4] = source->permission[4];
    newDir->permission[5] = source->permission[5];
    newDir->permission[6] = source->permission[6];
    newDir->permission[7] = source->permission[7];
    newDir->permission[8] = source->permission[8];
    addDirectoryRoute(newDir, destination, newName);

    if (destination->leftChild == NULL){
        destination->leftChild = newDir;
    }else{
        Directory * sibling = destination->leftChild;
        while(sibling->rightSibling != NULL){
            sibling = sibling->rightSibling;
        }
        sibling->rightSibling = newDir;
    }


    if(recursive){
        Directory * child = source->leftChild;
        while(child != NULL){
            if(child->type == 'd'){
                moveDirectory(child, newDir, child->name, true);
            } else{
                moveFile(child, newDir, child->name);
            }
            child = child->rightSibling;
        }
    }

    Directory* parent = source->parent;
    if (parent->leftChild == source) {
        parent->leftChild = source->rightSibling;
    } else {
        Directory* sibling = parent->leftChild;
        while (sibling->rightSibling != source) {
            sibling = sibling->rightSibling;
        }
        sibling->rightSibling = source->rightSibling;
    }


}
