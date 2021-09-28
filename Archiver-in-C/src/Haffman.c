#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Haffman.h"

void copyfile(FILE* input, FILE* output) {
    int symbol;
    while ((symbol = getc(input)) != EOF)
        putc(symbol, output);
}

node * creation_of_tree(node * nodes[], int len) {
    node *temporary;
    temporary = (node*)malloc(sizeof(node));
    temporary->prediction = nodes[len - 1]->prediction + nodes[len - 2]->prediction;
    temporary->left = nodes[len - 1];
    temporary->right = nodes[len - 2];
    temporary->code[0] = 0;
    if (len == 2)
        return temporary;
    for (int i = 0; i < len; i++) {
        if (temporary->prediction > nodes[i]->prediction) {
            for (int j = len - 1; j > i; j--) {
                nodes[j] = nodes[j - 1];
            }
            nodes[i] = temporary;
            break;
        }
    }
    return creation_of_tree(nodes, len - 1);
}

void counting_codes(node * root) {
    if (root->left) {
        strcpy(root->left->code, root->code);
        strcat(root->left->code, "1");
        counting_codes(root->left);
    }
    if (root->right) {
        strcpy(root->right->code, root->code);
        strcat(root->right->code, "0");
        counting_codes(root->right);
    }
}

void long_sort(node * nodes, int len) {
    for (int i = 0; i < len; i++) {
        nodes[i].left = NULL;
        nodes[i].right = NULL;
        float key = nodes[i].prediction;
        node temporary = nodes[i];
        int j = i - 1;
        while (j >= 0 && key > nodes[j].prediction) {
            nodes[j + 1] = nodes[j];
            j--;
        }
        nodes[j + 1] = temporary;
    }
}

void creation_of_array(float * predictions, char *symbols, int len, struct node *nodes) {
    for (int i = 0; i < len; i++) {
        nodes[i].prediction = predictions[i];
        nodes[i].meaning = symbols[i];
    }
}

void encoding(char* name_of_file) {
    FILE *file = fopen(name_of_file, "rb");
    char *symbols = malloc(sizeof(char));
    int *meanings = malloc(sizeof(int));
    char symbol[1];
    int number_of_unique = 0;
    int number_of_all = 0;
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    while (ftell(file) < size) {
        int flag = 0;
        number_of_all++;
        fread(symbol, sizeof(char), 1, file);
        for (int i = 0; i < number_of_unique; i++) {
            if (symbols[i] == symbol[0]) {
                flag = 1;
                meanings[i]++;
            }
        }
        if (flag == 1)
            continue;
        number_of_unique++;
        symbols = realloc(symbols, sizeof(char)*number_of_unique);
        meanings = realloc(meanings, sizeof(int)*number_of_unique);
        symbols[number_of_unique - 1] = symbol[0];
        meanings[number_of_unique - 1] = 1;
    }
    float probabilities[number_of_unique];
    char list_of_symbols[number_of_unique];
    for (int i = 0; i < number_of_unique; i++) {
        list_of_symbols[i] = symbols[i];
    }
    for (int i = 0; i < number_of_unique; i++) {
        probabilities[i] = (float)meanings[i]/(float)number_of_all;
    }
    free(meanings);
    free(symbols);
    //// Кодирование
    node nodes[number_of_unique];
    creation_of_array(probabilities, list_of_symbols, number_of_unique, nodes);
    long_sort(nodes, number_of_unique);
    node *new_node[number_of_unique];
    for (int i = 0; i < number_of_unique; i++)
        new_node[i] = &nodes[i];
    node * root = creation_of_tree(new_node, number_of_unique);
    counting_codes(root);
    //// Обработка файла
    fseek(file, 0, SEEK_SET);
    FILE *output = fopen("process_of_encoding.txt", "wb");
    char buffer;
    while (ftell(file) < size) {
        fread(&buffer, sizeof(char ), 1, file);
        for (int i = 0; i < number_of_unique; i++) {
            if (buffer == nodes[i].meaning) {
                fwrite(nodes[i].code, sizeof(char), strlen(nodes[i].code), output);
            }
        }
    }
    fclose(file);
    fclose(output);
    file = fopen(name_of_file, "wb");
    output = fopen("process_of_encoding.txt", "rb");
    fseek(output, 0, SEEK_END);
    int size_two = ftell(output);
    int remainder = ftell(output) % 8;
    fwrite(&number_of_unique, sizeof(int), 1, file);
    fwrite(&remainder, sizeof(int), 1, file);
    for (int i = 0; i < number_of_unique; i++) {
        fwrite(&nodes[i].meaning, sizeof(char), 1, file);
        fwrite(&nodes[i].prediction, sizeof(float), 1, file);
    }
    fseek(output, 0, SEEK_SET);
    union code code_for_this_file;
    int j = 0;
    char mes[8] = {0};
    for (int i = 0; i < size_two - remainder; i++) {
        fread(&mes[j], sizeof(char ), 1, output);
        if (j == 7) {
            code_for_this_file.byte.b1 = mes[0] - '0';
            code_for_this_file.byte.b2 = mes[1] - '0';
            code_for_this_file.byte.b3 = mes[2] - '0';
            code_for_this_file.byte.b4 = mes[3] - '0';
            code_for_this_file.byte.b5 = mes[4] - '0';
            code_for_this_file.byte.b6 = mes[5] - '0';
            code_for_this_file.byte.b7 = mes[6] - '0';
            code_for_this_file.byte.b8 = mes[7] - '0';
            fwrite(&code_for_this_file.str, sizeof(char ), 1, file);
            j = 0;
            continue;
        }
        j++;
    }
    char rem_mes[remainder];
    fread(rem_mes, sizeof(char ), remainder, output);
    fwrite(rem_mes, sizeof(char ), remainder, file);
    fclose(output);
    fclose(file);
    remove("process_of_encoding.txt");
}

void decoding(char * name_of_file) {
    ////Обработка данных в файле
    FILE *file = fopen(name_of_file, "rb");
    int numbers_of_unique;
    int remainder;
    fseek(file, 0, SEEK_END);
    int size = ftell(file);
    fseek(file, 0, SEEK_SET);
    fread(&numbers_of_unique, sizeof(int), 1, file);
    fread(&remainder, sizeof(int), 1, file);
    char symbols[numbers_of_unique];
    float probabilities[numbers_of_unique];
    for (int i = 0; i < numbers_of_unique; i++) {
        fread(&symbols[i], sizeof(char), 1, file);
        fread(&probabilities[i], sizeof(float), 1, file);
    }
    ////Построение дерева и нахождение кодов по данным из файла
    node nodes[numbers_of_unique];
    creation_of_array(probabilities, symbols, numbers_of_unique, nodes);
    long_sort(nodes, numbers_of_unique);
    node * new_node[numbers_of_unique];
    for (int i = 0; i < numbers_of_unique; i++) {
        new_node[i] = &nodes[i];
    }
    node * root = creation_of_tree(new_node, numbers_of_unique);
    counting_codes(root);
    /////Перзапись char -> char * 8
    FILE *output = fopen("process_of_decoding.txt", "wb");
    union code this_file_decode;
    char wet[8];
    while (ftell(file) + remainder < size) {
        fread(&this_file_decode.str, sizeof(char ), 1, file);
        wet[0] = this_file_decode.byte.b1 + '0';
        wet[1] = this_file_decode.byte.b2 + '0';
        wet[2] = this_file_decode.byte.b3 + '0';
        wet[3] = this_file_decode.byte.b4 + '0';
        wet[4] = this_file_decode.byte.b5 + '0';
        wet[5] = this_file_decode.byte.b6 + '0';
        wet[6] = this_file_decode.byte.b7 + '0';
        wet[7] = this_file_decode.byte.b8 + '0';
        fwrite(wet, sizeof(char ), 8, output);
    }
    char rew_wet[remainder];
    fread(rew_wet, sizeof(char ), remainder,file);
    fwrite(rew_wet, sizeof(char ), remainder, output);
    fclose(output);
    fclose(file);
    //// Конечная декордировка в file
    file = fopen(name_of_file, "wb");
    output = fopen("process_of_decoding.txt", "rb");
    fseek(output, 0, SEEK_END);
    size = ftell(output);
    fseek(output, 0, SEEK_SET);
    while (ftell(output) < size) {
        int i = -1;
        char variable[256] = {0};
        int flag = 0;
        while (flag == 0 && ftell(output) < size) {
            i++;
            fread(&variable[i], sizeof(char ), 1, output);
            for (int j = 0; j < numbers_of_unique; j++) {
                if (strcmp(nodes[j].code, variable) == 0) {
                    fwrite(&nodes[j].meaning, sizeof(char), 1, file);
                    flag = 1;
                    break;
                }
            }
        }
    }
    fclose(file);
    fclose(output);
    remove("process_of_decoding.txt");
}

