#include "../header/Header.h"

int main(){
    char cmd[100];
    loadUser();
    loadGroup();
    DirectoryTree *miniOS = loadDirectory();
    if (miniOS == NULL){
        fprintf(stderr, "failed to load directory tree\n");
        return 1;
    }

    loginUser = login();
    if (loginUser == NULL){
        fprintf(stderr, "Login failed\n");
        return 1;
    }

    printf("Debug: User directory path = '%s'\n", loginUser->dir);
    Directory* loginDirectory = findRoute(loginUser->dir);
    if (loginDirectory == NULL){
        fprintf(stderr, "Invalid user directory path\n");
        return 1;
    }

    miniOS->home = loginDirectory;
    miniOS->current = loginDirectory;

    while(true){
        printHeader(miniOS, loginUser);
        if (fgets(cmd, sizeof(cmd),stdin) == NULL){
            printf("Input error or EOF\n");
            break;
        }
        cmd[strlen(cmd) - 1] = '\0';
        classificationCommand(cmd);
    }

    return 0;
}