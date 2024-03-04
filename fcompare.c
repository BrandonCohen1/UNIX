/*
#  Title          : fcompare.c
#  Author         : Brandon Cohen
#  Created on     : December 13, 2023
#  Description    : A C program that prints sorted stat member in increasing or decreasing order.
#  Purpose        : To become more familair with statx
#  Usage          : ./fcompare [-abcmsu] [-lr] files ...
#  Build with     : gcc fcompare.c -o fcompare
#  Modifications  :
*/

// Note: I used some code from the showstat.c program.

/*
DESCRIPTION OF HOW I WROTE THE CODE
In the program, I used the statx system call to receive metadata about files and symbolic links, such as access time, birth time,
modification time, file size, and block count to sort the files provided on the command line. To begin this, I used the standard libraries
to have access to all necessary functions, but since the files needed to be stored, I thought the best way of implementing this was to
dynamically allocate the memory to hold the data. The next step was trying to figure out what data structure I was going to use to
 store this data so I used the man pages to find any functions that do this already for statx structs. I didn't find any functions that
would be helpful so instead I thought that the easiest implementation would be a array because it is simple to build, I can
dynamically allocate it easily using malloc, and I can print the reverse very easily. One problem that I had to solve was how I was
going to sort the struct Files, so I used the man page to find a function that could do this with a array. I hit gold because I found a
 qsort algorithm that could do this but the only thing I had to do was implement the comparison function which was very simple since
I was dealing with numbers. Looking back at my code, I think I wrote too much code. Firstly, the struct Files held three data members
 (name of the file, long long for either the size or block count, and time_t for the time), but I feel like I could have used one less data
member. I probably could have also used two arrays where the first one would hold the name of the string and the other array would
 contain its corresponding data member. This would mean that whenever the array becomes sorted, the files would have to move
during the sorting so they can match the same index as their data member. This would have saved some code by not writing two
 comparison functions (one for the size/block size and the other for time_t). I would also cut down on the code by adding a function
 that would check if the file is a symbolic link so that I could get rid of the repetitive code. I would also look up more sorting
algorithms. I am not familiar with all of the sorting algorithms provided by UNIX, so I believe looking into this could improve the
performance. Overall, in my code, I tried to keep the code portable so I included things like EXIT_FAILURE and I used setlocale so
that the data could be printed in the user's locale. If I were to start it all over, I think I would still use the array since it is very easy
 to print the reverse list. I would add more functions to cut down on the repetitive code and I probably would do more unit testing
before writing the whole code and then testing it. I would also change my testing procedure. For this project, I used files that were all
 within the same day so comparing the results was difficult since the numbers were so close to each other. I also only tested one
symbolic linked file with a group of regular files, but I think it would have been nice to test all symbolic links to make sure there are no
 errors. The way I compared the results was using stat in the command line, but if I could do it again, I would write a bash script
where I could put in the same files I used in the ./fcompare, and then in the bash script, I could grep so I only get the data member
that I need. This would have saved me time from typing in stat for every file.
*/

#define _GNU_SOURCE         // Needed to expose statx() function in glibc
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <fcntl.h>
#include <locale.h>


// Array to hold the file name and its data
typedef struct {
    char str[200];
    long long num;
    time_t date;
} Files;


// Comparison function for qsort using size or block count
int compare_num(const void *a, const void *b) {
    Files *fileA = (Files *)a;
    Files *fileB = (Files *)b;
    return (fileA->num - fileB->num);
}

// Comparison function for qsort for time_t
int compare_date(const void *a, const void *b) {
    Files *fileA = (Files *)a;
    Files *fileB = (Files *)b;
    return (fileA->date - fileB->date);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    struct statx file_stats;        // Will store statistics of the file
    unsigned int mask;              // mask to pass to statx()
    int report_on_link;             // Flag to report if it is a link

    // Mask set to all avaliable data
    mask = STATX_ALL;

    // Default is to report on symbolic links, not their targets!
    report_on_link = AT_SYMLINK_NOFOLLOW;

    // Set Locale
    if ( setlocale(LC_TIME, "") == NULL )
        perror("setlocale");

    int opt;
    char options[] = "abcmrsul";
    int opt_a = 0, opt_b = 0, opt_c = 0, opt_m = 0, opt_r = 0, opt_s = 0, opt_u = 0, opt_l = 0;
    int i = 0;  // Index of array

    // Check if options were present
    while ((opt = getopt(argc, argv, options)) != -1) {
        switch (opt) {
            case 'a':
                opt_a++;
                break;
            case 'b':
                opt_b++;
                break;
            case 'c':
                opt_c++;
                break;
            case 'm':
                opt_m++;
                break;
            case 'r':
                opt_r++;
                break;
            case 's':
                opt_s++;
                break;
            case 'u':
                opt_u++;
                break;
            case 'l':
                opt_l++;
                break;
            default:
                fprintf(stderr, "Usage: %s [-abcmsu] [-lr] [file...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Only one format option can be given, if there are more exit without an error
    int check_opt = opt_a + opt_b + opt_c + opt_m + opt_s + opt_u;

    // Print error if there was more than one print option or if there were none at all
    if (check_opt > 1 || 0 == check_opt) {
        fprintf(stderr, "Only one print option is allowed (a, b, c, m, s, or u).\nUsage: %s [-abcmsu] [-lr] files ...\n", argv[0]);
        exit(EXIT_FAILURE);
    }


    // Dynamically allocate an array of File structs
    Files *files = malloc((argc - optind) * sizeof(Files));

    // Check if there was any errors using malloc
    if (files == NULL) {
        fprintf(stderr, "Failed to allocate memory\n");
        return 1;
    }

    // Option a was chosen
    if (1 == opt_a) {
        // Option a (access time) and look at target of symbolic link
        if (1 == opt_l) {
            while (optind < argc) {

                // Checks if the file can open correctly
                if ( statx(AT_FDCWD, argv[optind], report_on_link, mask, &file_stats) < 0 ) {
                    fprintf(stderr, "statx could not open file %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                // If a symbolic link
                if ( S_ISLNK(file_stats.stx_mode)) {
                    if ( statx(AT_FDCWD, argv[optind], 0, mask, &file_stats) < 0 ) {
                        fprintf(stderr, "statx could not open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }

                    // Copy the name of the file
                    strcpy(files[i].str, argv[optind]);
                    // Copy the time
                    files[i].date = (time_t)file_stats.stx_atime.tv_sec;
                    optind++;
                    i++;
                } else {
                    // Copy the name of the file
                    strcpy(files[i].str, argv[optind]);
                    // Copy the name
                    files[i].date = (time_t)file_stats.stx_atime.tv_sec;
                    optind++;
                    i++;
                }
            }
        } else {

            // Option l is not given
            while (optind < argc) {
                if (statx(AT_FDCWD, argv[optind], report_on_link, STATX_ALL, &file_stats) < 0) {
                    perror("statx");
                    return 1;
                }

                // Copy the string
                strcpy(files[i].str, argv[optind]);
                // Copy the date
                files[i].date = (time_t)file_stats.stx_atime.tv_sec;
                optind++;
                i++;
            }
        }

    } else if (1 == opt_b) {
        // Option b (birth time) and look at target of symbolic link
        if (1 == opt_l) {
            while (optind < argc) {

                // Checks if the file can open correctly
                if ( statx(AT_FDCWD, argv[optind], report_on_link, mask, &file_stats) < 0 ) {
                    fprintf(stderr, "statx could not open file %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                // If a symbolic link
                if ( S_ISLNK(file_stats.stx_mode)) {
                    if ( statx(AT_FDCWD, argv[optind], 0, mask, &file_stats) < 0 ) {
                        fprintf(stderr, "statx could not open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }

                    // Copy the file name
                    strcpy(files[i].str, argv[optind]);
                    // Copy the date
                    files[i].date = (time_t)file_stats.stx_btime.tv_sec;
                    optind++;
                    i++;
                } else {
                    // Copy the file name and the date
                    strcpy(files[i].str, argv[optind]);
                    files[i].date = (time_t)file_stats.stx_btime.tv_sec;
                    optind++;
                    i++;
                }
            }
        } else {
            // Option l is not given
            while (optind < argc) {
                if (statx(AT_FDCWD, argv[optind], report_on_link, STATX_ALL, &file_stats) < 0) {
                    perror("statx");
                    return 1;
                }

                // Copy the name of the file and data
                strcpy(files[i].str, argv[optind]);

                files[i].date = (time_t)file_stats.stx_btime.tv_sec;
                optind++;
                i++;
            }
        }

    } else if (1 == opt_c) {
        // Look at target of symbolic link
        if (1 == opt_l) {
            while (optind < argc) {
                // Checks if the file can open correctly
                if ( statx(AT_FDCWD, argv[optind], report_on_link, mask, &file_stats) < 0 ) {
                    fprintf(stderr, "statx could not open file %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                // If a symbolic link
                if ( S_ISLNK(file_stats.stx_mode)) {
                    if ( statx(AT_FDCWD, argv[optind], 0, mask, &file_stats) < 0 ) {
                        fprintf(stderr, "statx could not open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }

                    // Copy the file name
                    strcpy(files[i].str, argv[optind]);
                    // Copy the date
                    files[i].date = (time_t)file_stats.stx_btime.tv_sec;
                    optind++;
                    i++;
                } else {
                    // Copy the file name
                    strcpy(files[i].str, argv[optind]);
                    // Copy the date
                    files[i].date = (time_t)file_stats.stx_btime.tv_sec;
                    optind++;
                    i++;
                }
            }
        } else {
            // Option l is not given
            while (optind < argc) {
                if (statx(AT_FDCWD, argv[optind], report_on_link, STATX_ALL, &file_stats) < 0) {
                    perror("statx");
                    return 1;
                }

                // Copy the name and date
                strcpy(files[i].str, argv[optind]);
                files[i].date = (time_t)file_stats.stx_btime.tv_sec;
                optind++;
                i++;
            }
        }

    } else if (1 == opt_m) {
        if (1 == opt_l) {
            while (optind < argc) {
                // Checks if the file can open correctly
                if ( statx(AT_FDCWD, argv[optind], report_on_link, mask, &file_stats) < 0 ) {
                    fprintf(stderr, "statx could not open file %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                // If a symbolic link
                if ( S_ISLNK(file_stats.stx_mode)) {
                    if ( statx(AT_FDCWD, argv[optind], 0, mask, &file_stats) < 0 ) {
                        fprintf(stderr, "statx could not open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }

                    strcpy(files[i].str, argv[optind]);
                    files[i].date = (time_t)file_stats.stx_mtime.tv_sec;
                    optind++;
                    i++;
                } else {
                    strcpy(files[i].str, argv[optind]);
                    files[i].date = (time_t)file_stats.stx_mtime.tv_sec;
                    optind++;
                    i++;
                }
            }
        } else {
            // Option l is not given
            while (optind < argc) {
                if (statx(AT_FDCWD, argv[optind], report_on_link, STATX_ALL, &file_stats) < 0) {
                    perror("statx");
                    return 1;
                }

                strcpy(files[i].str, argv[optind]);
                files[i].date = (time_t)file_stats.stx_mtime.tv_sec;
                optind++;
                i++;
            }
        }

    } else if (1 == opt_s) {
        if (1 == opt_l) {
            while (optind < argc) {
                // Checks if the file can open correctly
                if ( statx(AT_FDCWD, argv[optind], report_on_link, mask, &file_stats) < 0 ) {
                    fprintf(stderr, "statx could not open file %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                // If a symbolic link
                if ( S_ISLNK(file_stats.stx_mode)) {
                    if ( statx(AT_FDCWD, argv[optind], 0, mask, &file_stats) < 0 ) {
                        fprintf(stderr, "statx could not open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }

                    strcpy(files[i].str, argv[optind]);
                    files[i].num = (long long)file_stats.stx_size;
                    optind++;
                    i++;
                } else {
                    strcpy(files[i].str, argv[optind]);
                    files[i].num = (long long)file_stats.stx_size;
                    optind++;
                    i++;
                }
            }
        } else {
            // Option l is not given
            while (optind < argc) {
                if (statx(AT_FDCWD, argv[optind], report_on_link, STATX_ALL, &file_stats) < 0) {
                    perror("statx");
                    return 1;
                }

                strcpy(files[i].str, argv[optind]);
                files[i].num = (long long)file_stats.stx_size;
                optind++;
                i++;
            }
        }

    } else if (1 == opt_u) {
        // Process remaining arguments (not options)
        if (1 == opt_l) {
            while (optind < argc) {
                // Checks if the file can open correctly
                if ( statx(AT_FDCWD, argv[optind], report_on_link, mask, &file_stats) < 0 ) {
                    fprintf(stderr, "statx could not open file %s\n", argv[i]);
                    exit(EXIT_FAILURE);
                }

                // If a symbolic link
                if ( S_ISLNK(file_stats.stx_mode)) {
                    if ( statx(AT_FDCWD, argv[optind], 0, mask, &file_stats) < 0 ) {
                        fprintf(stderr, "statx could not open file %s\n", argv[i]);
                        exit(EXIT_FAILURE);
                    }

                    strcpy(files[i].str, argv[optind]);
                    files[i].num = (long long)file_stats.stx_blocks;
                    optind++;
                    i++;
                } else {
                    strcpy(files[i].str, argv[optind]);
                    files[i].num = (long long)file_stats.stx_blocks;
                    optind++;
                    i++;
                }
            }
        } else {
            // Option l is not given
            while (optind < argc) {
                if (statx(AT_FDCWD, argv[optind], report_on_link, STATX_ALL, &file_stats) < 0) {
                    perror("statx");
                    return 1;
                }

                strcpy(files[i].str, argv[optind]);
                files[i].num = (long long)file_stats.stx_blocks;
                optind++;
                i++;
            }
        }
    }


    if(opt_a > 0 || opt_b > 0 || opt_c > 0 || opt_m > 0){
        qsort(files, i, sizeof(Files), compare_date);

        if(0 == opt_r){
            for (int j = 0; j < i; j++) {
                printf("%s %s", files[j].str, ctime((time_t*)&files[j].date) );
            }
        } else {
            for (int j = i-1; j >= 0; j--) {
                printf("%s %s", files[j].str, ctime((time_t*)&files[j].date) );
            }
        }
    }

    if(opt_s > 0 || opt_u > 0){
        qsort(files, i, sizeof(Files), compare_num);

        if(0 == opt_r){
            for (int j = 0; j < i; j++) {
                printf("%s %d\n", files[j].str, (int)files[j].num );
            }
        } else {
            for (int j = i-1; j >= 0; j--) {
                printf("%s %d\n", files[j].str, (int)files[j].num );
            }
        }
    }

    free(files);

    return 0;
}