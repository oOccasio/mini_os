#include "../header/Header.h"

void showDirectoryDetail(Directory* temp, char * name){
    
    char permission[11] = {0};
    permission[0] = (temp->type == 'd') ? 'd' : '-';
    permission[1] = (temp->permission[0]) ? 'r' : '-';
    permission[2] = (temp->permission[1]) ? 'w' : '-';
    permission[3] = (temp->permission[2]) ? 'x' : '-';
    permission[4] = (temp->permission[3]) ? 'r' : '-';
    permission[5] = (temp->permission[4]) ? 'w' : '-';
    permission[6] = (temp->permission[5]) ? 'x' : '-';
    permission[7] = (temp->permission[6]) ? 'r' : '-';
    permission[8] = (temp->permission[7]) ? 'w' : '-';
    permission[9] = (temp->permission[8]) ? 'x' : '-';

    char*userName = findUserById(temp->UID);
    char*groupName = findGroupById(temp->GID);
    int link = countLink(temp);

    printf("%s %2d %-8s %-8s %5d ", 
        permission,
        link,
        userName,
        groupName,
        temp->size
    );

    getMonth(temp->month);
    printf("%02d %02d:%02d ",
        temp->day,
        temp->hour,
        temp->minute
    );

    if(temp->type == 'd') {
        BLUE;
        printf("%s\n", name);
        DEFAULT;
    } else if(temp->type == '-') {
        printf("%s\n", name);
    }
    
}


void listDirectory(Directory *directory, bool showAll, bool showDetails) {
    Directory* temp = directory->leftChild;

    if (temp == NULL && !showAll) {
        return;
    }

    int mode = (showAll ? 2 : 0) + (showDetails ? 1 : 0);

    switch (mode) {
        case 0: // showAll = false, showDetails = false
            // 기본: 아무것도 안 보여줌 (필요 시 내용 추가)
            break;

        case 1: // showAll = false, showDetails = true
            // 현재 디렉토리 상세 정보만 보여줌
            showDirectoryDetail(directory, ".");
            showDirectoryDetail(directory->parent, "..");
            break;

        case 2: // showAll = true, showDetails = false
            // 숨겨진 파일 포함, 간략 출력
            BLUE;
            printf("%-10s", ".");
            printf("%-10s", "..");
            DEFAULT;
            break;

        case 3: // showAll = true, showDetails = true
            // 숨겨진 파일 포함, 상세 출력
            showDirectoryDetail(directory, ".");
            showDirectoryDetail(directory->parent, "..");
            break;
    }

    while (temp != NULL) {
        if (temp->visible || showAll) {
            if (showDetails) {
                showDirectoryDetail(temp, temp->name);
            } else {
                if (temp->type == 'd') {
                    BLUE;
                    printf("%-10s", temp->name);
                    DEFAULT;
                } else if (temp->type == '-') {
                    printf("%-10s", temp->name);
                }
            }
        }
        temp = temp->rightSibling;
    }

    if (!showDetails) {
        printf("\n");
    }
}




//스레드 관련 공부 후 리팩토링 예정
void* listDirectoryThread(void* arg) {
    ListArgs* data = (ListArgs*)arg;
    Directory* directory = data->directory;
    bool showAll = data->showAll;
    bool showDetails = data->showDetails;

    printf("%s:\n", directory->name);
    listDirectory(directory, showAll, showDetails);
    printf("\n");

    free(data);
    pthread_exit(NULL);
}