// Rotate an array to the right by k positions

#include <stdio.h>

#define MAX_SIZE 100

void reverse(int *start, int *end) {
    while (start < end) {
        int temp = *start;
        *start = *end;
        *end = temp;
        start++;
        end--;
    }
}

int main(){
    int arr[MAX_SIZE];
    int size;
    int shift;

    printf("Enter array size: ");
    if (scanf("%d", &size) != 1 || size < 1 || size > MAX_SIZE) {
        printf("Invalid size.\n");
        return 1;
    }
    printf("Enter %d numbers: ", size);
    for (int index = 0; index < size; index++) {
        scanf("%d", &arr[index]);
    }
    printf("Rotate right by: ");
    if (scanf("%d", &shift) != 1 || shift < 0) {
        printf("Invalid shift.\n");
        return 1;
    }

    shift = shift % size;
    if (shift > 0) {
        reverse(arr, arr + size - 1);
        reverse(arr, arr + shift - 1);
        reverse(arr + shift, arr + size - 1);
    }

    for (int index = 0; index < size; index++) {
        printf("%d ", arr[index]);
    }
    printf("\n");
    return 0;
}