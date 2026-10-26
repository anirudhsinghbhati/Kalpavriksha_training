#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>

typedef struct Employee {
    int id;
    char name[50];
    char designation[50];
    struct Employee *manager;
    struct Employee *reports;
    struct Employee *next;
} Employee;

Employee *topLevelEmployees = NULL;

int readLine(char *buffer, int bufferSize) {
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

int readNumber(const char *question, int *value) {
    char inputLine[100];
    char *parseEndPointer;

    printf("%s", question);

    if (!readLine(inputLine, sizeof(inputLine))) {
        printf("Invalid input.\n");
        return 0;
    }

    errno = 0;
    long parsedNumber = strtol(inputLine, &parseEndPointer, 10);

    if (parseEndPointer == inputLine || *parseEndPointer != '\0' || errno == ERANGE ||
        parsedNumber > INT_MAX || parsedNumber < INT_MIN) {
        printf("Please enter a valid number.\n");
        return 0;
    }

    *value = (int)parsedNumber;
    return 1;
}

int readText(const char *question, char *destination, int destinationSize) {
    char inputLine[100];
    int foundLetter = 0;

    printf("%s", question);

    if (!readLine(inputLine, sizeof(inputLine))) {
        printf("Invalid input.\n");
        return 0;
    }
    if (strlen(inputLine) >= (size_t)destinationSize) {
        printf("Please enter at most %d characters.\n", destinationSize - 1);
        return 0;
    }

    for (int characterIndex = 0; inputLine[characterIndex] != '\0'; characterIndex++) {
        if (isalpha((unsigned char)inputLine[characterIndex])) {
            foundLetter = 1;
        } else if (inputLine[characterIndex] != ' ') {
            foundLetter = 0;
            break;
        }
    }
    if (!foundLetter) {
        printf("Only alphabets and spaces are allowed.\n");
        return 0;
    }

    strcpy(destination, inputLine);
    return 1;
}

Employee *findEmployee(Employee *currentList, int employeeId) {
    if (currentList == NULL) {
        return NULL;
    }
    if (currentList->id == employeeId) {
        return currentList;
    }

    Employee *foundInReports = findEmployee(currentList->reports, employeeId);
    if (foundInReports != NULL) {
        return foundInReports;
    }
    return findEmployee(currentList->next, employeeId);
}

int countAllInList(Employee *currentList) {
    if (currentList == NULL) {
        return 0;
    }
    return 1 + countAllInList(currentList->reports) + countAllInList(currentList->next);
}

int countEmployeesUnder(Employee *employee) {
    return countAllInList(employee->reports);
}

int countDirectReports(Employee *reportList) {
    if (reportList == NULL) {
        return 0;
    }
    return 1 + countDirectReports(reportList->next);
}

void printIndent(int depth) {
    if (depth <= 0) {
        return;
    }
    printf("    ");
    printIndent(depth - 1);
}

void displayList(Employee *currentList, int depth);

void displayEmployee(Employee *employee, int depth) {
    printIndent(depth);
    printf("%d - %s - %s\n", employee->id, employee->name, employee->designation);
    displayList(employee->reports, depth + 1);
}

void displayList(Employee *currentList, int depth) {
    if (currentList == NULL) {
        return;
    }
    displayEmployee(currentList, depth);
    displayList(currentList->next, depth);
}

void appendList(Employee **listHead, Employee *listToAttach) {
    if (*listHead == NULL) {
        *listHead = listToAttach;
        return;
    }
    appendList(&(*listHead)->next, listToAttach);
}

void setManager(Employee *currentList, Employee *newManager) {
    if (currentList == NULL) {
        return;
    }
    currentList->manager = newManager;
    setManager(currentList->next, newManager);
}

void unlinkFromList(Employee **listHead, Employee *target) {
    if (*listHead == NULL) {
        return;
    }
    if (*listHead == target) {
        *listHead = target->next;
        target->next = NULL;
        return;
    }
    unlinkFromList(&(*listHead)->next, target);
}

Employee *findSiblingWithFewestReports(Employee *siblingList, Employee *excluded, Employee *bestSoFar) {
    if (siblingList == NULL) {
        return bestSoFar;
    }

    if (siblingList != excluded) {
        if (bestSoFar == NULL ||
            countDirectReports(siblingList->reports) < countDirectReports(bestSoFar->reports)) {
            bestSoFar = siblingList;
        }
    }
    return findSiblingWithFewestReports(siblingList->next, excluded, bestSoFar);
}

void freeList(Employee *currentList) {
    if (currentList == NULL) {
        return;
    }
    freeList(currentList->reports);
    freeList(currentList->next);
    free(currentList);
}

void addEmployee(void) {
    int employeeId, managerId;
    char name[50], designation[50];

    if (!readNumber("Enter Employee ID: ", &employeeId)) {
        return;
    }
    if (employeeId <= 0) {
        printf("The ID must be a positive number.\n");
        return;
    }
    if (findEmployee(topLevelEmployees, employeeId) != NULL) {
        printf("An employee with this ID already exists.\n");
        return;
    }

    if (!readText("Enter Name: ", name, sizeof(name))) {
        return;
    }
    if (!readText("Enter Designation: ", designation, sizeof(designation))) {
        return;
    }
    if (!readNumber("Enter Reporting Manager ID (0 for none): ", &managerId)) {
        return;
    }

    Employee *manager = NULL;

    if (managerId < 0) {
        printf("A manager ID cannot be negative.\n");
        return;
    }

    if (managerId == 0) {
        if (topLevelEmployees != NULL) {
            printf("There is already a CEO, so this employee needs a reporting manager.\n");
            return;
        }
    } else {
        if (managerId == employeeId) {
            printf("An employee cannot report to themselves.\n");
            return;
        }
        manager = findEmployee(topLevelEmployees, managerId);
        if (manager == NULL) {
            printf("No manager found with that ID.\n");
            return;
        }
    }

    Employee *newEmployee = (Employee *)calloc(1, sizeof(Employee));
    if (newEmployee == NULL) {
        printf("Not enough memory to add the employee.\n");
        return;
    }

    newEmployee->id = employeeId;
    strcpy(newEmployee->name, name);
    strcpy(newEmployee->designation, designation);
    newEmployee->manager = manager;

    if (manager != NULL) {
        appendList(&manager->reports, newEmployee);
    } else {
        appendList(&topLevelEmployees, newEmployee);
    }
    printf("Employee added successfully.\n");
}

void removeEmployee(void) {
    int employeeId;

    if (!readNumber("Enter Employee ID to remove: ", &employeeId)) {
        return;
    }

    Employee *employeeToRemove = findEmployee(topLevelEmployees, employeeId);
    if (employeeToRemove == NULL) {
        printf("No employee found with that ID.\n");
        return;
    }
    if (employeeToRemove->manager == NULL) {
        printf("The CEO cannot be removed.\n");
        return;
    }

    Employee *manager = employeeToRemove->manager;
    Employee *reportsToReassign = employeeToRemove->reports;

    unlinkFromList(&manager->reports, employeeToRemove);
    employeeToRemove->reports = NULL;

    if (reportsToReassign != NULL) {
        Employee *newBoss = findSiblingWithFewestReports(manager->reports, NULL, NULL);
        if (newBoss == NULL) {
            newBoss = manager;
        }

        setManager(reportsToReassign, newBoss);
        appendList(&newBoss->reports, reportsToReassign);
        printf("Reports of %d have been moved to %d.\n", employeeToRemove->id, newBoss->id);
    }

    free(employeeToRemove);
    printf("Employee removed successfully.\n");
}

void displayEmployeeHierarchy(void) {
    int employeeId;

    if (!readNumber("Enter Employee ID: ", &employeeId)) {
        return;
    }

    Employee *employee = findEmployee(topLevelEmployees, employeeId);
    if (employee == NULL) {
        printf("No employee found with that ID.\n");
        return;
    }

    printf("\n");
    displayEmployee(employee, 0);
    printf("Total Employees Under %d: %d\n", employeeId, countEmployeesUnder(employee));
}

void displayCompleteHierarchy(void) {
    printf("\n===== ORGANIZATION HIERARCHY =====\n");

    if (topLevelEmployees == NULL) {
        printf("There are no employees yet.\n");
        return;
    }

    displayList(topLevelEmployees, 0);
    printf("Total Employees: %d\n", countAllInList(topLevelEmployees));
}

int main(void) {
    int menuChoice = 0;

    do {
        printf("\n===== EMPLOYEE HIERARCHY MANAGEMENT =====\n");
        printf("1. Add Employee\n");
        printf("2. Remove Employee\n");
        printf("3. Display Employee Hierarchy\n");
        printf("4. Display Complete Hierarchy\n");
        printf("5. Exit\n");

        if (!readNumber("Enter your choice: ", &menuChoice)) {
            menuChoice = feof(stdin) ? 5 : 0;
        }

        switch (menuChoice) {
            case 1: addEmployee(); break;
            case 2: removeEmployee(); break;
            case 3: displayEmployeeHierarchy(); break;
            case 4: displayCompleteHierarchy(); break;
            case 5: break;
            default: printf("Please choose a number from 1 to 5.\n");
        }
    } while (menuChoice != 5);

    freeList(topLevelEmployees);
    return 0;
}