#include "../header/Header.h"
#include <sys/stat.h>

void* changeMode(void* arg) {
    ChmodArgs* args = (ChmodArgs*)arg;
    char* path = args->path;
    char* mode = args->mode;

    Directory* targetDirectory = findRoute(path);
    if (targetDirectory == NULL) {
        printf("chmod: cannot access '%s': No such file or directory\n", path);
    } else {
        // 가상 디렉토리 권한 변경
        setPermission(targetDirectory, mode);

        // 실제 파일 시스템 권한 변경
        if (targetDirectory->type == '-') {
            char filePath[256];
            snprintf(filePath, sizeof(filePath), "information/resources/file/%s", targetDirectory->name);
            
            // 8진수 문자열을 정수로 변환
            int modeInt = 0;
            for (int i = 0; i < 3; i++) {
                modeInt = modeInt * 8 + (mode[i] - '0');
            }
            
            if (chmod(filePath, modeInt) != 0) {
                perror("chmod failed");
            }
        }
    }

    free(arg);
    pthread_exit(NULL);
}