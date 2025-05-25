#include "../header/Header.h"

// 이전 디렉토리 저장용
static Directory *prev_directory = NULL;

void changeDirectory(char *path) {
    if (path == NULL || strcmp(path, "~") == 0) {
        // 홈 디렉토리로 이동
        if (dirTree->home != NULL) {
            prev_directory = dirTree->current;
            dirTree->current = dirTree->home;
        } else {
            fprintf(stderr, "cd: 홈 디렉토리가 설정되어 있지 않습니다.\n");
        }
    }
    else if (strcmp(path, "-") == 0) {
        // 이전 디렉토리로 이동
        if (prev_directory != NULL) {
            Directory *temp = dirTree->current;
            dirTree->current = prev_directory;
            prev_directory = temp;
            printf("%s\n", dirTree->current->route);
        } else {
            fprintf(stderr, "cd: 이전 디렉토리가 존재하지 않습니다.\n");
        }
    }
    else {
        // 일반 경로 이동
        Directory *target = findRoute(path);
        if (target != NULL) {
            if (target->type == 'd') {
                prev_directory = dirTree->current;
                dirTree->current = target;
            } else {
                fprintf(stderr, "cd: '%s'는 디렉토리가 아닙니다.\n", path);
            }
        } else {
            fprintf(stderr, "cd: 경로 '%s'를 찾을 수 없습니다.\n", path);
        }
    }
}

