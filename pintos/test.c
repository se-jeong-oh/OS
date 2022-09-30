#include <stdio.h>
#include <string.h>

int main() {
    char *save_ptr;
    char *parse_ptr;
    int i= 0;
    char file_name[256] = "a      b   c";
    parse_ptr = strtok_r(file_name, " ", &save_ptr);
    while(parse_ptr != NULL) {
        printf("%d : %s\n", i, parse_ptr);
        parse_ptr = strtok_r(NULL, " ", &save_ptr);
        i++;
    }
    return 0;
}