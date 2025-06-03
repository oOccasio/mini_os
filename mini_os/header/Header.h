#ifndef HEADER_H_
#define HEADER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <utime.h>
#include <pthread.h>
#include <stdint.h>
#include <unistd.h>   

//Max 길이 설정
#define MAX_BUFFER 512
#define MAX_LENGTH 200
#define MAX_ROUTE 100
#define MAX_DIR 50
#define MAX_NAME 20
#define MAX_THREAD 50
#define MAX_QUEUE_SIZE 100
#define MAX_STRING_LENGTH 100

//글자 스타일 및 색상
#define DEFAULT printf("%c[%dm", 0x1B, 0)
#define BOLD printf("%c[%dm", 0x1B, 1)
#define WHITE printf("\x1b[37m")
#define BLUE printf("\x1b[34m")
#define GREEN printf("\x1b[32m")
#define RED printf("\x1b[31m")
#define RESET printf("\x1b[0m")

// User
typedef struct UserNode{
    char name[MAX_NAME];
    char dir[MAX_ROUTE];
    int UID;
    int GID;
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int sec;
    int wday; // 요일
} User;

// UserList
typedef struct UserNodeList{
    struct UserNode * user; //currentUser 가 더 명확해보임
    struct UserNodeList * nextUser;
} UserList;

// Group
typedef struct GroupNode{
    char name[MAX_NAME];
    int GID;
} Group;

// GroupList
typedef struct GroupNodeList{
    struct GroupNode * group;
    struct GroupNodeList * nextGroup;
} GroupList;

// Directory
typedef struct DirectoryNode{
    char name[MAX_NAME];
    char type;
    bool visible;
    bool permission[9];
    int UID;
    int GID;
    int size;
    // int year;
    int month;
    int day;
    int hour;
    int minute;
    //int second;
    char route[MAX_ROUTE];
    struct DirectoryNode * parent; //상위 디렉토리
    struct DirectoryNode * leftChild;     //하위 디렉토리
    struct DirectoryNode * rightSibling;  // 이웃 디렉토리
} Directory;

// DirectoryTree
typedef struct DirectoryNodeTree{
    Directory * root;
    Directory * home;
    Directory * current;
} DirectoryTree;


//스레드 구조(좀더 공부하고 채움)
typedef struct{
    Directory * directory;
    bool showAll;
    bool showDetails;
} ListArgs;

//Makdir 인자
typedef struct {
    char path[MAX_ROUTE];
    char mode[4];  //4인 이유는 3자리(755) + '/0' 널문자 + 1
    bool createParents;
} MkdirArgs;

//Chmod 인자
typedef struct{
    char path[MAX_ROUTE];
    char mode[4];
} ChmodArgs;

// rmdir 인자
typedef struct {
    char* dirPath;
    bool recursive;
} RmdirArgs;

//전역 변수
DirectoryTree* dirTree;
User* loginUser;
UserList * userList;
GroupList * groupList;

//Queue
typedef struct{
    int front;
    int rear;
    int size;
    char * items[MAX_QUEUE_SIZE];
} Queue;

//Queue 관리 코드
//utills/queue.c
void initQueue(Queue * queue);
bool isEmpty(Queue * queue);
bool isFull(Queue * queue);
void enqueue(Queue * queue, const char * str);
char * dequeue(Queue * queue);
char * peek(Queue * queue);
void freeQueue(Queue * queue);
void buildQueue(Queue * queue, char * queuestr);

//User 관리 코드
//user.c
char * findUserById(int UID);
void loadUser();

//Group 관리코드
//group.c
char * findGroupById(int GID);
void loadGroup();

//디렉토리 관리 코드
// directory.c
void writeDirectoryToFile(FILE * file, Directory * directory);
void updateDirectoryFile();
void setDirectoryTime(Directory * directory);
void getMonth(int month);
void buildDirectoryRoute(Queue *queue, Directory * parent, Directory* myAddress);
void buildDirectoryNode(char* dirstr);
Directory* loadDirectory();
Directory* findRouteRecursive(Queue * queue, Directory* parent);
Directory* findRoute(char* pathOrigin);


//허가 관리 코드
// permission.c
void atoiPermission(Directory * directory, char * perstr);
void setPermission(Directory * directory, const char * mode);

//utility 관련 코드
int countLink(Directory * directory);

//Command 관련 코드
void classificationCommand(char * cmd);


//ls
//ls.c
void showDirectoryDetail(Directory * temp, char* name);
void listDirectory(Directory * Directory, bool showAll, bool showDetails);
void * listDirectoryThread(void * arg);

//mkdir
Directory* createNewDirectory(char* name, const char* mode);
void addDirectoryRoute(Directory* newDir, Directory* parent, char* dirName);
void* makeDirectory(void* arg);

// chmod
void * changeMode(void * arg);

// cd
void changeDirectory(char* path);


// cat
void catFiles(char* fileNames[], int fileCount, bool showLineNumber);
void createFile(const char* fileName);
void appendFile(const char* fileName);


// cp
void copyFile(Directory* source, Directory* destination);
void copyDirectory(Directory* source, Directory* destinationParent, bool recursive, char* folderName);

// mv.c
void moveFile(Directory * source, Directory * destination, char * newName);
void moveDirectory(Directory * source, Directory * destination, char * newName, bool recursive);
void updateRouteRecursive(Directory* dir);




//rmdir
void freeDirectory(Directory* dir);
void removeDirectory(Directory* dir);
void removeDirectoryRecursive(Directory* dir);
void removeDirectoryThread(char* dirPaths[], int dirCount, bool recursive);
void writeDirectoryToFile(FILE* file, Directory* directory);
void* rmdirThread(void* arg);

//zip
void zip_files(const char* zip_filename, char* filenames[], int file_count);
void unzip_files(const char* zip_filename);
static void create_directory_safely(const char* path, const char* mode, bool createParents);
static Directory* create_file_safely(const char* path, const char* mode, long size);

//clear
int clear();

//pwd
int pwd(DirectoryTree *dirTree, Queue *dirQueue, char *option);

//useradd
User* adduser(char* username, int UID, int GID, DirectoryTree* dirTree, UserList** userTree);

//login.c
User* login();

//printHeader
void printHeader(DirectoryTree* workingDirectory, User* user);

#endif
