// Pair with a given sum in a sorted array

#include <stdio.h>

#define MAX_SIZE 100

int main() {
    int arr[MAX_SIZE];
    int size;
    int target;
    int *left;
    int *right;

    printf("Enter array size: ");
    if (scanf("%d", &size) != 1 || size < 2 || size > MAX_SIZE) {
        printf("Invalid size.\n");
        return 1;
    }
    printf("Enter %d sorted numbers: ", size);
    for (int index = 0; index < size; index++) {
        scanf("%d", &arr[index]);
    }
    printf("Enter target sum: ");
    if (scanf("%d", &target) != 1) {
        printf("Invalid target.\n");
        return 1;
    }

    left = arr;
    right = arr + size - 1;
    while (left < right) {
        int sum = *left + *right;

        if (sum == target) {
            printf("Pair found: %d + %d (indices %ld and %ld)\n",
                   *left, *right, (long)(left - arr), (long)(right - arr));
            return 0;
        }
        if (sum < target) {
            left++;
        } else {
            right--;
        }
    }
    printf("No pair found.\n");
    return 0;
}