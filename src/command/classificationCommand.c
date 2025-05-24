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
                    
                    for(int i = 0; i < threadCount; i++) {
                        pthread_join(threads[i], NULL);
                    }   

                    command = strtok_r(NULL, " ", &saveptr);
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
            threadCount++;

            command = strtok_r(NULL, " ", &saveptr);
        }

        for(int i = 0 ; i < threadCount; i++){
            pthread_join(threads[i], NULL);
        }

    }
    //cat 
    else if(strcmp(command, "cat") == 0) {
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

        // 리다이렉션 확인
        char* nextToken = command;
        while (nextToken != NULL) {
            if (strcmp(nextToken, ">") == 0) {
                isRedirect = true;
                nextToken = strtok_r(NULL, " ", &saveptr);
                if (nextToken != NULL) {
                    outputFile = nextToken;
                }
                break;
            } else if (strcmp(nextToken, ">>") == 0) {
                isAppend = true;
                nextToken = strtok_r(NULL, " ", &saveptr);
                if (nextToken != NULL) {
                    outputFile = nextToken;
                }
                break;
            }
            nextToken = strtok_r(NULL, " ", &saveptr);
        }

        // 파일 생성 또는 추가
        if (isRedirect && outputFile != NULL) {
            createFile(outputFile);
            return;
        } else if (isAppend && outputFile != NULL) {
            appendFile(outputFile);
            return;
        }

        char* files[MAX_BUFFER];
        int fileCount = 0;
        
        // 일반 cat 명령어 처리
        while (command != NULL && strcmp(command, ">") != 0 && strcmp(command, ">>") != 0) {
            files[fileCount++] = command;
            command = strtok_r(NULL, " ", &saveptr);
        }

        if (fileCount == 0) {
            printf("cat: missing file operand\n");
            return;
        }
        
        // 멀티스레딩 출력
        catFilesThread(files, fileCount, showLineNumber);
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
    // //grep
    // else if(strcmp(command, "grep") == 0){
    //     handleGrepCommand(saveptr);
    // }
    // //cp
    // else if(strcmp(command, "cp") == 0){
    //     bool recursive = false;
    //     char* command = strtok_r(NULL, " ", &saveptr);

    //     //cp -r
    //     if(command != NULL && strcmp(command, "-r") == 0){
    //         recursive = true;
    //         command = strtok_r(NULL, " ", &saveptr);
    //     }

    //     char* sourcePath = command;
    //     char * destinationPath = strtok_r(NULL, " ", &saveptr);

    //     if(sourcePath == NULL || destinationPath == NULL){
    //         printf("cp: missing file operand\n");
    //         return;
    //     }

    //     Directory * source = findRoute(sourcePath);
    //     Directory * destination = findRoute(destinationPath);

    //     if (source == NULL){
    //         printf("cp: cannot stat '%s': No such file or directory\n", sourcePath);
    //         return;
    //     }

    //     //복사 타입이 디렉토리인 경우
    //     if(destination != NULL && source->type == 'd'){
    //         if(recursive == false){
    //             printf("cp: %s is a directory (not copied).\n", source->name);
    //             return;
    //         }
    //         else{
    //             if(destination->type == 'd'){
    //                 copyDirectory(source, destination, recursive, source->name);
    //             }
    //             else{
    //                 printf("cp: cannot overwrite no-directory '%s' with directory '%s'\n", destinationPath, sourcePath);
    //             }
    //         }
    //     }
    //     //복사 타입이 파일인 경우
    //     else if( destination != NULL && source-> type == '-'){
    //         Directory * newFile = createNewDirectory(source->name, "644");
    //         newFile->type = '-';
    //         newFile->size = source->size;
    //         newFile->parent = destination;
    //         addDirectoryRoute(newFile, destination, source->name);

    //         if(destination->leftChild == NULL){
    //             destination->leftChild = newFile;
    //         }
    //         else{
    //             Directory * sibling = destination->leftChild;
    //             while(sibling->rightSibling != NULL){
    //                 sibling = sibling->rightSibling;
    //             }
    //             sibling->rightSibling = newFile;
    //         }

    //     }
    //     //목적지 경로가 디렉토리가 아닌 경우
    //     else if(destination == NULL){
    //         char* destinationName = strrchr(destinationPath, '/');
    //         if(destinationName != NULL){
    //             *destinationName = '\0';
    //             destination = findRoute(destinationPath);
    //             *destinationName = '/';
    //             destinationName++;
    //         }
    //         else{
    //             destination = dirTree->current;
    //             destinationName = destinationPath;
    //         }
            
    //         if(destination == NULL || destination->type != 'd'){
    //             printf("cp: cannot create '%s': No such directory\n", destinationPath);
    //             return;
    //         }
    //         if(source->type == '-'){
    //             Directory * newFile = createNewDirectory(source->name, "644");
    //             newFile->type = '-';
    //             newFile->size = source->size;
    //             newFile->parent = destination;
    //             addDirectoryRoute(newFile, destination, destinationName);

    //             if(strcmp(source->name, newFile->name) != 0){
    //                 copyFile(source, newFile);
    //             }

    //             if(destination->leftChild == NULL){
    //                 destination->leftChild = newFile;
    //             }
    //             else{
    //                 Directory * sibling = destination->leftChild;
    //                 while(sibling->rightSibling != NULL){
    //                     sibling = sibling->rightSibling;
    //                 }
    //                 sibling->rightSibling = newFile;
                    
    //             }
    //         }
    //         else if(source->type == 'd'){
    //             if(recursive == false){
    //                 printf("cp: %s is a directory (not copied).\n", source->name);
    //                 return;
    //             }
    //             else{
    //                 if(destination->type == 'd'){
    //                     copyDirectory(source, destination, recursive, destinationName);
    //                     return;
    //                 }
    //                 else{
    //                     if(destination->type == 'd'){
    //                         copyDirectory(source, destination, recursive, destinationName);
    //                     }
    //                     else{
    //                         printf("cp: cannot overwrite non-directory '%s' with directory '%s'\n", destinationPath, sourcePath);
    //                     }
    //                 }
    //             }
    //         }
    //     }

    //     updateDirectoryFile();
    
    // } 
    //pwd
    else if(strcmp(command, "pwd") == 0){
        printf("%s\n", dirTree->current->route);
    
    //mv
    else if(strcmp(command, "mv") == 0){
        bool recursive = false;
        char * command = strtok_r(NULL, " ", &saveptr);

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
            //이동 목적지가 디렉토리인 경우
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
            //이름 변경
            char * newName = destinationPath;

            if(source->type == '-'){
                char sourcePath[256];
                snprintf(sourcePath, sizeof(sourcePath), "information/resources/file/%s", source->name);
                
                char destinationPath_[256];
                snprintf(destinationPath_, sizeof(destinationPath_), "information/resources/file/%s", newName);

                strcpy(source->name, newName);
                rename(sourcePath, destinationPath_);
                updateDirectoryFile();
            }
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
    
    //zip.c
    else if (strcmp(command, "zip") == 0) {
        // zip output.zip file1 file2 ...
        char* zipName = strtok_r(NULL, " ", &saveptr);
        if (zipName == NULL) {
            printf("사용법: zip [압축파일이름] [압축할파일1] [압축할파일2] ...\n");
            printf("예시: zip archive file1.txt file2.txt\n");
            return;
        }

        // .zip 확장자 추가
        char zipFilename[MAX_ROUTE] = {0};
        snprintf(zipFilename, sizeof(zipFilename), "%s%s", zipName, 
            (strstr(zipName, ".zip") != NULL) ? "" : ".zip");

        char* fileArgs[MAX_BUFFER];
        int fileCount = 0;
        char* nextFile = strtok_r(NULL, " ", &saveptr);
        
        // 입력 파일이 없는 경우
        if (nextFile == NULL) {
            printf("사용법: zip [압축파일이름] [압축할파일1] [압축할파일2] ...\n");
            printf("예시: zip archive file1.txt file2.txt\n");
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
            printf("사용법: zip [압축파일이름] [압축할파일1] [압축할파일2] ...\n");
            return;
        }

        zip_files(zipFilename, fileArgs, fileCount);

        // 압축 파일을 가상 디렉토리에 추가
        MkdirArgs* args = (MkdirArgs*)calloc(1, sizeof(MkdirArgs));
        if (!args) {
            printf("메모리 할당 실패\n");
            return;
        }
        strncpy(args->path, zipFilename, MAX_ROUTE - 1);
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
            snprintf(filePath, sizeof(filePath), "information/resources/file/%s", zipFilename);
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
    }
}
