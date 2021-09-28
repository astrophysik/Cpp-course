#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Haffman.h"


typedef union Header {
    struct {
        int size_of_file;
        int size_of_header;
        int number_of_names;
    };
    char buffer[12];
}Header_of_archive;

typedef union info_of_file {
    struct {
        int len_of_name;
        int size;
    };
    char buffer[8];
} info_of_files;

void creation(char*name_of_archive, char * files_names[], int number_of_files) {
    FILE *file;
    FILE *buffer;
    for (int i = 0; i < number_of_files; i++)
        encoding(files_names[i]);
    file = fopen(name_of_archive, "wb");
    int size_of_file = 0;
    info_of_files info_of_this_files[number_of_files];
    for (int i = 0; i < number_of_files; i++) {
        buffer = fopen(files_names[i], "rb");
        fseek(buffer, 0, SEEK_END);
        info_of_this_files[i].size = ftell(buffer);
        info_of_this_files[i].len_of_name = strlen(files_names[i]);
        size_of_file += ftell(buffer);
        fclose(buffer);
    }
    Header_of_archive this_file_header;
    this_file_header.size_of_file = size_of_file;
    this_file_header.size_of_header = 12;
    this_file_header.number_of_names = number_of_files;
    fwrite(this_file_header.buffer, sizeof(char), 12, file);
    for (int i = 0; i < number_of_files; i++) {
        fwrite(info_of_this_files[i].buffer, sizeof(char), 8, file);
    }
    for (int i = 0; i < number_of_files; i++) {
        buffer = fopen(files_names[i], "rb");
        fwrite(files_names[i], sizeof(char), info_of_this_files[i].len_of_name, file);
        copyfile(buffer, file);
        fclose(buffer);
        remove(files_names[i]);
    }
    fclose(file);
}

void list(char* name_of_archive) {
    FILE* file = fopen(name_of_archive, "rb");
    Header_of_archive Header_of_this_archive;
    fread(Header_of_this_archive.buffer, sizeof (char), 12, file);
    info_of_files info_of_this_files[Header_of_this_archive.number_of_names];
    for (int i = 0; i < Header_of_this_archive.number_of_names; i++) {
        fread(info_of_this_files[i].buffer, sizeof(char), 8, file);
    }
    for (int i = 0; i < Header_of_this_archive.number_of_names; i++) {
        char *name = malloc(sizeof(char)*info_of_this_files[i].len_of_name + 1);
        fread(name, sizeof(char), info_of_this_files[i].len_of_name, file);
        name[info_of_this_files[i].len_of_name] = '\0';
        printf("%s\n", name);
        fseek(file, info_of_this_files[i].size, SEEK_CUR);
        free(name);
    }
    fclose(file);

}

void extract(char* name_of_archive) {
    FILE * file = fopen(name_of_archive, "rb");
    FILE * output;
    Header_of_archive Header_of_this_archive;
    fread(Header_of_this_archive.buffer, sizeof (char), 12, file);
    info_of_files info_of_this_files[Header_of_this_archive.number_of_names];
    for (int i = 0; i < Header_of_this_archive.number_of_names; i++) {
        fread(info_of_this_files[i].buffer, sizeof(char), 8, file);
    }
    for (int i = 0; i < Header_of_this_archive.number_of_names; i++) {
        char * name = malloc(sizeof(char) * info_of_this_files[i].len_of_name + 1);
        fread(name, sizeof(char), info_of_this_files[i].len_of_name, file);
        name[info_of_this_files[i].len_of_name] = '\0';
        output = fopen(name, "wb");
        int start_cur = ftell(file);
        int symbol;
        while (ftell(file) < start_cur + info_of_this_files[i].size) {
            symbol = getc(file);
            putc(symbol, output);
        }
        fclose(output);
        decoding(name);
    }
    fclose(file);
    remove(name_of_archive);
}

int main(int argc, char* argv[]) {
    char *name_of_archive;
    char *name_of_files[10];
    int number_of_files = 0;
    char * command;
    for (int i = 1; i < argc; i++) {
        if (argv[i][2] == 'f') {
            name_of_archive = argv[i + 1];
            continue;
        }
        if  (argv[i][2] == 'c' && argv[i][1] == '-') {
            command = "create";
            int k = i + 1;
            while (k < argc) {
                name_of_files[k - 5] = argv[k];
                number_of_files++;
                k++;
            }
            break;
        }
        if (argv[i][2] == 'e') {
            command = "extract";
            continue;
        }
        if (argv[i][2] == 'l') {
            command = "list";
            continue;
        }
    }
    //char * command = "extract";
    //char *name_of_archive = "data.arc";
    //char *name_of_files[10] = {"a.txt", "b.bmp"};
    //int number_of_files = 2;
    //for (int i = 0; i < number_of_files; i++) {
    //    printf("%s\n", name_of_files[i]);
    //}
    if (strcmp(command, "create") == 0) {
        creation(name_of_archive, name_of_files, number_of_files);
    }
    if (strcmp(command, "list") == 0) {
        list(name_of_archive);
    }
    if (strcmp(command, "extract") == 0) {
        extract(name_of_archive);
    }
    return 0;
}
