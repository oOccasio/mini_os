#include "../header/Header.h"

User* parseUserLine(char* line);
void addUserToList(UserList** userList, User* newUser);



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



//유저 로드
void loadUser() {
    FILE* userFile = fopen("../information/User.txt", "r");
    if (userFile == NULL) {
        fprintf(stderr, "Error: Unable to open User.txt\n");
        return;
    }

    userList = NULL;  // 초기화

    char tmp[MAX_LENGTH];
    bool rootAdded = false;

    while (fgets(tmp, MAX_LENGTH, userFile) != NULL) {
        size_t len = strlen(tmp);
        if (len > 0 && tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
        }
        
        
        User* newUser = parseUserLine(tmp);
        if (!newUser) {
            fprintf(stderr, "Failed to parse user line\n");
            continue;
        }

        if (strcmp(newUser->name, "root") == 0) {
            if (!rootAdded) {
                addUserToList(&userList, newUser);
                rootAdded = true;
            } else {
                free(newUser);
                fprintf(stderr, "Duplicate root user ignored\n");
            }
        } else {
            addUserToList(&userList, newUser);
        }
    }


    fclose(userFile);
}


//유저 parsing하는 코드드
User* parseUserLine(char* line) {
    line[strcspn(line, "\r\n")] = '\0';

    User* newUser = (User*)malloc(sizeof(User));
    if (!newUser) {
        fprintf(stderr, "Memory allocation failed for User\n");
        return NULL;
    }

    char* ptr = strtok(line, " "); // name
    if (!ptr) return NULL;
    strncpy(newUser->name, ptr, MAX_NAME);

    ptr = strtok(NULL, " "); // UID
    newUser->UID = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // GID
    newUser->GID = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // year
    newUser->year = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // month
    newUser->month = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // day
    newUser->day = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // hour
    newUser->hour = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // minute
    newUser->minute = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // sec
    newUser->sec = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // wday
    newUser->wday = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " "); // dir
    if (ptr)
        strncpy(newUser->dir, ptr, MAX_NAME);
    else
        newUser->dir[0] = '\0';

    return newUser;
}


//User추가 코드
void addUserToList(UserList** userList, User* newUser) {
    if (!newUser) return;

    if (*userList == NULL) {
        *userList = (UserList*)malloc(sizeof(UserList));
        if (!(*userList)) {
            fprintf(stderr, "Memory allocation failed for UserList\n");
            return;
        }
        (*userList)->user = newUser;
        (*userList)->nextUser = NULL;
    } else {
        UserList* tmpRoute = *userList;
        while (tmpRoute->nextUser != NULL) {
            tmpRoute = tmpRoute->nextUser;
        }
        UserList* nextUserList = (UserList*)malloc(sizeof(UserList));
        if (!nextUserList) {
            fprintf(stderr, "Memory allocation failed for UserList\n");
            return;
        }
        nextUserList->user = newUser;
        nextUserList->nextUser = NULL;
        tmpRoute->nextUser = nextUserList;
    }
}
