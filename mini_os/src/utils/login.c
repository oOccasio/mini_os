#include "../header/Header.h"

User * login(){
    User * user;
    UserList * users = userList;
    char name[MAX_NAME];
    bool isExist = false;

    printf("Users: ");

    while(users != NULL){
        printf("%s ", users->user->name);
        users = users->nextUser;
    }
    printf("\n");

    while(isExist != true){
        printf("Login: ");
        fgets(name, sizeof(name), stdin);
        name[strlen(name) - 1]= '\0';

        users = userList;
        while(users != NULL){
            if(strcmp(name, users->user->name) == 0){
                user = users->user;
                isExist = true;
                break;
            }
            users = users->nextUser;
        }
        if(isExist == false){
            printf("'%s' User does not exists\n", name);
        }
    }

    return user;
}