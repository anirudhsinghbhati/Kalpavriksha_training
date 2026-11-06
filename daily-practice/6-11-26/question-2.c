// Run-length encoding of a string

#include <stdio.h>
#include <string.h>

#define MAX_LEN 100

int main() {
    char text[MAX_LEN];
    char encoded[MAX_LEN * 2 + 1];
    int written = 0;
    char *ptr;

    printf("Enter a string: ");
    if (fgets(text, sizeof text, stdin) == NULL) {
        return 1;
    }
    text[strcspn(text, "\n")] = '\0';

    ptr = text;
    while (*ptr != '\0') {
        char current = *ptr;
        int run = 0;

        while (*ptr == current) {
            run++;
            ptr++;
        }
        written += sprintf(encoded + written, "%c%d", current, run);
    }
    encoded[written] = '\0';

    printf("Encoded: %s\n", encoded);
    return 0;
}