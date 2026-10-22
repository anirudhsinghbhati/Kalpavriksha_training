#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

#define DISCOUNT_PERCENTAGE 10.0
#define DISCOUNT_MIN_TICKETS 5
#define MAX_TICKETS 20

const int STANDARD_PRICE = 150;
const int PREMIUM_PRICE = 250;
const int RECLINER_PRICE = 400;

static int is_alpha_only(const char *text) {
    if (*text == '\0') return 0;
    for (; *text != '\0'; text++) {
        if (!isalpha((unsigned char)*text)) {
            return 0;
        }
    }
    return 1;
}

static int read_line(char *buffer, int bufferSize) {
    if (fgets(buffer, bufferSize, stdin) == NULL) {
        return 0;
    }

    size_t lineLength = strlen(buffer);

    if (lineLength > 0 && buffer[lineLength - 1] == '\n') {
        buffer[lineLength - 1] = '\0';
        return 1;
    }

    if (feof(stdin)) {
        return 1;
    }

    int characterRead;
    while ((characterRead = getchar()) != '\n' && characterRead != EOF) {
    }
    return 0;
}

static int read_number(int *value) {
    char inputLine[100];
    char *parseEndPointer;

    if (!read_line(inputLine, sizeof(inputLine))) {
        return 0;
    }

    errno = 0;
    long parsedNumber = strtol(inputLine, &parseEndPointer, 10);

    if (parseEndPointer == inputLine || *parseEndPointer != '\0' || errno == ERANGE ||
        parsedNumber > INT_MAX || parsedNumber < INT_MIN) {
        return 0;
    }

    *value = (int)parsedNumber;
    return 1;
}

int main() {
    float subtotal = 0, discount = 0, total = 0;
    int tickets = 0, choice = 0;
    char name[20];
    char nameLine[100];
    const char *cat = "";

    printf("Enter name: ");
    if (!read_line(nameLine, sizeof(nameLine)) || strlen(nameLine) >= sizeof(name) ||
        !is_alpha_only(nameLine)) {
        fprintf(stderr, "Invalid name!\n");
        return 1;
    }
    strcpy(name, nameLine);

    printf("Number of tickets: ");
    if (!read_number(&tickets) || tickets < 1 || tickets > MAX_TICKETS) {
        printf("Invalid ticket count!\n");
        return 1;
    }

    printf("Ticket category choice\n1. Standard\n2. Premium\n3. Recliner\n");
    if (!read_number(&choice)) {
        printf("Invalid category choice!\n");
        return 1;
    }

    switch (choice) {
        case 1:
            subtotal = tickets * STANDARD_PRICE;
            cat = "Standard";
            break;
        case 2:
            subtotal = tickets * PREMIUM_PRICE;
            cat = "Premium";
            break;
        case 3:
            subtotal = tickets * RECLINER_PRICE;
            cat = "Recliner";
            break;
        default:
            printf("Invalid category choice!\n");
            return 1;
    }

    if (tickets >= DISCOUNT_MIN_TICKETS) {
        discount = subtotal * DISCOUNT_PERCENTAGE / 100.0f;
    }
    total = subtotal - discount;

    printf("Customer: %s\n", name);
    printf("Ticket Category: %s\n", cat);
    printf("Tickets: %d\n", tickets);
    printf("Subtotal: %.2f\n", subtotal);
    printf("Discount: %.2f\n", discount);
    printf("Final Bill: %.2f\n", total);

    return 0;
}