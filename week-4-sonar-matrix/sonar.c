#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define CELL(row, col) (matrix + (row) * size + (col)) 

static void display(int **matrix, int size){
    int **cursor = matrix;
    int **end = matrix + size * size;
    while(cursor < end){
        printf("%d ", **cursor);
        cursor++;
        if((cursor - matrix) % size == 0)printf("\n");
    }
}

static void rotate(int **matrix, int size){
    for (int layer = 0; layer < size / 2; layer++) {
        for (int offset = layer; offset < size - 1 - layer; offset++) {
            int **top    = CELL(layer, offset);
            int **right  = CELL(offset, size - 1 - layer);
            int **bottom = CELL(size - 1 - layer, size - 1 - offset);
            int **left   = CELL(size - 1 - offset, layer);
            int *temp = *left;
            *left   = *bottom;
            *bottom = *right;
            *right  = *top;
            *top    = temp;
        }
    }
}

static int smooth(int **matrix, int size){
    int *previousRow = malloc(sizeof(int) * size);
    int *currentRow  = malloc(sizeof(int) * size);
    if(!previousRow || !currentRow){
        printf("failed to allocate memory\n");
        free(previousRow);
        free(currentRow);
        return 1;
    }
    for(int row = 0; row < size; row++){
        for(int *dest = currentRow, **source = CELL(row, 0); dest < currentRow + size; dest++, source++)
            *dest = **source;
        for(int col = 0; col < size; col++){
            int sum = 0, count = 0;
            for(int rowShift = -1; rowShift <= 1; rowShift++){
                int neighborRow = row + rowShift;
                if(neighborRow < 0 || neighborRow >= size)continue;
                for(int colShift = -1; colShift <= 1; colShift++){
                    int neighborCol = col + colShift;
                    if(neighborCol < 0 || neighborCol >= size)continue;
                    if(rowShift == -1)sum += *(previousRow + neighborCol);
                    else if(rowShift == 0)sum += *(currentRow + neighborCol);
                    else sum += **CELL(neighborRow, neighborCol);
                    count++;
                }
            }
            **CELL(row, col) = sum / count;
        }
        for(int *dest = previousRow, *source = currentRow; dest < previousRow + size; dest++, source++)
            *dest = *source;
    }
    free(previousRow);
    free(currentRow);
    return 0;
}

int main(){
    int size;
    printf("Enter matrix size (2-10): ");
    if(scanf("%d", &size) != 1 || size < 2 || size > 10){
        printf("Invalid\n");
        return 1;
    }

    srand((unsigned)time(NULL));

    int **matrix = malloc(sizeof(int *) * size * size);
    if(!matrix){
        printf("failed to allocate memory\n");
        return 1;
    }
    for(int **cursor = matrix; cursor < matrix + size * size; cursor++){
        *cursor = malloc(sizeof(int));
        if(!*cursor){
            printf("failed to allocate memory\n");
            for(int **done = matrix; done < cursor; done++) free(*done);
            free(matrix);
            return 1;
        }
        **cursor = rand() % 256;
    }
    printf("Original randomly generated matrix:\n");
    display(matrix, size);

    rotate(matrix, size);
    printf("Matrix after 90 degree clockwise rotation:\n");
    display(matrix, size);

    if(smooth(matrix, size)){
        for(int **cursor = matrix; cursor < matrix + size * size; cursor++)free(*cursor);
        free(matrix);
        return 1;
    }
    printf("Matrix after applying 3 x 3 smoothing filter:\n");
    display(matrix, size);

    for(int **cursor = matrix; cursor < matrix + size * size; cursor++) free(*cursor);
    free(matrix);
    return 0;
}