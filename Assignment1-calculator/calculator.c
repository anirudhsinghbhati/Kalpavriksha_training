#include<stdio.h>
#include<ctype.h>
#define MAX 1000

char expr[MAX];
int idx = 0;
int error = 0;
int division_by_0 = 0;

// Read a number
int getNumber(){
    
    while(isspace(expr[idx])) idx++;

    //to handles negative numbers
    int sign = 1;
    if(expr[idx] == '-'){
        sign = -1;
        idx++;
    }

    if(!isdigit(expr[idx])){
        error = 1;
        return 0;
    }

    int num = 0;
    while(isdigit(expr[idx])){
        num = num * 10 + (expr[idx] - '0');
        idx++;
    }

    return sign * num;
}

//Handle multiplication and divistion
int mul_div(){
    int result = getNumber();

    while(!error){
        while(isspace(expr[idx])) idx++;

        if(expr[idx] == '*'){
            idx++;
            result *= getNumber();
        }
        else if(expr[idx] == '/'){
            idx++;
            int val = getNumber();

            if(error)return 0;

            if(val == 0){
                division_by_0 = 1;
                return 0;
            }
            result /= val;
        }
        else 
            break;
    }
    return result;
}

// Handles addition and subtraction
int add_sub(){

    int result = mul_div();
    
    while(!error && !division_by_0){
        while(isspace(expr[idx])) idx++;

        if(expr[idx] == '+'){
            idx++;
            result += mul_div();
        }
        else if(expr[idx] == '-'){
            idx++;
            result -= mul_div();
        }
        else
            break;
    }
    return result;
}

int main(){

    printf("Enter Expression\n");
    fgets(expr , MAX , stdin);

    int result = add_sub();
    while(isspace(expr[idx])) idx++;

    if(division_by_0)
        printf("Error: division by 0");
    
    else if(error || expr[idx] != '\0')
        printf("Error: Invalid expression");
    
    else
        printf("Result : %d" , result);

    return 0;
}