#include "../../header/Header.h"

// 외부 함수 선언: mkdir.c에 구현된 스레드 함수
extern void* makeDirectory(void* arg);

// 경로에서 디렉터리와 파일명 분리
static void split_path_filename(const char* fullpath, char* dirpath, char* filename) {
    const char* last_slash = strrchr(fullpath, '/');
    if (last_slash) {
        size_t dir_len = last_slash - fullpath;
        if (dir_len >= MAX_ROUTE) dir_len = MAX_ROUTE - 1;
        strncpy(dirpath, fullpath, dir_len);
        dirpath[dir_len] = '\0';
        strncpy(filename, last_slash + 1, MAX_ROUTE - 1);
        filename[MAX_ROUTE - 1] = '\0';
    } else {
        dirpath[0] = '\0';
        strncpy(filename, fullpath, MAX_ROUTE - 1);
        filename[MAX_ROUTE - 1] = '\0';
    }
}

// RLE 압축 함수
void rle_compress(FILE *in, FILE *out) {
    int count = 0;
    int curr_char, prev_char = EOF;

    while ((curr_char = fgetc(in)) != EOF) {
        if (curr_char == prev_char) {
            count++;
            if (count == 255) {
                fputc(prev_char, out);
                fputc(count, out);
                count = 0;
            }
        } else {
            if (prev_char != EOF) {
                fputc(prev_char, out);
                fputc(count, out);
            }
            prev_char = curr_char;
            count = 1;
        }
    }
    if (prev_char != EOF) {
        fputc(prev_char, out);
        fputc(count, out);
    }
}

// RLE 압축 해제 함수
void rle_decompress(FILE *in, FILE *out) {
    int ch, count;
    while ((ch = fgetc(in)) != EOF) {
        count = fgetc(in);
        if (count == EOF) break;
        for (int i = 0; i < count; i++) {
            fputc(ch, out);
        }
    }
}

// 파일명 중복 처리 함수 추가
static void get_unique_filename(const char* original_name, char* unique_name, size_t size) {
    // 버퍼 초기화
    memset(unique_name, 0, size);
    
    // 파일명과 확장자 분리
    char name_without_ext[MAX_ROUTE] = {0};
    char ext[32] = {0};
    
    const char* dot = strrchr(original_name, '.');
    if (dot) {
        // 확장자가 있는 경우
        size_t name_len = dot - original_name;
        if (name_len >= sizeof(name_without_ext)) {
            name_len = sizeof(name_without_ext) - 1;
        }
        strncpy(name_without_ext, original_name, name_len);
        
        if (strlen(dot) < sizeof(ext)) {
            strcpy(ext, dot);
        }
    } else {
        // 확장자가 없는 경우
        strncpy(name_without_ext, original_name, sizeof(name_without_ext) - 1);
    }
    
    // 원본 파일명으로 시도
    strncpy(unique_name, original_name, size - 1);
    
    char temp_path[256] = {0};
    snprintf(temp_path, sizeof(temp_path), "information/resources/file/%s", unique_name);
    
    // 파일이 존재하지 않으면 원본 이름 사용
    if (access(temp_path, F_OK) != 0) {
        return;
    }
    
    // 파일이 존재하면 번호를 붙여가며 시도
    int counter = 1;
    while (counter < 1000) {  // 안전을 위해 최대 시도 횟수 제한
        memset(unique_name, 0, size);
        memset(temp_path, 0, sizeof(temp_path));
        
        if (ext[0] != '\0') {
            snprintf(unique_name, size - 1, "%s(%d)%s", name_without_ext, counter, ext);
        } else {
            snprintf(unique_name, size - 1, "%s(%d)", name_without_ext, counter);
        }
        
        snprintf(temp_path, sizeof(temp_path), "information/resources/file/%s", unique_name);
        
        if (access(temp_path, F_OK) != 0) {
            break;
        }
        counter++;
    }
}

static void create_directory_safely(const char* path, const char* mode, bool createParents) {
    MkdirArgs* args = (MkdirArgs*)calloc(1, sizeof(MkdirArgs));
    if (!args) {
        fprintf(stderr, "메모리 할당 실패\n");
        return;
    }

    // 경로 복사
    size_t path_len = strlen(path);
    if (path_len >= MAX_ROUTE) {
        path_len = MAX_ROUTE - 1;
    }
    memcpy(args->path, path, path_len);
    
    // 모드 설정
    size_t mode_len = strlen(mode);
    if (mode_len >= 4) {
        mode_len = 3;
    }
    memcpy(args->mode, mode, mode_len);
    
    args->createParents = createParents;

    pthread_t thread;
    void* result;
    if (pthread_create(&thread, NULL, makeDirectory, args) == 0) {
        pthread_join(thread, &result);
        return;
    }
    
    free(args);
}

static Directory* create_file_safely(const char* path, const char* mode, long size) {
    MkdirArgs* args = (MkdirArgs*)calloc(1, sizeof(MkdirArgs));
    if (!args) {
        fprintf(stderr, "메모리 할당 실패\n");
        return NULL;
    }

    // 경로 복사
    size_t path_len = strlen(path);
    if (path_len >= MAX_ROUTE) {
        path_len = MAX_ROUTE - 1;
    }
    memcpy(args->path, path, path_len);
    
    // 모드 설정
    size_t mode_len = strlen(mode);
    if (mode_len >= 4) {
        mode_len = 3;
    }
    memcpy(args->mode, mode, mode_len);
    
    args->createParents = false;

    pthread_t thread;
    void* result = NULL;
    if (pthread_create(&thread, NULL, makeDirectory, args) == 0) {
        pthread_join(thread, &result);
        Directory* newFile = (Directory*)result;
        if (newFile) {
            newFile->type = '-';
            newFile->size = size;
            updateDirectoryFile();
        }
        return newFile;
    }
    
    free(args);
    return NULL;
}

// 여러 파일을 하나의 압축파일로 묶는 함수
void zip_files(const char *zip_filename, char *filenames[], int file_count) {
    // zip 파일을 현재 디렉토리 기준으로 생성
    char zip_virtual_path[MAX_ROUTE] = {0};
    snprintf(zip_virtual_path, sizeof(zip_virtual_path), "%s/%s", dirTree->current->route, zip_filename);

    // 실제 파일 시스템상의 경로
    char zip_path[256] = {0};
    snprintf(zip_path, sizeof(zip_path), "information/resources/file%s", zip_virtual_path);

    // 상위 디렉토리가 없으면 생성
    char zip_dir[256] = {0};
    strncpy(zip_dir, zip_path, sizeof(zip_dir));
    char* lastSlash = strrchr(zip_dir, '/');
    if (lastSlash) {
        *lastSlash = '\0';
        char mkdirCmd[300];
        snprintf(mkdirCmd, sizeof(mkdirCmd), "mkdir -p %s", zip_dir);
        system(mkdirCmd);
    }

    FILE *zip_file = fopen(zip_path, "wb");
    if (!zip_file) {
        perror("압축 파일 생성 실패");
        return;
    }

    for (int i = 0; i < file_count; i++) {
        Directory* fileDir = findRoute(filenames[i]);
        if (!fileDir || fileDir->type != '-') {
            fprintf(stderr, "파일 찾기 실패: %s\n", filenames[i]);
            continue;
        }

        char file_path[256] = {0};
        snprintf(file_path, sizeof(file_path), "information/resources/file%s", fileDir->route);

        FILE *in = fopen(file_path, "rb");
        if (!in) {
            fprintf(stderr, "파일 열기 실패: %s\n", file_path);
            continue;
        }

        char virtual_path[MAX_ROUTE] = {0};
        strncpy(virtual_path, fileDir->route, MAX_ROUTE - 1);
        uint8_t name_len = (uint8_t)strlen(virtual_path);
        fwrite(&name_len, 1, 1, zip_file);
        fwrite(virtual_path, 1, name_len, zip_file);

        fseek(in, 0, SEEK_END);
        long filesize = ftell(in);
        fseek(in, 0, SEEK_SET);
        fwrite(&filesize, sizeof(long), 1, zip_file);

        FILE *temp = tmpfile();
        if (!temp) {
            fprintf(stderr, "임시 파일 생성 실패\n");
            fclose(in);
            fclose(zip_file);
            return;
        }

        rle_compress(in, temp);
        rewind(temp);

        fseek(temp, 0, SEEK_END);
        long compressed_size = ftell(temp);
        fseek(temp, 0, SEEK_SET);
        fwrite(&compressed_size, sizeof(long), 1, zip_file);

        char buffer[MAX_BUFFER];
        size_t n;
        while ((n = fread(buffer, 1, MAX_BUFFER, temp)) > 0) {
            fwrite(buffer, 1, n, zip_file);
        }

        fclose(temp);
        fclose(in);
    }

    fclose(zip_file);
    printf("압축 완료: %s\n", zip_virtual_path);
}


void unzip_files(const char *zip_filename) {
    char zip_virtual_path[MAX_ROUTE] = {0};
    snprintf(zip_virtual_path, sizeof(zip_virtual_path), "%s/%s", dirTree->current->route, zip_filename);

    char zip_path[256] = {0};
    snprintf(zip_path, sizeof(zip_path), "information/resources/file%s", zip_virtual_path);

    FILE *zip_file = fopen(zip_path, "rb");
    if (!zip_file) {
        perror("압축 파일 열기 실패");
        return;
    }

    while (1) {
        uint8_t name_len;
        if (fread(&name_len, 1, 1, zip_file) != 1) break;

        char filename[MAX_ROUTE] = {0};
        if (fread(filename, 1, name_len, zip_file) != name_len) {
            fprintf(stderr, "파일명 읽기 실패\n");
            break;
        }
        filename[name_len] = '\0';

        long orig_size, compressed_size;
        if (fread(&orig_size, sizeof(long), 1, zip_file) != 1) break;
        if (fread(&compressed_size, sizeof(long), 1, zip_file) != 1) break;

        // 디렉토리 생성
        char dirpath[MAX_ROUTE] = {0};
        char fname[MAX_ROUTE] = {0};
        split_path_filename(filename, dirpath, fname);

        if (strlen(dirpath) > 0) {
            create_directory_safely(dirpath, "755", true);
        }

        // 중복 방지 파일명 설정
        char unique_filename[MAX_ROUTE] = {0};
        strncpy(unique_filename, filename, sizeof(unique_filename) - 1);
        get_unique_filename(filename, unique_filename, sizeof(unique_filename));

        char file_path[256] = {0};
        snprintf(file_path, sizeof(file_path), "information/resources/file/%s", unique_filename);

        FILE *out = fopen(file_path, "wb");
        if (!out) {
            fprintf(stderr, "파일 생성 실패: %s\n", unique_filename);
            fseek(zip_file, compressed_size, SEEK_CUR);
            continue;
        }

        FILE *temp = tmpfile();
        if (!temp) {
            fprintf(stderr, "임시 파일 생성 실패\n");
            fclose(out);
            fclose(zip_file);
            return;
        }

        char buffer[MAX_BUFFER];
        long left = compressed_size;
        while (left > 0) {
            size_t to_read = (left > MAX_BUFFER) ? MAX_BUFFER : left;
            size_t n = fread(buffer, 1, to_read, zip_file);
            if (n == 0) break;
            fwrite(buffer, 1, n, temp);
            left -= n;
        }
        rewind(temp);

        rle_decompress(temp, out);
        fclose(out);
        fclose(temp);

        // 가상 디렉토리 반영
        create_file_safely(unique_filename, "644", orig_size);

        printf("압축 해제 완료: %s\n", unique_filename);
    }

    fclose(zip_file);
}
