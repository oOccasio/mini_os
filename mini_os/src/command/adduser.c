#include "../header/Header.h"

// 옵션 문자열이 '-'로 시작하는지 확인
bool IsOption(char* option) {
    return (option && option[0] == '-');
}

//adduser
void adduser(char* argument, DirectoryTree* dirtree, UserList** usertree) {
    time_t timer = time(NULL);
    struct tm* t = localtime(&timer);

    int optionCount = 0;
    char* options[5];
    char* token = NULL;

    int setUID = 0, setGID = 0;
    int newUID = 1, newGID = 1;
    char* newName = NULL;

    // 옵션 파싱
    if (IsOption(argument)) {
        token = strtok(argument, " ");
        while (token != NULL) {
            if (token[0] == '-') {
                options[optionCount++] = token;
            } else {
                break;
            }
            token = strtok(NULL, " ");
        }
        for (int i = 0; i < optionCount; i++) {
            if (strcmp(options[i], "-u") == 0) setUID = 1;
            else if (strcmp(options[i], "-g") == 0) setGID = 1;
            else {
                printf("Wrong option : %s\n", options[i]);
                return;
            }
        }
    } else {
        token = strtok(argument, " ");
    }

    // 옵션별 인자 파싱
    if (setUID && !setGID) {
        if (token != NULL) {
            newUID = atoi(token);
            token = strtok(NULL, " ");
            if (token != NULL) newName = token;
            else { printf("Missing arguments for -u option.\n"); return; }
        } else { printf("Missing arguments for -u option.\n"); return; }
    } else if (!setUID && setGID) {
        if (token != NULL) {
            newGID = atoi(token);
            token = strtok(NULL, " ");
            if (token != NULL) newName = token;
            else { printf("Missing arguments for -g option.\n"); return; }
        } else { printf("Missing arguments for -g option.\n"); return; }
    } else if (setUID && setGID) {
        if (token != NULL) {
            newUID = atoi(token);
            token = strtok(NULL, " ");
            if (token != NULL) {
                newGID = atoi(token);
                token = strtok(NULL, " ");
                if (token != NULL) newName = token;
                else { printf("Missing arguments for -u and -g options.\n"); return; }
            } else { printf("Missing arguments for -u and -g options.\n"); return; }
        } else { printf("Missing arguments for -u and -g options.\n"); return; }
    } else {
        // 옵션 없이 이름만 입력된 경우
        newName = token;
    }

    if (!newName) {
        printf("adduser: 사용자 이름이 필요합니다.\n");
        return;
    }

    // User 구조체 생성 및 초기화
    User* newUser = (User*)malloc(sizeof(User));
    if (!newUser) { perror("malloc failed"); return; }
    strncpy(newUser->name, newName, MAX_NAME-1);
    newUser->name[MAX_NAME-1] = '\0';
    newUser->UID = newUID;
    newUser->GID = newGID;
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
    if (!newUserList) { free(newUser); perror("malloc failed"); return; }
    newUserList->user = newUser;
    newUserList->nextUser = NULL;

    // usertree가 비어있으면 첫 노드로, 아니면 마지막에 연결
    if (*usertree == NULL) {
        *usertree = newUserList;
    } else {
        UserList* tmp = *usertree;
        while (tmp->nextUser != NULL) tmp = tmp->nextUser;
        tmp->nextUser = newUserList;
    }
    printf("adduser: 사용자 '%s' 추가 완료 (UID=%d, GID=%d)\n", newUser->name, newUser->UID, newUser->GID);

    FILE* userFile = fopen("information/User.txt", "a");
    if (userFile == NULL) {
        fprintf(stderr, "Error: Unable to open User.txt for writing\n");
        return;
    }
    fprintf(userFile, "%s %d %d %d %d %d %d %d %d %d %d %s / \n",
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
        0,                // 필요시 dir 필드가 비어있으면 0 또는 ""로 대체
        newUser->dir
    ); // <<== 반드시 \n 포함!
    fclose(userFile);


}
