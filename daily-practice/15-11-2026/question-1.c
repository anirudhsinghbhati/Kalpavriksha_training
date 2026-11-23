// First non-repeating character in a string

#include <stdio.h>
#include <string.h>

#define MAX_LEN 200

int main() {
    char text[MAX_LEN];
    int frequency[256] = {0};
    char *ptr;

    printf("Enter a string: ");
    if (fgets(text, sizeof text, stdin) == NULL) {
        return 1;
    }
    text[strcspn(text, "\n")] = '\0';

    for (ptr = text; *ptr != '\0'; ptr++) {
        frequency[(unsigned char)*ptr]++;
    }

    for (ptr = text; *ptr != '\0'; ptr++) {
        if (frequency[(unsigned char)*ptr] == 1) {
            printf("First non-repeating character: '%c' at index %ld\n", *ptr, (long)(ptr - text));
            return 0;
        }
    }
    printf("No non-repeating character.\n");
    return 0;
}