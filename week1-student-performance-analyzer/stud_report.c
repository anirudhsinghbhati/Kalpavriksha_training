#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

#define MAX_STUDENTS 100
#define NUM_SUBJECTS 3

typedef struct {
    int rollNumber;
    char name[50];
    int marks[NUM_SUBJECTS];
} Student;

int totalStudents;

bool isValidName(const char *name) {
    if (name[0] == '\0') return false;
    for (int charIndex = 0; name[charIndex] != '\0'; charIndex++) {
        if (!isalpha((unsigned char)name[charIndex])) {
            return false;
        }
    }
    return true;
}

int calculateTotal(Student student) {
    int total = 0;
    for (int subjectIndex = 0; subjectIndex < NUM_SUBJECTS; subjectIndex++) {
        total += student.marks[subjectIndex];
    }
    return total;
}

float calculateAverage(int total) {
    return (float)total / NUM_SUBJECTS;
}

char calculateGrade(float average) {
    if (average >= 85)
        return 'A';
    else if (average >= 70)
        return 'B';
    else if (average >= 50)
        return 'C';
    else if (average >= 35)
        return 'D';
    else
        return 'F';
}

int getStarCount(char grade) {
    switch (grade) {
        case 'A':
            return 5;
        case 'B':
            return 4;
        case 'C':
            return 3;
        case 'D':
            return 2;
        default:
            return 0;
    }
}

void printPerformance(char grade) {
    int stars = getStarCount(grade);

    for (int starIndex = 0; starIndex < stars; starIndex++) {
        printf("*");
    }

    printf("\n");
}

void printRollNumbers(Student students[], int index) {
    if (index >= totalStudents)
        return;

    printf("%d", students[index].rollNumber);

    if (index < totalStudents - 1)
        printf(" ");

    printRollNumbers(students, index + 1);
}

int main() {
    Student students[MAX_STUDENTS];

    int studentCount;

    printf("Enter number of students: ");
    if (scanf("%d", &studentCount) != 1 || studentCount <= 0 || studentCount > MAX_STUDENTS) {
        printf("Invalid! Number of Students must be between 1 and %d.\n", MAX_STUDENTS);
        return 1;
    }

    totalStudents = studentCount;
    printf("Enter Student details\nROLL_NUM NAME MARKS1 MARKS2 MARKS3\n");
    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        if (scanf("%d %49s", &students[studentIndex].rollNumber, students[studentIndex].name) != 2) {
            printf("Invalid input format for student %d!\n", studentIndex + 1);
            return 1;
        }

        if (students[studentIndex].rollNumber < 0) {
            printf("Invalid roll number\nt");
            return 1;
        }

        if (!isValidName(students[studentIndex].name)) {
            printf("Invalid! Name must contain only alphabets\n");
            return 1;
        }

        for (int subjectIndex = 0; subjectIndex < NUM_SUBJECTS; subjectIndex++) {
            if (scanf("%d", &students[studentIndex].marks[subjectIndex]) != 1) {
                printf("Invalid marks input");
                return 1;
            }

            if (students[studentIndex].marks[subjectIndex] < 0 || students[studentIndex].marks[subjectIndex] > 100) {
                printf("Invalid marks!");
                return 1;
            }
        }
    }
    printf("\n");

    for (int studentIndex = 0; studentIndex < studentCount; studentIndex++) {
        int total = calculateTotal(students[studentIndex]);
        float average = calculateAverage(total);
        char grade = calculateGrade(average);

        printf("Roll: %d\n", students[studentIndex].rollNumber);
        printf("Name: %s\n", students[studentIndex].name);
        printf("Total: %d\n", total);
        printf("Average: %.2f\n", average);
        printf("Grade: %c\n", grade);

        if (average < 35) {
            continue;
        }

        printf("Performance: ");
        printPerformance(grade);
        printf("\n");
    }

    printf("List of Roll Numbers: ");
    printRollNumbers(students, 0);
    printf("\n");

    return 0;
}