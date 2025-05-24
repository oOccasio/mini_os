#include "../header/Header.h"

int countLink(Directory* directory){
    int link = 0;
    Directory * child = directory->leftChild;

    while(child != NULL){
        link += 1;
        child = child->rightSibling;
    }

    if(directory->type == 'd'){
        link +=2;
    }
    else if(directory->type == '-'){
        link += 1;
    }

    return link;
}