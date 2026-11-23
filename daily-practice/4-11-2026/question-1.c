// Reverse the words of a sentence

#include <stdio.h>
#include <string.h>

#define MAX_WORDS 50
#define MAX_LEN 200

int main() {
    char sentence[MAX_LEN];
    char *words[MAX_WORDS];
    int count = 0;
    char *token;

    printf("Enter a sentence: ");
    if (fgets(sentence, sizeof sentence, stdin) == NULL) {
        return 1;
    }
    sentence[strcspn(sentence, "\n")] = '\0';

    token = strtok(sentence, " ");
    while (token != NULL && count < MAX_WORDS) {
        words[count] = token;
        count++;
        token = strtok(NULL, " ");
    }

    for (int index = count - 1; index >= 0; index--) {
        printf("%s ", words[index]);
    }
    printf("\n");
    return 0;
}