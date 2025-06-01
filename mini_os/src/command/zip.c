#include "../../header/Header.h"

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

static void get_unique_filename(const char* original_name, char* unique_name, size_t size) {
    memset(unique_name, 0, size);
    char name_without_ext[MAX_ROUTE] = {0};
    char ext[32] = {0};
    const char* dot = strrchr(original_name, '.');
    if (dot) {
        size_t name_len = dot - original_name;
        if (name_len >= sizeof(name_without_ext)) name_len = sizeof(name_without_ext) - 1;
        strncpy(name_without_ext, original_name, name_len);
        if (strlen(dot) < sizeof(ext)) strcpy(ext, dot);
    } else {
        strncpy(name_without_ext, original_name, sizeof(name_without_ext) - 1);
    }

    strncpy(unique_name, original_name, size - 1);
    char temp_path[256] = {0};
    snprintf(temp_path, sizeof(temp_path), "information/resources/file/%s", unique_name);

    if (access(temp_path, F_OK) != 0) return;

    int counter = 1;
    while (counter < 1000) {
        memset(unique_name, 0, size);
        memset(temp_path, 0, sizeof(temp_path));
        if (ext[0] != '\0') {
            snprintf(unique_name, size - 1, "%s(%d)%s", name_without_ext, counter, ext);
        } else {
            snprintf(unique_name, size - 1, "%s(%d)", name_without_ext, counter);
        }
        snprintf(temp_path, sizeof(temp_path), "information/resources/file/%s", unique_name);
        if (access(temp_path, F_OK) != 0) break;
        counter++;
    }
}

static Directory* create_file_safely(const char* path, const char* mode, long size) {
    MkdirArgs* args = (MkdirArgs*)calloc(1, sizeof(MkdirArgs));
    if (!args) return NULL;
    size_t path_len = strlen(path);
    if (path_len >= MAX_ROUTE) path_len = MAX_ROUTE - 1;
    memcpy(args->path, path, path_len);
    size_t mode_len = strlen(mode);
    if (mode_len >= 4) mode_len = 3;
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

void zip_files(const char *zip_filename, char *filenames[], int file_count) {
    char zip_path[256] = {0};
    snprintf(zip_path, sizeof(zip_path), "information/resources/file/%s", zip_filename);

    FILE *zip_file = fopen(zip_path, "wb");
    if (!zip_file) {
        perror("압축 파일 생성 실패");
        return;
    }

    for (int i = 0; i < file_count; i++) {
        // 실제 파일 경로
        char file_path[256] = {0};
        snprintf(file_path, sizeof(file_path), "information/resources/file/%s", filenames[i]);

        FILE *in = fopen(file_path, "rb");
        if (!in) {
            printf("파일 열기 실패: %s\n", file_path);
            continue;
        }

        // 파일 이름만 추출
        char *base = strrchr(filenames[i], '/');
        base = base ? base + 1 : filenames[i];

        uint8_t name_len = (uint8_t)strlen(base);
        fwrite(&name_len, 1, 1, zip_file);
        fwrite(base, 1, name_len, zip_file);

        fseek(in, 0, SEEK_END);
        long filesize = ftell(in);
        fseek(in, 0, SEEK_SET);
        fwrite(&filesize, sizeof(long), 1, zip_file);

        FILE *temp = tmpfile();
        if (!temp) { fclose(in); fclose(zip_file); return; }
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
    printf("압축 완료: %s\n", zip_path);
}

void unzip_files(const char *zip_filename) {
    char zip_path[256] = {0};
    snprintf(zip_path, sizeof(zip_path), "information/resources/file/%s", zip_filename);

    FILE *zip_file = fopen(zip_path, "rb");
    if (!zip_file) {
        perror("압축 파일 열기 실패");
        return;
    }

    while (1) {
        uint8_t name_len;
        if (fread(&name_len, 1, 1, zip_file) != 1) break;

        char filename[MAX_ROUTE] = {0};
        if (fread(filename, 1, name_len, zip_file) != name_len) break;
        filename[name_len] = '\0';

        long orig_size, compressed_size;
        if (fread(&orig_size, sizeof(long), 1, zip_file) != 1) break;
        if (fread(&compressed_size, sizeof(long), 1, zip_file) != 1) break;

        char unique_filename[MAX_ROUTE] = {0};
        get_unique_filename(filename, unique_filename, sizeof(unique_filename));

        char file_path[256] = {0};
        snprintf(file_path, sizeof(file_path), "information/resources/file/%s", unique_filename);

        FILE *out = fopen(file_path, "wb");
        if (!out) { fseek(zip_file, compressed_size, SEEK_CUR); continue; }

        FILE *temp = tmpfile();
        if (!temp) { fclose(out); fclose(zip_file); return; }

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

        create_file_safely(unique_filename, "644", orig_size);
        printf("압축 해제 완료: %s\n", unique_filename);
    }

    fclose(zip_file);
}