#include<stdio.h>
#include<ctype.h>
#include<string.h>

#define MAX 1000

typedef struct {
    const char *expr;
    int idx;
    int error;
    int division_by_0;
} Parser;

int addSub(Parser *p);
// Read a number
int getNumber(Parser *p){
    
    if (p->expr[p->idx] == '(') {
        p->idx++; 
        int result = addSub(p);
        while (isspace(p->expr[p->idx])) p->idx++;

        if (p->expr[p->idx] == ')') {
            p->idx++; 
            return result;
        } else {
            p->error = 1;
            return 0;
        }
    }
    while(isspace(p->expr[p->idx])) p->idx++;

    //to handles negative numbers
    int sign = 1;
    if(p->expr[p->idx] == '-'){
        sign = -1;
        p->idx++;
    }
    else if(p->expr[p->idx] == '+'){
        sign = 1;
        p->idx++;
    }

    if(!isdigit(p->expr[p->idx])){
        p->error = 1;
        return 0;
    }

    int num = 0;
    while(isdigit(p->expr[p->idx])){
        num = num * 10 + (p->expr[p->idx] - '0');
        p->idx++;
    }

    return sign * num;
}

//Handle multiplication and divistion
int mulDiv(Parser *p){
    int result = getNumber(p);

    while(!p->error){
        while(isspace(p->expr[p->idx])) p->idx++;

        if(p->expr[p->idx] == '*'){
            p->idx++;
            result *= getNumber(p);
        }
        else if(p->expr[p->idx] == '/'){
            p->idx++;
            int val = getNumber(p);

            if(p->error)return 0;

            if(val == 0){
                p->division_by_0 = 1;
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
int addSub(Parser *p){

    int result = mulDiv(p);
    
    while(!p->error && !p->division_by_0){
        while(isspace(p->expr[p->idx])) p->idx++;

        if(p->expr[p->idx] == '+'){
            p->idx++;
            result += mulDiv(p);
        }
        else if(p->expr[p->idx] == '-'){
            p->idx++;
            result -= mulDiv(p);
        }
        else
            break;
    }
    return result;
}

int main(){
    char expr[MAX];

    printf("Enter Expression\n");
    if (fgets(expr, sizeof(expr), stdin) == NULL) return 0;

    expr[strcspn(expr, "\n")] = '\0';

    Parser p = { .expr = expr, .idx = 0, .error = 0, .division_by_0 = 0 };

    int result = addSub(&p);
    while(isspace(p.expr[p.idx])) p.idx++;

    if(p.division_by_0)
        printf("error: division by 0\n");
    else if(p.error || p.expr[p.idx] != '\0')
        printf("error: Invalid expression\n");
    else
        printf("Result : %d\n", result);

    return 0;
}