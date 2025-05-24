#include "../header/Header.h"

char * findUserById(int UID){
    UserList * currentUser = userList;  //currentUserList 로 변수를 하고
    while (currentUser != NULL){
        if (currentUser->user->UID == UID){   //currentUserList -> currentUser -> UID 가 더명확할것 같음
            return currentUser->user->name;   //currentUserList -> currentUser -> name; 
        }
        currentUser = currentUser->nextUser;  //currentUserList = currentUserList -> nextUser;

    }
    return NULL;
}

//유저로드
void loadUser() {
    FILE* userFile = fopen("information/User.txt", "r");
    if (userFile == NULL) {
        fprintf(stderr, "Error: Unable to open User.txt\n");
        return;
    }

    userList = NULL;  // 초기화
    bool rootAdded = false;
    char tmp[MAX_LENGTH];

    while (fgets(tmp, MAX_LENGTH, userFile) != NULL) {
        size_t len = strlen(tmp);
        if (len > 0 && tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
        }

        User* newUser = (User*)malloc(sizeof(User));
        if (!newUser) {
            fprintf(stderr, "Memory allocation failed for User\n");
            fclose(userFile);
            return;
        }

        char* ptr = strtok(tmp, " "); // name
        if (!ptr) { free(newUser); continue; }
        strncpy(newUser->name, ptr, MAX_NAME - 1);
        newUser->name[MAX_NAME - 1] = '\0';

        ptr = strtok(NULL, " "); // UID
        if (!ptr) { free(newUser); continue; }
        newUser->UID = atoi(ptr);

        ptr = strtok(NULL, " "); // GID
        if (!ptr) { free(newUser); continue; }
        newUser->GID = atoi(ptr);

        ptr = strtok(NULL, " "); // year
        if (!ptr) { free(newUser); continue; }
        newUser->year = atoi(ptr);

        ptr = strtok(NULL, " "); // month
        if (!ptr) { free(newUser); continue; }
        newUser->month = atoi(ptr);

        ptr = strtok(NULL, " "); // day
        if (!ptr) { free(newUser); continue; }
        newUser->day = atoi(ptr);

        ptr = strtok(NULL, " "); // hour
        if (!ptr) { free(newUser); continue; }
        newUser->hour = atoi(ptr);

        ptr = strtok(NULL, " "); // minute
        if (!ptr) { free(newUser); continue; }
        newUser->minute = atoi(ptr);

        ptr = strtok(NULL, " "); // sec
        if (!ptr) { free(newUser); continue; }
        newUser->sec = atoi(ptr);

        ptr = strtok(NULL, " "); // wday
        if (!ptr) { free(newUser); continue; }
        newUser->wday = atoi(ptr);

        ptr = strtok(NULL, " "); // dir
        if (ptr) {
            strncpy(newUser->dir, ptr, MAX_NAME - 1);
            newUser->dir[MAX_NAME - 1] = '\0';
        } else {
            newUser->dir[0] = '\0';
        }

        if (strcmp(newUser->name, "root") == 0) {
            if (!rootAdded) {
                userList = (UserList*)malloc(sizeof(UserList));
                if (!userList) {
                    fprintf(stderr, "Memory allocation failed for UserList\n");
                    free(newUser);
                    fclose(userFile);
                    return;
                }
                userList->user = newUser;
                userList->nextUser = NULL;
                rootAdded = true;
            } else {
                free(newUser);
                fprintf(stderr, "Duplicate root user ignored\n");
            }
        } else {
            if (userList == NULL) {
                userList = (UserList*)malloc(sizeof(UserList));
                if (!userList) {
                    fprintf(stderr, "Memory allocation failed for UserList\n");
                    free(newUser);
                    fclose(userFile);
                    return;
                }
                userList->user = newUser;
                userList->nextUser = NULL;
            } else {
                UserList* tmpRoute = userList;
                while (tmpRoute->nextUser != NULL) {
                    tmpRoute = tmpRoute->nextUser;
                }
                UserList* nextUserList = (UserList*)malloc(sizeof(UserList));
                if (!nextUserList) {
                    fprintf(stderr, "Memory allocation failed for UserList\n");
                    free(newUser);
                    fclose(userFile);
                    return;
                }
                nextUserList->user = newUser;
                nextUserList->nextUser = NULL;
                tmpRoute->nextUser = nextUserList;
            }
        }
    }

    fclose(userFile);
}