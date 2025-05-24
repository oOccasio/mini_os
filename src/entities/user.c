#include "../header/Header.h"

User* parseUserLine(char* line);
void addUserToList(UserList** userList, User* newUser);

// UID로 사용자 이름 찾기
char* findUserById(int UID) {
    UserList* currentUser = userList;
    while (currentUser != NULL) {
        if (currentUser->user->UID == UID) {
            return currentUser->user->name;
        }
        currentUser = currentUser->nextUser;
    }
    return NULL;
}

// 유저 로드
void loadUser() {
    FILE* userFile = fopen("../information/User.txt", "r");
    if (userFile == NULL) {
        fprintf(stderr, "Error: Unable to open User.txt\n");
        return;
    }

    userList = NULL;  // 리스트 초기화
    char tmp[MAX_LENGTH];
    bool rootAdded = false;

    while (fgets(tmp, MAX_LENGTH, userFile) != NULL) {
        // 줄 끝 개행 문자 제거 (\n, \r 포함)
        tmp[strcspn(tmp, "\r\n")] = '\0';

        User* newUser = parseUserLine(tmp);
        if (!newUser) {
            fprintf(stderr, "Failed to parse user line\n");
            continue;
        }

        // name에 남은 쓰레기 제거
        newUser->name[strcspn(newUser->name, "\r\n ")] = '\0';

        // 디버그용 출력
        printf("== Loaded User: '%s'\n", newUser->name);

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

// 한 줄 파싱 → User 구조체
User* parseUserLine(char* line) {
    User* newUser = (User*)malloc(sizeof(User));
    if (!newUser) return NULL;

    char* ptr = strtok(line, " ");
    if (!ptr) return NULL;
    strncpy(newUser->name, ptr, MAX_NAME - 1);
    newUser->name[MAX_NAME - 1] = '\0';

    ptr = strtok(NULL, " ");
    newUser->UID = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->GID = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->year = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->month = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->day = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->hour = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->minute = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->sec = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    newUser->wday = ptr ? atoi(ptr) : 0;

    ptr = strtok(NULL, " ");
    if (ptr)
        strncpy(newUser->dir, ptr, MAX_NAME - 1);
    else
        newUser->dir[0] = '\0';
    newUser->dir[MAX_NAME - 1] = '\0';

    return newUser;
}

// 연결 리스트에 유저 추가
void addUserToList(UserList** userList, User* newUser) {
    if (!newUser) return;

    UserList* newNode = (UserList*)malloc(sizeof(UserList));
    if (!newNode) {
        fprintf(stderr, "Memory allocation failed for UserList\n");
        free(newUser);
        return;
    }

    newNode->user = newUser;
    newNode->nextUser = NULL;

    if (*userList == NULL) {
        *userList = newNode;
    } else {
        UserList* cur = *userList;
        while (cur->nextUser != NULL) {
            cur = cur->nextUser;
        }
        cur->nextUser = newNode;
    }
}
