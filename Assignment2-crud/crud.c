#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define FILE_NAME "users.txt"

struct User {
    int id;
    char name[50];
    int age;
};

void clearBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

FILE* openFile(const char *filename, const char *mode) {
    FILE *fp = fopen(filename, mode);
    if (fp == NULL) {
        printf("Could not open file: %s\n", filename);
    }
    return fp;
}

int readUser(FILE *fp, struct User *u) {
    return fscanf(fp, " %d,%49[^,],%d", &u->id, u->name, &u->age) == 3;
}

// Input validation
int isNumber(const char *str) {
    if (str == NULL || *str == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    return 1;
}

int isAlphaOnly(const char *str) {
    if (str == NULL || *str == '\0') return 0;
    int hasAlpha = 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (isalpha((unsigned char)str[i])) {
            hasAlpha = 1;
        } else if (str[i] != ' ') {
            return 0;
        }
    }
    return hasAlpha;
}

int getValidId(const char *prompt) {
    char input[100];
    int id;
    while (1) {
        printf("%s", prompt);
        if (scanf("%99s", input) != 1) {
            clearBuffer();
            continue;
        }
        clearBuffer();
        if (!isNumber(input)) {
            printf("Invalid ID! ID must contain numbers only.\n");
            continue;
        }
        id = atoi(input);
        if (id <= 0) {
            printf("Invalid ID! ID must be greater than 0.\n");
            continue;
        }
        return id;
    }
}

void getValidName(char *name, const char *prompt) {
    while (1) {
        printf("%s", prompt);
        if (scanf(" %49[^\n]", name) != 1) {
            clearBuffer();
            continue;
        }
        clearBuffer();
        if (!isAlphaOnly(name)) {
            printf("Invalid Name! Name must contain alphabets only.\n");
            continue;
        }
        return;
    }
}

int getValidAge(const char *prompt) {
    char input[100];
    int age;
    while (1) {
        printf("%s", prompt);
        if (scanf("%99s", input) != 1) {
            clearBuffer();
            continue;
        }
        clearBuffer();
        if (!isNumber(input)) {
            printf("Invalid Age! Age must contain numbers only.\n");
            continue;
        }
        age = atoi(input);
        if (age <= 0 || age > 120) {
            printf("Invalid Age! Age must be between 1 and 120.\n");
            continue;
        }
        return age;
    }
}

void createFile() {
    FILE *fp = openFile(FILE_NAME, "a");
    if (fp == NULL) {
        exit(1);
    }
    fclose(fp);
}

int idExists(int id) {
    struct User u;
    FILE *fp = openFile(FILE_NAME, "r");
    if (fp == NULL) return 0;

    while (readUser(fp, &u)) {
        if (u.id == id) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
}

void addUser() {
    struct User u;

    u.id = getValidId("Enter ID: ");

    if (idExists(u.id)) {
        printf("ID already exists\n");
        return;
    }

    getValidName(u.name, "Enter name: ");
    u.age = getValidAge("Enter age: ");

    FILE *fp = openFile(FILE_NAME, "a");
    if (fp == NULL) return;

    fprintf(fp, "%d,%s,%d\n", u.id, u.name, u.age);
    fclose(fp);

    printf("User added\n");
}

void displayUsers() {
    struct User u;
    int count = 0;
    FILE *fp = openFile(FILE_NAME, "r");
    if (fp == NULL) return;

    printf("\nID\tName\t\tAge\n");
    while (readUser(fp, &u)) {
        printf("%d\t%s\t\t%d\n", u.id, u.name, u.age);
        count++;
    }
    if (count == 0) {
        printf("No users found\n");
    }
    fclose(fp);
}

void modifyUserRecord(int isDelete) {
    struct User u;
    int id, found = 0;

    id = getValidId(isDelete ? "Enter ID to delete: " : "Enter ID to update: ");

    FILE *fp = openFile(FILE_NAME, "r");
    FILE *temp = openFile("temp.txt", "w");
    if (fp == NULL || temp == NULL) {
        if (fp) fclose(fp);
        if (temp) fclose(temp);
        return;
    }

    while (readUser(fp, &u)) {
        if (u.id == id) {
            found = 1;
            if (isDelete) {
                continue;
            } else {
                getValidName(u.name, "Enter new name: ");
                u.age = getValidAge("Enter new age: ");
            }
        }
        fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove(FILE_NAME);
    rename("temp.txt", FILE_NAME);

    if (found) {
        printf(isDelete ? "User deleted\n" : "User updated\n");
    } else {
        printf("User not found\n");
    }
}

void updateUser() {
    modifyUserRecord(0);
}

void deleteUser() {
    modifyUserRecord(1);
}

int main() {
    int choice;
    createFile();

    while (1) {
        printf("\n1. Add\n2. Show\n3. Update\n4. Delete\n5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input\n");
            clearBuffer();
            continue;
        }
        clearBuffer();

        switch (choice) {
            case 1: addUser(); break;
            case 2: displayUsers(); break;
            case 3: updateUser(); break;
            case 4: deleteUser(); break;
            case 5: exit(0);
            default: printf("Invalid choice\n");
        }
    }
    return 0;
}