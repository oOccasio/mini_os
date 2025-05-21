#include "../header/Header.c"

//스레드 부분 공부 후에 리팩토링 예정정
void* changeMode(void* arg) {
    ChmodArgs* args = (ChmodArgs*)arg;
    char* path = args->path;
    char* mode = args->mode;

    Directory* targetDirectory = findRoute(path);
    if (targetDirectory == NULL) {
        printf("chmod: cannot access '%s': No such file or directory\n", path);
    } else {
        setPermission(targetDirectory, mode);
    }

    free(arg);
    pthread_exit(NULL);
}