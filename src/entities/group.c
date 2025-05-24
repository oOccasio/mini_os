#include "../header/Header.h"


Group* parseGroupLine(char* line);
void addGroupToList(GroupList** groupList, Group* newGroup);



char* findGroupById(int GID){
    GroupList * currentGroup = groupList;
    while (currentGroup != NULL){
        if (currentGroup->group->GID == GID){
            return currentGroup->group->name;
        }
        currentGroup = currentGroup->nextGroup;
    }
    return NULL;
}



void loadGroup() {
    FILE* groupFile = fopen("../information/Group.txt", "r");
    if (!groupFile) {
        fprintf(stderr, "Error: Unable to open Group.txt\n");
        return;
    }

    groupList = NULL;
    char tmp[MAX_LENGTH];

    while (fgets(tmp, MAX_LENGTH, groupFile)) {
        size_t len = strlen(tmp);
        if (len > 0 && tmp[len - 1] == '\n') {
            tmp[len - 1] = '\0';
        }

        Group* newGroup = parseGroupLine(tmp);
        if (!newGroup) {
            fprintf(stderr, "Failed to parse group line\n");
            continue;
        }

        addGroupToList(&groupList, newGroup);
    }
    fclose(groupFile);
}



Group* parseGroupLine(char* line) {
    Group* newGroup = malloc(sizeof(Group));
    if (!newGroup) {
        fprintf(stderr, "Memory allocation failed for Group\n");
        return NULL;
    }

    char* ptr = strtok(line, " ");
    if (!ptr) { free(newGroup); return NULL; }
    strncpy(newGroup->name, ptr, MAX_NAME - 1);
    newGroup->name[MAX_NAME - 1] = '\0';

    ptr = strtok(NULL, " ");
    if (!ptr) { free(newGroup); return NULL; }
    newGroup->GID = atoi(ptr);

    return newGroup;
}

void addGroupToList(GroupList** groupList, Group* newGroup) {
    if (!newGroup) return;

    if (*groupList == NULL) {
        *groupList = malloc(sizeof(GroupList));
        if (!*groupList) {
            fprintf(stderr, "Memory allocation failed for GroupList\n");
            free(newGroup);
            return;
        }
        (*groupList)->group = newGroup;
        (*groupList)->nextGroup = NULL;
    } else {
        GroupList* tmp = *groupList;
        while (tmp->nextGroup) {
            tmp = tmp->nextGroup;
        }
        GroupList* newNode = malloc(sizeof(GroupList));
        if (!newNode) {
            fprintf(stderr, "Memory allocation failed for GroupList\n");
            free(newGroup);
            return;
        }
        newNode->group = newGroup;
        newNode->nextGroup = NULL;
        tmp->nextGroup = newNode;
    }
}


