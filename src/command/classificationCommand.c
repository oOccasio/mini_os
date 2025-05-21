#include "../my_os/header/Header.h"


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
                        pthread_join(threads[threadCount], NULL);
                        threadCount++;
                    
                    }

                    command = strtok_r(NULL, " ", &saveptr);
                }
        }
        
    } 
    //cd  
    else if(strcmp(command, "cd") == 0){
        command = strtok_r(NULL, " ", &saveptr);
        changeDriectory(command);  
    } 
    //mkdir
    else if(strcmp(command, "mkdir")){
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
    else if(strcmp(command, "cat") == 0){
        command = strtok_r(NULL, " ", &saveptr);
        
        if(command == NULL){
            printf("cat: mission operand\n");
            return;
        }

        if (strcmp(command, ">") == 0){
            //cat > [파일명]
            command = strtok_r(NULL, " ", &saveptr);
            if(command == NULL){
                printf("cat: missing file operand after '>'\n");
                return;
            }
            createFile(command);  
        }
        else if(strcmp(command, ">>") == 0){
            //cat >> [파일명]
            command = strtok_r(NULL, " ", &saveptr);
            if(command == NULL){
                printf("cat: missing file operand after '>>'\n");
                return;
            }
            appendToFile(command); //cat.c 파일 추가
        }
        else{
            bool showLineNumbers = false;
            //cat -n
            if (strcmp(command, "-n") == 0){
                showLineNumbers = true;
                command = strtok_r(NULL, " ", &saveptr);
            }

            char* files[MAX_BUUFFER];
            int fileCount = 0;
            while(command != NULL && strcmp(command, ">>") != 0){
                files[fileCount++] = command;
                command = strtok_r(NULL, " ", &saveptr);
            }
            
            if(command == NULL){
                //cat [파일명] or cat -n [파일명]
                if(showLineNumbers){
                    catFilesWithLineNumbers(files, fileCount); 
                }
                else{
                    catFiles(files, fileCount);  
                }
            }
            else{
                //cat [파일명] [파일명] >> [파일명]
                command = strtok_r(NULL, " ", &saveptr);
                if(command == NULL){
                    printf("cat: missing file operand after '>>'\n");
                    return;
                }
                concatenateFilesToNewFile(files, fileCount, command);
            }
        }
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
    //grep
    else if(strcmp(command, "grep") == 0){
        handleGrepCommand(saveptr);
    }
    //cp
    else if(strcmp(command, "cp") == 0){
        bool recursive = false;
        char* command = strtok_r(NULL, " ", &saveptr);

        //cp -r
        if(command != NULL && strcmp(command, "-r") == 0){
            recursive = true;
            command = strtok_r(NULL, " ", &saveptr);
        }

        char* sourcePath = command;
        char * destinationPath = strtok_r(NULL, " ", &saveptr);

        if(sourcePath == NULL || destinationPath == NULL){
            printf("cp: missing file operand\n");
            return;
        }

        Directory * source = findRoute(sourcePath);
        Directory * destination = findRoute(destinationPath);

        if (source == NULL){
            printf("cp: cannot stat '%s': No such file or directory\n", sourcePath);
            return;
        }

        //복사 타입이 디렉토리인 경우
        if(destination != NULL && source->type == 'd'){
            if(recursive == false){
                printf("cp: %s is a directory (not copied).\n", source->name);
                return;
            }
            else{
                if(destination->type == 'd'){
                    copyDirectory(source, destination, recursive, source->name);
                }
                else{
                    printf("cp: cannot overwrite no-directory '%s' with directory '%s'\n", destinationPath, sourcePath);
                }
            }
        }
        //복사 타입이 파일인 경우
        else if( destination != NULL && source-> type == '-'){
            Directory * newFile = createNewDirectory(source->name, "644");
            newFile->type = '-';
            newFile->size = source->size;
            newFile->parent = destination;
            addDirectoryRoute(newFile, destination, source->name);

            if(destination->leftChild == NULL){
                destination->leftChild = newFile;
            }
            else{
                Directory * sibling = destination->leftChild;
                while(sibling->rightSibling != NULL){
                    sibling = sibling->rightSibling;
                }
                sibling->rightSibling = newFile;
            }

        }
        //목적지 경로가 디렉토리가 아닌 경우
        else if(destination == NULL){
            char* destinationName = strrchr(destinationPath, '/');
            if(destinationName != NULL){
                *destinationName = '\0';
                destination = findRoute(destinationPath);
                *destinationName = '/';
                destinationName++;
            }
            else{
                destination = dirTree->current;
                destinationName = destinationPath;
            }
            
            if(destination == NULL || destination->type != 'd'){
                printf("cp: cannot create '%s': No such directory\n", destinationPath);
                return;
            }
            if(source->type == '-'){
                Directory * newFile = createNewDirectory(source->name, "644");
                newFile->type = '-';
                newFile->size = source->size;
                newFile->parent = destination;
                addDirectoryRoute(newFile, destination, destinationName);

                if(strcmp(source->name, newFile->name) != 0){
                    copyFile(source, newFile);
                }

                if(destination->leftChild == NULL){
                    destination->leftChild = newFile;
                }
                else{
                    Directory * sibling = destination->leftChild;
                    while(sibling->rightSibling != NULL){
                        sibling = sibling->rightSibling;
                    }
                    sibling->rightSibling = newFile;
                    
                }
            }
            else if(source->type == 'd'){
                if(recursive == false){
                    printf("cp: %s is a directory (not copied).\n", source->name);
                    return;
                }
                else{
                    if(destination->type == 'd'){
                        copyDirectory(source, destination, recursive, destinationName);
                        return;
                    }
                    else{
                        if(destination->type == 'd'){
                            copyDirectory(source, destination, recursive, destinationName);
                        }
                        else{
                            printf("cp: cannot overwrite non-directory '%s' with directory '%s'\n", destinationPath, sourcePath);
                        }
                    }
                }
            }
        }

        updateDirectoryFile();
    
    } 
    //pwd
    else if(strcmp(command, "pwd") == 0){
        printf("%s\n", dirTree->current->route);
    }
    //mv
    else if(strcmp(command, "mv") == 0){
        bool recursive = false;
        char * command = strtok_r(NULL, " ", &saveptr);

        //mv -r
        if (command != NULL && strcmp(command,'-r') == 0){
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
            pritnf("mv: cannot stat '%s': No such file or directory\n", sourcePath);
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
                sprintf(sourcePath, sizeof(sourcePath), "../my_os/information/resources/file/%s", source->name);
                
                char destinationPath_[256];
                sprintf(destinationPath_, sizeof(destinationPath_), "../my_os/information/resources/file/%s", newName);

                strcpy(source->name, newName);
                rename(sourcePath, destinationPath_);
                updateDirectoryFile();
            }
        }

    }
 
    
}
