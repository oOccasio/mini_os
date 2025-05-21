#include "../my_os/header/Header.h"

int main(){
    char cmd[100];
    loadUser();
    loadGroup();
    DirectoryTree * miniOS = loadDirectory();
    loginUser = login();
    Directory* loginDirectory = findRoute(loginUser->dir);
    miniOS->home = loginDirectory;
    miniOS->current = loginDirectory;

    while(true){
        printHeader(miniOS, loginUser);
        fgets(cmd, sizeof(cmd), stdin);
        cmd[strlen(cmd) - 1] = '\0';
        classificationCommand(cmd);
    }

    return 0;
}