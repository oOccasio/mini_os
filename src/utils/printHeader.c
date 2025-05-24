#include "../header/Header.h"

void printHeader(DirectoryTree * workingDirectory, User * user){
    BOLD; GREEN;
    printf("%s@1-os-linux", user->name);
    DEFAULT;

    printf(":");

    BOLD; BLUE;
    printf("%s", workingDirectory->current->route);
    DEFAULT;

    printf("#");
}