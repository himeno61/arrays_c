#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IFILE "tekst.html"
#define OFILE "return.html"

int fileCharCount(FILE *ft)
{
    int count = 0;
    while( fgetc(ft) != EOF)
        count++;
    rewind(ft);
    return count;
}

int rowCount(const char* string)
{
    int count = 0;
    char *where = string;
    while ((where = strstr(where, "<tr>")) != NULL) {
        count++;
        where += 4;
    }
    return count;
}

int colCount(const char* string)
{
    int count = 0;
    char *where = string;
    while ((where = strstr(where, "<td>")) != NULL) {
        count++;
        where += 4;
    }
    return count/rowCount(string);
}

char* readCell(const char* string, char** ret){
    char* begin = strstr(string, "<td>") + strlen("<td>");
    char* end = strstr(begin, "</td>");
    char* buf = calloc(end - begin + 1, sizeof(char));
    strncpy(buf, begin, end - begin);
    buf[end - begin] = '\0';
    *ret = end + strlen("</td>");
    return buf;
}

typedef struct Table {
    char*** cell;
    int x, y;
} Table;

void importTableData(Table* table, char* string, int x, int y){
    char* str = string;
    table->x = x;
    table->y = y;
    table->cell = calloc(y, sizeof(char**));
    int i, j;
    for (i = 0; i < y; i++){
        table->cell[i] = calloc(x, sizeof(char*));
    }
    for (i = 0; i < y; i++){
        for (j = 0; j < x; j++){
            table->cell[i][j] = readCell(str, &str);
        }
    }
}

void addRows(Table* table, int n){
    table->y += n;
    char*** temp;
    temp = realloc(table->cell, table->y * sizeof(char**));
    if (temp == NULL) {
        printf("Nie mozna realokowac pamieci!\n");
        return;
    }
    table->cell = temp;
    if (n > 0) {
        int i;
        for (i = 0; i < n; i++){
            table->cell[table->y - i - 1] = calloc(table->x, sizeof(char*));
            int j;
            for (j = 0; j < table->x; j++){
                table->cell[table->y - i - 1][j] = "";
            }
        }
    }
}

void addColumns(Table* table, int n){
    table->x += n;
    char** temp;
    int i;
    for (i = 0; i < table->y; i++){
        temp = realloc(table->cell[i], table->x * sizeof(char*));
        if (temp == NULL) {
            printf("Nie mozna realokowac pamieci!\n");
            return;
        }
        table->cell[i] = temp;
        if (n > 0){
            int j;
            for (j = 0; j < n; j++){
                table->cell[i][table->x - j - 1] = "";
            }
        }
    }
}

void fillCell(Table* table, int y, int x, const char* content){
    if (x < 0 || y < 0 || x > table->x || y > table->y){
        printf("Nie ma takiego miejsca w tabeli!\n");
        return;
    }
    int wordLength = strlen(content);
    table->cell[y-1][x-1] = calloc(wordLength + 1, sizeof(char));
    strcpy(table->cell[y-1][x-1], content);
    table->cell[y-1][x-1][wordLength] = '\0';
}

void printTableContents(const Table* table){
    int x = table->x, y = table->y;
    int i, j;
    for (i = 0; i < y; i++){
        for (j = 0; j < x; j++){
            printf("%s - ", table->cell[i][j]);
        }
        printf("\n");
    }
}

void generateDocument(Table* table, FILE* stream){
    int x = table->x, y = table->y;
    int i, j;
    fputs("<html>\n<body>\n\t<table border=\"1px\">\n", stream);
    for (i = 0; i < y; i++){
        fputs("\t\t<tr>\n", stream);
        for (j = 0; j < x; j++){
            fputs("\t\t\t<td width=\"100px\">", stream);
            fputs(table->cell[i][j], stream);
            fputs("</td>\n", stream);
        }
        fputs("\t\t</tr>\n", stream);
    }
    fputs("\t<table>\n<body>\n<html>\n", stream);
}

int main()
{
    char *fString;
    int i = 0, j;
    int charCount;
    FILE *file;
    if ((file = fopen(IFILE, "r")) != NULL) {
        charCount = fileCharCount(file);
        fString = (char*)malloc(charCount * sizeof(char) + 1);
        while(fscanf(file, "%c", &fString[i++]) != EOF);
        fString[charCount] = '\0';
        fclose(file);
    }
    else {
        printf("Nie mozna otworzyc pliku wejsciowego!\n");
        return 0;
    }

    Table* table = malloc(sizeof(Table));
    importTableData(table, fString, colCount(fString), rowCount(fString));

    //menu
    int number = 0;
    int select=1;
    char buf[50];
    system("clear");
    while (select != 0) {
        printf("\nPodaj numer operacji ktora chcesz wykonac: \a");
        printf("\n===========================================");
        printf("\n[1] Dodaj wiersz ");
        printf("\n[2] Dodaj komune ");
        printf("\n[3] Wyswietl tabele ");
        printf("\n[0] Wyjscie \n");
        scanf("%d",&select);
        switch(select){
            case 1:
                system("clear");
                printf("\n===========================================\n\a  \q");
                printf("\n Podaj ile wierszy chcesz dodac: ");
                scanf("%d",&number);
                getc(stdin);
                if (number != 0){
                    addRows(table,number);
                    printf("\n\t!Dane beda dodawane wierszami!\n");
                    for(i = (table->y) - number +1; i<=table->y; i++){
                        for(j = 1; j <= table->x; j++){
                            printf("Podaj wartosc do pola [%2d][%2d]: ",i,j);
                            gets(buf);
                            fillCell(table,i,j,buf);
                        }
                    }
                }
                else system("clear");
                fflush(stdin);
                break;
            case 2:
                system("clear");
                printf("\n===========================================\n");
                printf("\n Podaj ile kolumn chcesz dodac : ");
                scanf("%d",&number);
                getc(stdin);
                if (number != 0){
                    addColumns(table,number);
                    printf("\n\t!Dane beda dodawane wierszami!\n");
                    for(i = 1; i<=table->y; i++){
                        for(j = table->x - number +1; j <= table->x; j++){
                            printf("Podaj wartosc do pola [%2d][%2d]: ",i,j);
                            gets(buf);
                            fillCell(table,i,j,buf);
                        }
                    }
                }
                else system("clear");
                fflush(stdin);
                break;
            case 3:
                system("clear");
                printf("\n===========================================\n");
                printf("Zawartosc tabeli:");
                printf("\n-------------------------------------------\n");
                printTableContents(table);
                printf("===========================================\n");
                fflush(stdin);
                break;
            case 0:
                break;
            default:
                system("clear");
                printf("===========================================\n");
                printf("\t!!!Liczba %d nie obslugiwana!!!\n",select);
                printf("===========================================");
                fflush(stdin);
                break;

        }
    }
    system("clear");
    printTableContents(table);

    if ((file = fopen(OFILE, "w+")) == NULL){
        printf("Nie mozna utworzyc pliku wyjsciowego!\n");
        return 0;
    }

    generateDocument(table, file);
    fclose(file);

    //Cleaning
    free(fString);
    return 0;
}
