#include "../header/Header.h"



void classificationCommand(char * cmd){
    
    //명령어 없음
    if(strcmp(cmd, "") == 0 || cmd[0] == ' '){
        return;
    }

    Directory * targetDirectory;
    char * saveptr;
    char * command = strtok_r(cmd, " ", &saveptr);
 
    //ls
    if(strcmp(command, "ls") == 0){
        command = strtok_r(NULL, " ", &saveptr);
        bool showAll = false;
        bool showDetails = false;

        if(command != NULL && command[0] == '-'){
            //ls -a
            if(strcmp(command, "-a") == 0){
                showAll = true;
            } //ls -l
            else if(strcmp(command, "-l") == 0){
                showDetails = true;
            } //ls -al or ls -la
            else if(strcmp(command, "-al") == 0 || strcmp(command, "-la") == 0){
                showAll = true;
                showDetails = true;
            }else{
                char * error = strtok_r(command, "-", &saveptr);
                printf("ls: invalid option -- '%s'\n", error);
                return;
            }
            command = strtok_r(NULL, " ", &saveptr);
        }
    
        
        pthread_t threads[MAX_THREAD];
        int threadCount = 0;

        //command가 없을 시 현재 디렉토리 내용 출력
        if(command == NULL){
            targetDirectory = dirTree->current;
            listDirectory(targetDirectory, showAll, showDetails); //타겟 디렉토리 내용 나열
            }else{
                //command가 NULL이 아니면 여러 인자 반복해서 처리
                while(command != NULL){
                    targetDirectory = findRoute(command);

                    if(targetDirectory == NULL){
                        //해당 경로 없을 시 오류 출력
                        printf("ls: No such file or directory: %s\n", command);
                    }else{
                        ListArgs* data = (ListArgs*)malloc(sizeof(ListArgs));
                        data->directory = targetDirectory;
                        data->showAll = showAll;
                        data->showDetails = showDetails;
                    
                        pthread_create(&threads[threadCount], NULL, listDirectoryThread, (void*)data);
                        threadCount++;
                    
                    }

                    command = strtok_r(NULL, " ", &saveptr);
                }

                // 모든 스레드 완료 대기
                for (int i = 0; i < threadCount; i++) {
                    pthread_join(threads[i], NULL);
                }
        }
        
    } 
    //cd  
    else if(strcmp(command, "cd") == 0){
        command = strtok_r(NULL, " ", &saveptr);
        changeDirectory(command);  
    } 
    //mkdir
    else if(strcmp(command, "mkdir") == 0){
        char mode[4] = "755";
        bool createParents = false;

        command = strtok_r(NULL, " ", &saveptr);
        while(command != NULL && command[0] == '-'){
            //mkdir -m (권한 설정)
            if(strcmp(command, "-m") == 0){
                command = strtok_r(NULL, " ", &saveptr);
                strncpy(mode, command, 4);
            }
            //mkdir -p (상위 디렉토리 자동 생성)
            else if(strcmp(command, "-p") == 0){
                createParents = true;
            }
            command = strtok_r(NULL, " ", &saveptr);
        }

        pthread_t threads[MAX_THREAD];
        int threadCount = 0;


        while(command != NULL){
            MkdirArgs* args = (MkdirArgs*)malloc(sizeof(MkdirArgs));
            strncpy(args->path, command, MAX_ROUTE);
            strncpy(args->mode, mode, 4);
            args->createParents = createParents;

            pthread_create(&threads[threadCount], NULL, makeDirectory, (void*)args); //mkdir.c
            pthread_join(threads[threadCount], NULL);
            threadCount++;

            command = strtok_r(NULL, " ", &saveptr);
        }

    }
    //cat 
    else if (strcmp(command, "cat") == 0) {
        command = strtok_r(NULL, " ", &saveptr);
        if (command == NULL) {
            printf("cat: missing operand\n");
            return;
        }

        bool showLineNumber = false;
        bool isAppend = false;
        bool isRedirect = false;
        char* outputFile = NULL;

        if (strcmp(command, "-n") == 0) {
            showLineNumber = true;
            command = strtok_r(NULL, " ", &saveptr);
        }

        char* files[MAX_BUFFER];
        int fileCount = 0;

        // 파일 목록 수집
        while (command != NULL && strcmp(command, ">") != 0 && strcmp(command, ">>") != 0) {
            files[fileCount++] = command;
            command = strtok_r(NULL, " ", &saveptr);
        }

        // 리다이렉션 확인
        // 리다이렉션 확인
        if (command != NULL) {
            if (strcmp(command, ">") == 0) {
                isRedirect = true;
                outputFile = strtok_r(NULL, " ", &saveptr);
                if (outputFile == NULL) {
                    printf("cat: missing file operand after '>'\n");
                    return;
                }
            } else if (strcmp(command, ">>") == 0) {
                isAppend = true;
                outputFile = strtok_r(NULL, " ", &saveptr);
                if (outputFile == NULL) {
                    printf("cat: missing file operand after '>>'\n");
                    return;
                }
            }
        }


        // 리다이렉션 관련 처리
        if (isRedirect && outputFile != NULL) {
            createFile(outputFile);
            return;
        } else if (isAppend && outputFile != NULL) {
            appendFile(outputFile);
            return;
        }

        // 단일 스레드로 파일 출력 호출
        catFiles(files, fileCount, showLineNumber);
    }

    //chmod
    else if(strcmp(command, "chmod") == 0){
        command = strtok_r(NULL, " ", &saveptr);
        
        if(command == NULL){
            printf("chmod: missing operand\n");
            return;
        }
        char mode[4];
        strncpy(mode, command, 4);
        command = strtok_r(NULL, " ", &saveptr);
        if (command == NULL){
            printf("chmod: missing operand after '%s'\n", mode);
            return;
        }

        pthread_t threads[MAX_THREAD];
        int threadCount = 0;

        while(command != NULL){
            ChmodArgs * args = (ChmodArgs*)malloc(sizeof(ChmodArgs));
            strncpy(args->path, command, MAX_ROUTE);
            strncpy(args->mode, mode, 4);

            pthread_create(&threads[threadCount], NULL, changeMode,(void*)args);
            threadCount++;

            command = strtok_r(NULL, " ", &saveptr);
        }

        for (int i = 0; i < threadCount; i++){
            pthread_join(threads[i], NULL);
        }

        updateDirectoryFile();
    }

    //cp
    else if (strcmp(command, "cp") == 0) {
    bool recursive = false;
    char* command = strtok_r(NULL, " ", &saveptr);

    if (command != NULL && strcmp(command, "-r") == 0) {
        recursive = true;
        command = strtok_r(NULL, " ", &saveptr);
    }

    char* sourcePath = command;
    char* destinationPath = strtok_r(NULL, " ", &saveptr);

    if (sourcePath == NULL || destinationPath == NULL) {
        printf("cp: missing file operand\n");
        return;
    }

    Directory* source = findRoute(sourcePath);
    Directory* destination = findRoute(destinationPath);

    if (source == NULL) {
        printf("cp: cannot stat '%s': No such file or directory\n", sourcePath);
        return;
    }

    // 목적지가 존재하고 디렉토리면: 내부에 복사
    if (destination != NULL && destination->type == 'd') {
        if (source->type == 'd') {
            if (!recursive) {
                printf("cp: -r option required to copy a directory\n");
                return;
            }

            // 새 이름은 source->name
            copyDirectory(source, destination, recursive, source->name);
        } else if (source->type == '-') {
            Directory* newFile = createNewDirectory(source->name, "644");
            newFile->type = '-';
            newFile->size = source->size;
            newFile->parent = destination;
            addDirectoryRoute(newFile, destination, source->name);
            copyFile(source, newFile);

            if (destination->leftChild == NULL) {
                destination->leftChild = newFile;
            } else {
                Directory* sibling = destination->leftChild;
                while (sibling->rightSibling != NULL)
                    sibling = sibling->rightSibling;
                sibling->rightSibling = newFile;
            }
        }
    }

    // 목적지가 존재하지 않음: 이름 변경 복사
    else {
        char* destinationName = strrchr(destinationPath, '/');
        if (destinationName != NULL) {
            *destinationName = '\0';
            destination = findRoute(destinationPath);
            *destinationName = '/';
            destinationName++;
        } else {
            destination = dirTree->current;
            destinationName = destinationPath;
        }

        if (destination == NULL || destination->type != 'd') {
            printf("cp: cannot create '%s': No such directory\n", destinationPath);
            return;
        }

        if (source->type == '-') {
            Directory* newFile = createNewDirectory(destinationName, "644");
            newFile->type = '-';
            newFile->size = source->size;
            newFile->parent = destination;
            addDirectoryRoute(newFile, destination, destinationName);
            copyFile(source, newFile);

            if (destination->leftChild == NULL) {
                destination->leftChild = newFile;
            } else {
                Directory* sibling = destination->leftChild;
                while (sibling->rightSibling != NULL)
                    sibling = sibling->rightSibling;
                sibling->rightSibling = newFile;
            }
        } else if (source->type == 'd') {
            if (!recursive) {
                printf("cp: -r option required to copy a directory\n");
                return;
            }
            copyDirectory(source, destination, recursive, destinationName);
        }
    }

    updateDirectoryFile();
}

    //pwd.c
    else if (strcmp(command, "pwd") == 0){
        Queue dirQueue;
        initQueue(&dirQueue);
        command = strtok_r(NULL, " ", &saveptr);

        if (command != NULL && strcmp(command,"-L") == 0){
            pwd(dirTree, &dirQueue, "-L");
        } else if (command != NULL && strcmp(command,"-P") == 0){
            pwd(dirTree, &dirQueue, "-P");
        } else if(command != NULL && strcmp(command, "--help") == 0){
            pwd(dirTree, &dirQueue, "--help");
        } else{
            pwd(dirTree, &dirQueue, NULL);
        }
        freeQueue(&dirQueue);
    }

    //clear.c
    else if (strcmp(command, "clear")==0){
        clear();
        return;
    }


    //mv
    //mv 디렉토리 이름도 변경 가능하도록 확장
    else if(strcmp(command, "mv") == 0){
        bool recursive = false;
        command = strtok_r(NULL, " ", &saveptr);

        //mv -r
        if (command != NULL && strcmp(command,"-r") == 0){
            recursive = true;
            command = strtok_r(NULL, " ", &saveptr);
        }

        char* sourcePath = command;
        char* destinationPath = strtok_r(NULL, " ", &saveptr);

        if(sourcePath == NULL || destinationPath == NULL){
            printf("mv: missing file operand\n");
            return;
        }

        Directory* source = findRoute(sourcePath);
        Directory* destination = findRoute(destinationPath);

        if(source == NULL){
            printf("mv: cannot stat '%s': No such file or directory\n", sourcePath);
            return;
        }

        if(destination != NULL && destination->type == 'd'){
            if(source->type == 'd' && recursive){
                moveDirectory(source, destination, source->name, recursive);
            }
            else if(source->type == '-'){
                moveFile(source, destination, source->name);
            }
            else{
                printf("mv: omitting directory '%s'\n", source->name);
            }
        }
        else{
            char * newName = destinationPath;

            if(source->type == '-'){
                char sourcePathBuf[256];
                snprintf(sourcePathBuf, sizeof(sourcePathBuf), "information/resources/file/%s", source->name);
                char destinationPathBuf[256];
                snprintf(destinationPathBuf, sizeof(destinationPathBuf), "information/resources/file/%s", newName);
                
                strcpy(source->name, newName);
                rename(sourcePathBuf, destinationPathBuf);
                updateRouteRecursive(source); // 이름 바꾼 후 route 재귀 갱신
            } else if (source->type == 'd') {
                // 디렉토리 이름 변경 추가
                strcpy(source->name, newName);
                updateRouteRecursive(source); // 이름 바꾼 후 route 재귀 갱신
            }
            updateDirectoryFile();
        }
    }
    //rmdir.c
    else if (strcmp(command, "rmdir") == 0){
        bool recursive = false;
        command = strtok_r(NULL, " ", &saveptr);

        // 옵션 -r
        if (command != NULL && strcmp(command, "-r") == 0){
            recursive = true;
            command = strtok_r(NULL, " ", &saveptr);
        }

        if (command == NULL){
            printf("rmdir: missing operand\n");
            return;
        }

        // 삭제할 디렉토리 경로들 저장
        char* dirPaths[MAX_BUFFER];
        int dirCount = 0;

        while (command != NULL){
            dirPaths[dirCount] = strdup(command);  // 문자열 복사
            if (dirPaths[dirCount] == NULL) {
                // 메모리 할당 실패 시 이전에 할당된 메모리 해제
                for (int i = 0; i < dirCount; i++) {
                    free(dirPaths[i]);
                }
                printf("rmdir: memory allocation failed\n");
                return;
            }
            dirCount++;
            command = strtok_r(NULL, " ", &saveptr);
        }

        // 멀티스레딩으로 디렉토리 삭제 실행
        removeDirectoryThread(dirPaths, dirCount, recursive);

        // 할당된 메모리 해제
        for (int i = 0; i < dirCount; i++) {
            free(dirPaths[i]);
        }
    }
    
    //adduser.c
    else if (strcmp(command, "adduser") == 0){
        int UID = 1000;
        int GID = 1000;
        char username[MAX_NAME] = {0};
        command = strtok_r(NULL, " ", &saveptr);

        while (command != NULL){
            if (strcmp(command, "-u") == 0) {
                command = strtok_r(NULL, " ", &saveptr);
                if (command != NULL) {
                    UID = atoi(command);
                }
            } else if (strcmp(command, "-g") == 0) {
                command = strtok_r(NULL, " ", &saveptr);
                if (command != NULL) {
                    GID = atoi(command);
                }
            } else {
                strncpy(username, command, MAX_NAME - 1);
                username[MAX_NAME - 1] = '\0';
            }
            command = strtok_r(NULL, " ", &saveptr);
        }

        if(strlen(username) == 0){
            printf("adduser: have to enter user name.\n");
            return;
        }

        adduser(username, UID, GID, dirTree, &userList);
    }
    
    //zip.c
    else if (strcmp(command, "zip") == 0) {
        // zip output.zip file1 file2 ...
        char* zipName = strtok_r(NULL, " ", &saveptr);
        if (zipName == NULL) {
            printf("사용법: zip [압축파일이름].zip [압축할파일1] [압축할파일2] ...\n");
            return;
        }

        char* fileArgs[MAX_BUFFER];
        int fileCount = 0;
        char* nextFile = strtok_r(NULL, " ", &saveptr);
        
        // 입력 파일이 없는 경우
        if (nextFile == NULL) {
            printf("사용법: zip [압축파일이름].zip [압축할파일1] [압축할파일2] ...\n");
            printf("예시: zip archive.zip file1.txt file2.txt\n");
            return;
        }

        while (nextFile != NULL && fileCount < MAX_BUFFER) {
            // 파일 존재 여부 확인
            Directory* fileDir = findRoute(nextFile);
            if (fileDir == NULL || fileDir->type != '-') {
                printf("zip: '%s': 파일을 찾을 수 없습니다.\n", nextFile);
                nextFile = strtok_r(NULL, " ", &saveptr);
                continue;
            }
            fileArgs[fileCount++] = nextFile;
            nextFile = strtok_r(NULL, " ", &saveptr);
        }

        if (fileCount == 0) {
            printf("zip: 압축할 수 있는 파일이 없습니다.\n");
            printf("사용법: zip [압축파일이름].zip [압축할파일1] [압축할파일2] ...\n");
            return;
        }

        zip_files(zipName, fileArgs, fileCount);

        // 압축 파일을 가상 디렉토리에 추가
        MkdirArgs* args = (MkdirArgs*)calloc(1, sizeof(MkdirArgs));
        if (!args) {
            printf("메모리 할당 실패\n");
            return;
        }
        strncpy(args->path, zipName, MAX_ROUTE - 1);
        args->path[MAX_ROUTE - 1] = '\0';
        strncpy(args->mode, "644", 3);
        args->mode[3] = '\0';
        args->createParents = false;

        pthread_t thread;
        void* threadResult;
        pthread_create(&thread, NULL, makeDirectory, (void*)args);
        pthread_join(thread, &threadResult);

        Directory* newFile = (Directory*)threadResult;
        if (newFile != NULL) {
            newFile->type = '-';
            // 압축 파일 크기 업데이트
            char filePath[256] = {0};
            snprintf(filePath, sizeof(filePath), "information/resources/file/%s", zipName);
            FILE* fp = fopen(filePath, "rb");
            if (fp != NULL) {
                fseek(fp, 0, SEEK_END);
                newFile->size = ftell(fp);
                fclose(fp);
            }
            updateDirectoryFile();
        }
    }
    else if (strcmp(command, "unzip") == 0) {
        // unzip input.zip
        char* zipName = strtok_r(NULL, " ", &saveptr);
        if (zipName == NULL) {
            printf("unzip: 압축 해제할 파일명을 입력하세요.\n");
            return;
        }

        // 압축 파일 존재 여부 확인
        Directory* zipFile = findRoute(zipName);
        if (zipFile == NULL || zipFile->type != '-') {
            printf("unzip: '%s': 파일을 찾을 수 없습니다.\n", zipName);
            return;
        }

        unzip_files(zipName);
        // 압축 해제된 파일들은 zip_files 함수 내에서 가상 디렉토리에 추가됨
    }else {
    printf("잘못된 명령어입니다: %s\n", command);
}
}
