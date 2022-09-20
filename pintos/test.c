#include <stdio.h>
#include <string.h>

int main() {
    char txt[100] = "aaa bbb ccc";
    char parsed[3][100];
    char *ptr;
    int idx = 0;

    ptr = strtok(txt, " ");
    while(ptr != NULL) {
        //printf("%s\n", ptr);
        strcpy(parsed[idx], ptr);
        ptr = strtok(NULL, " ");
        idx++;
    }
    for (int i = 0; i< idx; i++) {
        printf("%s\n", parsed[i]);
    }
    return 0;
}