// Merge two sorted arrays without duplicates

#include <stdio.h>

#define MAX_SIZE 50

int merge_unique(const int *first, int first_size,
                 const int *second, int second_size, int *result) {
    int i = 0;
    int j = 0;
    int k = 0;

    while (i < first_size || j < second_size) {
        int value;

        if (j >= second_size || (i < first_size && first[i] <= second[j])) {
            value = first[i];
            i++;
        } else {
            value = second[j];
            j++;
        }
        if (k == 0 || result[k - 1] != value) {
            result[k] = value;
            k++;
        }
    }
    return k;
}

int main() {
    int first[MAX_SIZE];
    int second[MAX_SIZE];
    int result[MAX_SIZE * 2];
    int first_size;
    int second_size;
    int result_size;

    printf("Size of first array: ");
    if (scanf("%d", &first_size) != 1 || first_size < 1 || first_size > MAX_SIZE) {
        printf("Invalid size.\n");
        return 1;
    }
    printf("Enter %d sorted numbers: ", first_size);
    for (int index = 0; index < first_size; index++) {
        scanf("%d", &first[index]);
    }

    printf("Size of second array: ");
    if (scanf("%d", &second_size) != 1 || second_size < 1 || second_size > MAX_SIZE) {
        printf("Invalid size.\n");
        return 1;
    }
    printf("Enter %d sorted numbers: ", second_size);
    for (int index = 0; index < second_size; index++) {
        scanf("%d", &second[index]);
    }

    result_size = merge_unique(first, first_size, second, second_size, result);

    for (int index = 0; index < result_size; index++) {
        printf("%d ", result[index]);
    }
    printf("\n");
    return 0;
}