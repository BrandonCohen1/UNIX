/*
#  Title          : myseq.c
#  Author         : Brandon Cohen
#  Created on     : October 19, 2023
#  Description    : A C program that prints increasing or decreasing numbers given an incrementor or decrementor.
#  Purpose        : To similate the seq command and to learn how to write out first code
#  Usage          : ./myseq
#  Build with     : ./myseq <num1> [increment] [<num2>]
#  Modifications  :
*/

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[]) {


    // If there are between one and three arguments or less,
    if (argc > 1 && argc <= 4) {
        // Check if any argument is not an integer.
        for (int i=1; i<argc; i++) {
            char *endptr;
            long long val = strtoll(argv[i], &endptr, 10);
            if (*endptr != '\0' || errno == ERANGE) {
                fprintf(stderr, "USAGE: %s <num1> [increment] [<num2>]\n", argv[0]);
                return -1;
            }
        }

        // If only one number given,
        if(argc == 2){
            // and if the first argument is less than 1,
            if(atoi(argv[1]) < 1){
                // then do nothing.
            } else {
                // else print from 1 to num1 in increments of 1.
                for (int i=1; i<=atoi(argv[1]); i++){
                    printf("%d\n", i);
                }
            }
        }

        // If there is only 2 numbers (num1 and num2)
        if(argc == 3){
            // and if num2 is less than num1,
            if(atoi(argv[2]) < atoi(argv[1])){
                // then do nothing.
            } else {
                // else print from num1 to num2 in increments of 1.
                for (int i=atoi(argv[1]); i<=atoi(argv[2]); i++){
                    printf("%d\n", i);
                }
            }
        }

        // If there are three numbers,
        if(argc == 4){
            // and if the increment is 0, print error.
            if(atoi(argv[2]) == 0){
                fprintf(stderr, "Increment must be a non-zero number\n");
                return -1;
            }
            // and if the increment is positive,
            if(atoi(argv[2]) > 0){
                // then print from num1 to num2 increasing by the increment for each iteration.
                for (int i=atoi(argv[1]); i<=atoi(argv[3]); i+=atoi(argv[2])){
                    printf("%d\n", i);
                }
            } else {
                // else print from num1 to num2 decreasing by the increment for each increment.
                for (int i=atoi(argv[1]); i>=atoi(argv[3]); i+=atoi(argv[2])){
                    printf("%d\n", i);
                }
            }
        }
    } else {
        // Do nothing
    }

    return 0;
}