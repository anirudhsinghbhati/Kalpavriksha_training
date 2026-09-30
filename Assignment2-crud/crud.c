#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct User {
    int id;
    char name[50];
    int age;
};

// create file if it doesn't exist
void createFile() {
    FILE *fp = fopen("users.txt", "a");
    if (fp == NULL) {
        printf("Could not create file\n");
        exit(1);
    }
    fclose(fp);
}

// returns 1 if id is already in the file
int idExists(int id) {
    struct User u;
    FILE *fp = fopen("users.txt", "r");
    if (fp == NULL) return 0;

    while (fscanf(fp, " %d,%49[^,],%d", &u.id, u.name, &u.age) == 3) {
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

    printf("Enter ID: ");
    scanf("%d", &u.id);

    if (idExists(u.id)) {
        printf("ID already exists\n");
        return;
    }

    printf("Enter name: ");
    scanf(" %49[^\n]", u.name);
    printf("Enter age: ");
    scanf("%d", &u.age);

    FILE *fp = fopen("users.txt", "a");
    fprintf(fp, "%d,%s,%d\n", u.id, u.name, u.age);
    fclose(fp);

    printf("User added\n");
}

void displayUsers() {
    struct User u;
    int count = 0;
    FILE *fp = fopen("users.txt", "r");

    printf("\nID\tName\t\tAge\n");
    while (fscanf(fp, " %d,%49[^,],%d", &u.id, u.name, &u.age) == 3) {
        printf("%d\t%s\t\t%d\n", u.id, u.name, u.age);
        count++;
    }
    if (count == 0) {
        printf("No users found\n");
    }
    fclose(fp);
}

void updateUser() {
    struct User u;
    int id, found = 0;

    printf("Enter ID to update: ");
    scanf("%d", &id);

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    while (fscanf(fp, " %d,%49[^,],%d", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            printf("Enter new name: ");
            scanf(" %49[^\n]", u.name);
            printf("Enter new age: ");
            scanf("%d", &u.age);
        }
        fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found)
        printf("User updated\n");
    else
        printf("User not found\n");
}

void deleteUser() {
    struct User u;
    int id, found = 0;

    printf("Enter ID to delete: ");
    scanf("%d", &id);

    FILE *fp = fopen("users.txt", "r");
    FILE *temp = fopen("temp.txt", "w");

    while (fscanf(fp, " %d,%49[^,],%d", &u.id, u.name, &u.age) == 3) {
        if (u.id == id) {
            found = 1;
            continue;   // skip this user, so it gets deleted
        }
        fprintf(temp, "%d,%s,%d\n", u.id, u.name, u.age);
    }

    fclose(fp);
    fclose(temp);
    remove("users.txt");
    rename("temp.txt", "users.txt");

    if (found)
        printf("User deleted\n");
    else
        printf("User not found\n");
}

int main() {
    int choice;
    createFile();

    while (1) {
        printf("\n1. Add\n2. Show\n3. Update\n4. Delete\n5. Exit\n");
        printf("Enter choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input\n");
            while (getchar() != '\n');   // clear wrong input
            continue;
        }

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