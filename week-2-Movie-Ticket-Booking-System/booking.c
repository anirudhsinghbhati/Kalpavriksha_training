#include <stdio.h>
#include <ctype.h>

#define DISCOUNT_PERCENTAGE 10.0
#define DISCOUNT_MIN_TICKETS 5
#define MAX_TICKETS 20

const int STANDARD_PRICE = 150;
const int PREMIUM_PRICE = 250;
const int RECLINER_PRICE = 400;

static int is_alpha_only(const char *s) {
    if (*s == '\0') return 0;          
    for (; *s != '\0'; s++) {
        if (!isalpha((unsigned char)*s)) {
            return 0;
        }
    }
    return 1;
}

int main() {
    float subtotal = 0, discount = 0, total = 0;
    int tickets = 0, choice = 0;
    char name[20];
    const char *cat = "";

    printf("Enter name: ");
    if (scanf("%19s", name) != 1 || !is_alpha_only(name)) {
        fprintf(stderr, "Invalid name!\n");
        return 1;
    }

    printf("Number of tickets: ");
    if (scanf("%d", &tickets) != 1 || tickets < 1 || tickets > MAX_TICKETS) {
        printf("Invalid ticket count!\n");
        return 1;
    }

    printf("Ticket category choice\n1. Standard\n2. Premium\n3. Recliner\n");
    scanf("%d", &choice);

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