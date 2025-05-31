#include "../../header/Header.h"


// Directory.txt 파일에 파일의 정보를 작성하기기
void writeDirectoryToFile(FILE * file, Directory * directory){
    if(directory == NULL){
        return; 
    }

    if(strcmp(directory->name, "/") == 0){
        fprintf(file, "%s %c %d %d%d%d %d %d %d %d %d %d %d\n",
            directory->name,
            directory->type,
            directory->visible,
            (directory->permission[0] << 2) | (directory->permission[1] << 1) | directory->permission[2],
            (directory->permission[3] << 2) | (directory->permission[4] << 1) | directory->permission[5],
            (directory->permission[6] << 2) | (directory->permission[7] << 1) | directory->permission[8],
            directory->UID,
            directory->GID,
            directory->size,
            directory->month,
            directory->day,
            directory->hour,
            directory->minute
        );
    }
    else{
        fprintf(file, "%s %c %d %d%d%d %d %d %d %d %d %d %d %s\n",
            directory->name,
            directory->type,
            directory->visible,
            (directory->permission[0] << 2) | (directory->permission[1] << 1) | directory->permission[2],
            (directory->permission[3] << 2) | (directory->permission[4] << 1) | directory->permission[5],
            (directory->permission[6] << 2) | (directory->permission[7] << 1) | directory->permission[8],
            directory->UID,
            directory->GID,
            directory->size,
            directory->month,
            directory->day,
            directory->hour,
            directory->minute,
            directory->parent->route
        );
    }

    writeDirectoryToFile(file, directory->rightSibling);
    writeDirectoryToFile(file, directory->leftChild);
}

//
void updateDirectoryFile(){
    FILE* file = fopen("information/Directory.txt", "w");
    if (file == NULL) {
        perror("Failed to open Directory.txt fo waiting");
        return;
    }

    writeDirectoryToFile(file, dirTree->root);  //root 부터 하위폴더까지 업데이트 시키기
    fclose(file);
}

// 저장된 시간 변경
void setDirectoryTime(Directory * directory){
    time_t now;
    struct tm* currentTime;

    //현재 시간
    time(&now);
    currentTime = localtime(&now);

    //디렉토리의 시간값 수정
    directory->month = currentTime->tm_mon + 1;
    directory->day = currentTime->tm_mday;
    directory->hour = currentTime->tm_hour;
    directory->minute = currentTime->tm_min;
}

void getMonth(int month){
    switch(month){
    case 1:
        printf("Jan ");
        break;
    case 2:
        printf("Feb ");
        break;
    case 3:
        printf("Mar ");
        break;
    case 4:
        printf("Apr ");
        break;
    case 5:
        printf("May ");
        break;
    case 6:
        printf("Jun ");
        break;
    case 7:
        printf("Jul ");
        break;
    case 8:
        printf("Aug ");
        break;
    case 9:
        printf("Sep ");
        break;
    case 10:
        printf("Oct ");
        break;
    case 11:
        printf("Nov ");
        break;
    case 12:
        printf("Dec ");
        break;
    default:
        break;
    }
}

void buildDirectoryRoute(Queue *queue, Directory * parent, Directory* myAddress){
    if (parent == NULL || myAddress == NULL) {
        printf("잘못된 디렉토리 포인터\n");
        return;
    }

    if(isEmpty(queue)){
        myAddress->parent = parent;
        myAddress->leftChild = NULL;
        myAddress->rightSibling = NULL;  // 미리 NULL로 설정

        if(parent->leftChild == NULL){
            parent->leftChild = myAddress;
        }
        else{
            Directory* tmp = parent->leftChild;
            // 이미 같은 이름의 디렉토리가 있는지 확인
            while(tmp != NULL) {
                if(strcmp(tmp->name, myAddress->name) == 0) {
                    printf("이미 존재하는 디렉토리 이름입니다: %s\n", myAddress->name);
                    return;
                }
                if(tmp->rightSibling == NULL) break;
                tmp = tmp->rightSibling;
            }
            tmp->rightSibling = myAddress;
        }
    }
    else{
        char *str = dequeue(queue);
        if (str == NULL) {
            printf("큐에서 문자열을 가져오는데 실패했습니다\n");
            return;
        }

        Directory *tmp = parent->leftChild;
        while(tmp != NULL && strcmp(tmp->name, str) != 0){
            tmp = tmp->rightSibling;
        }

        if(tmp == NULL){
            printf("디렉토리를 찾을 수 없습니다: %s\n", str);
            free(str);
            return;
        }
        
        free(str);
        buildDirectoryRoute(queue, tmp, myAddress);
    }
}


// DirectoryNode 만들기
void buildDirectoryNode(char * dirstr){
    if (dirstr == NULL || dirstr[0] == '\0') {
        printf("Debug: Empty input string\n");
        return;
    }

    printf("Debug: Input string: '%s'\n", dirstr);

    Directory * directory = (Directory*)calloc(1, sizeof(Directory));
    if (!directory) {
        perror("Failed to allocate Directory");
        return;
    }

    char name[MAX_NAME] = {0};
    char routestr[MAX_ROUTE] = {0};
    char *tmp_str = strdup(dirstr);
    if (!tmp_str) {
        printf("Debug: Failed to duplicate string\n");
        free(directory);
        return;
    }

    char *tmp = strtok(tmp_str, " "); // name
    if (tmp == NULL) {
        printf("Debug: Failed to get name\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Processing directory: '%s'\n", tmp);
    strncpy(directory->name, tmp, MAX_NAME - 1);
    directory->name[MAX_NAME - 1] = '\0';

    tmp = strtok(NULL, " ");  // type
    if (tmp == NULL) {
        printf("Debug: Failed to get type\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Type: '%c'\n", tmp[0]);
    directory->type = tmp[0];
    
    tmp = strtok(NULL, " ");  // visible
    if (tmp == NULL) {
        printf("Debug: Failed to get visible\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Visible: '%s'\n", tmp);
    directory->visible = atoi(tmp);
    
    tmp = strtok(NULL, " "); //permission
    if (tmp == NULL) {
        printf("Debug: Failed to get permission\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Permission string: '%s'\n", tmp);
    atoiPermission(directory, tmp);
    
    tmp = strtok(NULL, " "); //UID
    if (tmp == NULL) {
        printf("Debug: Failed to get UID\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: UID: '%s'\n", tmp);
    directory->UID = atoi(tmp);
    
    tmp = strtok(NULL, " "); //GID
    if (tmp == NULL) {
        printf("Debug: Failed to get GID\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: GID: '%s'\n", tmp);
    directory->GID = atoi(tmp);
    
    tmp = strtok(NULL, " "); //size
    if (tmp == NULL) {
        printf("Debug: Failed to get size\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Size: '%s'\n", tmp);
    directory->size = atoi(tmp);
    
    tmp = strtok(NULL, " "); //month
    if (tmp == NULL) {
        printf("Debug: Failed to get month\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Month: '%s'\n", tmp);
    directory->month = atoi(tmp);
    
    tmp = strtok(NULL, " "); //day
    if (tmp == NULL) {
        printf("Debug: Failed to get day\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Day: '%s'\n", tmp);
    directory->day = atoi(tmp);
    
    tmp = strtok(NULL, " "); //hour
    if (tmp == NULL) {
        printf("Debug: Failed to get hour\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Hour: '%s'\n", tmp);
    directory->hour = atoi(tmp);

    tmp = strtok(NULL, " "); //minute
    if (tmp == NULL) {
        printf("Debug: Failed to get minute\n");
        free(tmp_str);
        free(directory);
        return;
    }
    printf("Debug: Minute: '%s'\n", tmp);
    directory->minute = atoi(tmp);
    
    tmp = strtok(NULL, " "); //route
    if (tmp != NULL) {
        if (strcmp(directory->name, "root") == 0 && strcmp(tmp, "/") == 0) {
            printf("Debug: Setting up root directory with route '/'\n");
            strncpy(directory->route, "/", MAX_ROUTE - 1);
            directory->route[MAX_ROUTE - 1] = '\0';
            directory->parent = directory;
            directory->leftChild = NULL;
            directory->rightSibling = NULL;
            dirTree->root = directory;
            dirTree->current = directory;
            printf("Debug: Root directory initialized successfully\n");
        } else {
            printf("Debug: Processing non-root directory with route: '%s'\n", tmp);
            
            // 경로 문자열 처리를 안전하게 수행
            char temp_route[MAX_ROUTE] = {0};
            if (strcmp(tmp, "/") == 0) {
                snprintf(temp_route, MAX_ROUTE - 1, "/%s", directory->name);
            } else {
                snprintf(temp_route, MAX_ROUTE - 1, "%s/%s", tmp, directory->name);
            }
            strncpy(directory->route, temp_route, MAX_ROUTE - 1);
            directory->route[MAX_ROUTE - 1] = '\0';

            if (dirTree->root == NULL) {
                printf("Debug: Error - trying to add non-root directory before root is initialized\n");
                free(tmp_str);
                free(directory);
                return;
            }

            Queue *queue = (Queue*)calloc(1, sizeof(Queue));
            if (!queue) {
                printf("Debug: Failed to allocate Queue\n");
                free(tmp_str);
                free(directory);
                return;
            }
            initQueue(queue);

            char *routeToken = strtok(tmp, "/");
            buildQueue(queue, routeToken);
            buildDirectoryRoute(queue, dirTree->root, directory);
            freeQueue(queue);
            free(queue);
        }
    } else {
        printf("Debug: Error - no route specified\n");
        free(tmp_str);
        free(directory);
        return;
    }
    
    free(tmp_str);
    printf("Debug: Successfully built directory: '%s' with route '%s'\n", directory->name, directory->route);
}

Directory* loadDirectory() {
    dirTree = (DirectoryTree*)malloc(sizeof(DirectoryTree));
    if (dirTree == NULL){
        perror("Failed to allocate DirectoryTree");
        return NULL;
    }

    dirTree->root = NULL;
    dirTree->current = NULL;

    char tmp[MAX_LENGTH];

    FILE * directory = fopen("information/Directory.txt", "r");
    if (directory == NULL) {
        perror("Failed to open Directory.txt for reading");
        free(dirTree);
        return NULL;
    }

    while(fgets(tmp, MAX_LENGTH, directory) != NULL){
        if (tmp[strlen(tmp) - 1] == '\n') {
            tmp[strlen(tmp) - 1] = '\0';
        }
        if (tmp[0] == '\0') continue;  // 빈 줄 건너뛰기

        buildDirectoryNode(tmp);
        if (dirTree->root == NULL) {
            fprintf(stderr, "Failed to build directory node\n");
            continue;
        }
    }

    fclose(directory);

    if (dirTree->root == NULL){
        fprintf(stderr, "Directory tree root not initialized\n");
        free(dirTree);
        return NULL;
    }

    return dirTree;
}

//find Recursive
Directory* findRouteRecursive(Queue *queue, Directory *parent){
    if(isEmpty(queue)){
        return parent;
    }
    else{
        char * str = dequeue(queue);
        Directory * tmp = parent->leftChild;
        if(tmp == NULL){
            printf("에러 발생\n");
            free(str);
            return NULL;
        }
        while(strcasecmp(tmp->name, str) != 0){
            tmp = tmp -> rightSibling;
            if(tmp == NULL){
                printf("에러 발생\n");
                free(str);
                return NULL;
            }
        }
    
        free(str);

        return findRouteRecursive(queue,tmp);
    }

}

Directory* findRoute(char * pathOrigin){
    if (pathOrigin == NULL || strcmp(pathOrigin, "") == 0){
        return dirTree->current;
    }

    char *path = strdup(pathOrigin);
    if (path == NULL){
        perror("메모리 할당 실패1");
        return NULL;
    }

    Directory* currentDirectory;
    char *pathStart = path;  // 원래 포인터 저장

    if(path[0] == '/'){
        currentDirectory = dirTree->root;
        path++; 
    }
    else{
        currentDirectory = dirTree->current;
    }

    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL){
        perror("메모리 할당 실패2");
        free(pathStart);  // 원래 포인터 해제
        return NULL;
    }
    initQueue(queue);

    char* token = strtok(path, "/");
    while(token != NULL){
        char *copy = strdup(token);
        if (copy == NULL){
            perror("메모리 할당 실패3");
            free(pathStart);  // 원래 포인터 해제
            freeQueue(queue);
            free(queue);
            return NULL;
        }
        enqueue(queue, copy);
        token = strtok(NULL, "/");
    }

    free(pathStart);  // 원래 포인터 해제
    
    while(!isEmpty(queue)){
        char *segment = dequeue(queue);
        if(segment == NULL) {
            freeQueue(queue);
            free(queue);
            return NULL;
        }

        if(strcmp(segment, "..") == 0){
            if(currentDirectory->parent != NULL){
                currentDirectory = currentDirectory->parent;
            }
        }
        else if(strcmp(segment, ".") == 0){
            free(segment);
            continue;
        }
        else{
            Directory* tmp = currentDirectory->leftChild;
            while(tmp != NULL && strcasecmp(tmp->name, segment) != 0){
                tmp = tmp->rightSibling;
            }
            if(tmp == NULL){
                free(segment);
                freeQueue(queue);
                free(queue);
                return NULL;
            }
            currentDirectory = tmp;
        }
        free(segment);
    }
    
    freeQueue(queue);
    free(queue);
    return currentDirectory;
}

void updateRouteRecursive(Directory* dir) {
    if (!dir) return;
    // 루트가 아니면 부모 route + '/' + name
    if (dir->parent && dir != dir->parent) {
        snprintf(dir->route, MAX_ROUTE, "%s/%s", dir->parent->route, dir->name);
    }
    // 자식도 재귀적으로 갱신
    Directory* child = dir->leftChild;
    while (child) {
        updateRouteRecursive(child);
        child = child->rightSibling;
    }
}




