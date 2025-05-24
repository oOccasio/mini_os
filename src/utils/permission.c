#include "../header/Header.h"

void atoiPermission(Directory * directory, char* perstr){
    // 주석 처리된 코드를 사용하는 것이 더 안전함
    int permissionUser = perstr[0] - '0';
    int permissionGroup = perstr[1] - '0';    
    int permissionOther = perstr[2] - '0';

    for (int i = 0; i < 3; i++) {
    directory->permission[i]     = (permissionUser  >> (2 - i)) & 1;
    directory->permission[i + 3] = (permissionGroup >> (2 - i)) & 1;
    directory->permission[i + 6] = (permissionOther >> (2 - i)) & 1;
    }
}

void setPermission(Directory * directory, const char * mode){
    for(int i = 0; i < 3; i++){
        if (mode[i] < '0' || mode[i] > '7'){
            fprintf(stderr, "chmod: invalid mode: %s\n", mode);
            return;
        }
    }

    int permissionUser = mode[0] - '0';
    int permissionGroup = mode[1] - '0';
    int permissionOther = mode[2] - '0';
    
    for (int i = 0; i < 3; i++) {
    directory->permission[i]     = (permissionUser  >> (2 - i)) & 1;
    directory->permission[i + 3] = (permissionGroup >> (2 - i)) & 1;
    directory->permission[i + 6] = (permissionOther >> (2 - i)) & 1;
    }
}