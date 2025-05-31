#include "../header/Header.h"

// adduser 명령어 구현
User* adduser(char* argument, int UID, int GID, DirectoryTree* dirtree, UserList** usertree) {
    time_t timer = time(NULL);
    struct tm* t = localtime(&timer);

    User* newUser = (User*)malloc(sizeof(User));
    if (!newUser) { perror("malloc failed"); return NULL; }

    strncpy(newUser->name, argument, MAX_NAME - 1);
    newUser->name[MAX_NAME-1] = '\0';
    newUser->UID = UID;
    newUser->GID = GID;
    newUser->year = t->tm_year + 1900;
    newUser->month = t->tm_mon + 1;
    newUser->day = t->tm_mday;
    newUser->hour = t->tm_hour;
    newUser->minute = t->tm_min;
    newUser->sec = t->tm_sec;
    newUser->wday = t->tm_wday;
    newUser->dir[0] = '\0'; // 홈 디렉토리 경로 등 필요시 추가

    // UserList에 추가
    UserList* newUserList = (UserList*)malloc(sizeof(UserList));
    if (!newUserList) { free(newUser); perror("malloc failed"); return NULL; }
    newUserList->user = newUser;
    newUserList->nextUser = NULL;

    if (*usertree == NULL) {
        *usertree = newUserList;
    } else {
        UserList* tmp = *usertree;
        while (tmp->nextUser != NULL) tmp = tmp->nextUser;
        tmp->nextUser = newUserList;
    }
    printf("adduser: user '%s' added successfully (UID=%d, GID=%d)\n", newUser->name, newUser->UID, newUser->GID);

    FILE* userFile = fopen("information/User.txt", "a");
    if (userFile == NULL) {
        fprintf(stderr, "Error: Unable to open User.txt for writing\n");
        return NULL;
    }
    fprintf(userFile, "%s %d %d %d %d %d %d %d %d %d %s / \n",
        newUser->name,
        newUser->UID,
        newUser->GID,
        newUser->year,
        newUser->month,
        newUser->day,
        newUser->hour,
        newUser->minute,
        newUser->sec,
        newUser->wday,
        newUser->dir
    );
    fclose(userFile);
}
