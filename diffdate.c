/*
#  Title          : diffdate.c
#  Author         : Brandon Cohen
#  Created on     : November 7, 2023
#  Description    : A C program that prints the days in between two days
#  Purpose        : To understand time and locale used in Linux
#  Usage          : ./diffdate
#  Build with     : ./diffdate date1 date2
#  Modifications  :
*/

// Note: To compile, use -lm

#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <locale.h>

int check_for_letters(char* cmd, char* input){
    char *endptr;
    // Check to see if there are any letters in the date
    long long val = strtoll(input, &endptr, 10);
    if (*endptr != '\0' || errno == ERANGE) {
        fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD]\n", cmd);
        exit(-1);
    } else {
        // No error
        return 0;
    }
}

void compare_dates(struct tm d1, struct tm d2, int today){
    time_t time1, time2;

    // mktime() converts struct tm into seconds as long int from EPOCH
    time1 = mktime(&d1);
    time2 = mktime(&d2);
   // date1, today -> today - time1 = > 0
    double diff = difftime(time1, time2); //time2 - time1

    int days = round(diff / 86400);


    // Create output buffer to store the date as MM DD, YYYY
    char output1[30];
    char output2[30];
    strftime(output1, sizeof(output1), "%B %d, %Y", &d1);
    strftime(output2, sizeof(output2), "%B %d, %Y", &d2);

    // When comparing days between today
    if(1 == today){

        if(days+1 > 0){
            // after
            strftime(output1, sizeof(output1), "%B %d, %Y", &d1);

            printf("%s is %d days after today\n", output1, abs(days+1));

        } else if(0 == abs(days+1)){
            strftime(output1, sizeof(output1), "%B %d, %Y", &d1);

            printf("%s is the same as %s\n", output1, output1);
        } else {

            // before
            strftime(output1, sizeof(output1), "%B %d, %Y", &d1);

            printf("%s was %d days before today\n", output1, abs(days+1));
        }
    } else {
        // Comparing two different days
        if(days > 0){
            // before
            strftime(output1, sizeof(output1), "%B %d, %Y", &d1);
            strftime(output2, sizeof(output2), "%B %d, %Y", &d2);

            printf("%s was %d days before %s\n", output2, abs(days), output1);

        } else if(0 == days){
            strftime(output1, sizeof(output1), "%B %d, %Y", &d1);

            printf("%s is the same as %s\n", output1, output1);
        } else {

            // after
            strftime(output1, sizeof(output1), "%B %d, %Y", &d1);
            strftime(output2, sizeof(output2), "%B %d, %Y", &d2);

            printf("%s is %d days after %s\n", output2, abs(days), output1);
        }
    }
}

int main(int argc, char *argv[]) {
    setlocale(LC_ALL, "");

    if(argc == 1) {
        fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD] \nNot enough arguments\n", argv[0]);
    }

    int flag; // Flag is for edge case for any month that has less than 31 days

    // Check for errors in the arguments
    for(int i=1; i < argc ; i++){
        char date_copy[strlen(argv[i]) + 1];
        strcpy(date_copy, argv[i]);

        char *token = strtok(date_copy, "-");

        int j = 0;
        while (token != NULL) {

            if(j == 0) {
                // Year can be any number
                if(atoi(token) == 0){
                    // Check if there is invalid date due to characters
                    check_for_letters(argv[0], token);
                    printf("%s\n", token);
                }

            } else if (j == 1) {
                check_for_letters(argv[0], token);
                // Month: 1-12

                if(atoi(token) < 1 || atoi(token) > 12){
                    fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD].\n Month can only be between 01 (Jan) to 12 (Dec)\n", argv[0]);
                    exit(-1);
                    printf("%s\n", token);
                }

                if(atoi(token) == 2){
                    // Month of Febraury
                    flag = 0;
                }
                if( atoi(token) == 4 || atoi(token) == 6 || atoi(token) == 9 || atoi(token) == 11 ){
                    // April, June, September, November
                    flag = 1;
                }

            } else if (j == 2) {
                check_for_letters(argv[0], token);

                // Date: 1-31
                if(atoi(token) < 1 || atoi(token) > 31){
                    fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD].\n Date can only be between 01 to 31\n", argv[0]);
                    exit(-1);
                }


                if(0 == flag){
                    if(atoi(token) > 29){
                        fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD].\n Month of February only has 28 or 29 days.\n", argv[0]);
                        exit(-1);
                    }
                }
                if(1 == flag){

                    if(atoi(token) > 30) {
                        fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD].\n April, June, September, and November only have 30 days.\n", argv[0]);
                        exit(-1);
                    }
                }
            flag = -1;

            } else {
                fprintf(stderr, "USAGE: %s [YYYY-MM-DD] [YYYY-MM-DD]. \nToo many arguments.\n", argv[0]);
                exit(-1);
            }

            // Set token to the next date
            token = strtok(NULL, "-");
            j++;
        }
    }

    struct tm date1;

    if(argc == 3){
        struct tm date2;
        if (strptime(argv[1], "%Y-%m-%d", &date1) == NULL) {
            fprintf(stderr, "Invalid date format for date1: %s\n", argv[1]);
            return -1;
        }
        if (strptime(argv[2], "%Y-%m-%d", &date2) == NULL) {
            fprintf(stderr, "Invalid date format for date2: %s\n", argv[2]);
            return -1;
        }

        compare_dates(date1, date2, 0);
    } else {
        // Only one argument
        time_t n;
        struct tm *today;
        n = time(NULL);
        today = localtime(&n);

        if (strptime(argv[1], "%Y-%m-%d", &date1) == NULL) {
            fprintf(stderr, "Invalid date format for date1: %s\n", argv[1]);
            return -1;
        }

        compare_dates(date1, *today, 1);
    }

    return 0;
}