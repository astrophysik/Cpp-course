#ifndef LAB_PART_SIX_HAFFMAN_H
#define LAB_PART_SIX_HAFFMAN_H
union code {
    char str;
    struct byte {
        unsigned b1 : 1;
        unsigned b2 : 1;
        unsigned b3 : 1;
        unsigned b4 : 1;
        unsigned b5 : 1;
        unsigned b6 : 1;
        unsigned b7 : 1;
        unsigned b8 : 1;
    }byte;
}code;

typedef struct node {
    char meaning;
    char code[256];
    float prediction;
    struct node * left;
    struct node * right;
}node;

void copyfile(FILE* input, FILE* output);

node * creation_of_tree(node * nodes[], int len);

void counting_codes(node * root);

void long_sort(node * nodes, int len);

void creation_of_array(float * predictions, char *symbols, int len, struct node *nodes);

void encoding(char* name_of_file);

void decoding(char * name_of_file);

#endif //LAB_PART_SIX_HAFFMAN_H
