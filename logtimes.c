/*
#  Title          : logtimes.c
#  Author         : Brandon Cohen
#  Created on     : November 27, 2023
#  Description    : A C program that prints logtime statistics from the utmp file.
#  Purpose        : To combine multiple concepts such as string parsing, open/read functions, locales, etc.
#  Usage          : ./logtimes
#  Build with     : ./logtimes [options] username
#  Modifications  :
*/

// I used certain implemntations of last.c in this code.

#define _GNU_SOURCE
#include <paths.h>
#include <utmpx.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <locale.h>
#include <time.h>
#include <libgen.h>

#ifndef SHUTDOWN_TIME
    #define SHUTDOWN_TIME 32 /* Give it a value larger than the other types */
#endif

typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define MAXLEN 256
#define BAD_FORMAT_ERROR 2


void convertTime(time_t seconds) {
    int days, hours, minutes, secs; // Will hold the number of days, hours, minutes, seconds

    minutes  = (seconds / 60) % 60;
    hours    = (seconds / 3600) % 24;
    days     = secs / 86400;
    secs = seconds % 60;

    if (days > 0)
        printf("%d days", days);
    if (hours > 0)
        printf(" %d hours", hours);
    if (minutes > 0)
        printf(" %d mins", minutes);
    if (secs > 0)
        printf(" %d secs", secs);
    printf("\n");
}


// Linked List strucutre to hold users and tehir duration logged in
struct Users {
    char str[100];
    long num;
    struct Users* next;
};

// Function to create a new node with given string and integer values
struct Users* createNode(char str[], long num) {
    struct Users* newNode = (struct Users*)malloc(sizeof(struct Users));
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        exit(1);
    }
    strcpy(newNode->str, str);
    newNode->num = num;
    newNode->next = NULL;
    return newNode;
}

// Function to insert a new node at the end of the linked list
void insertEnd(struct Users** head, char str[], long num) {
    struct Users* newNode = createNode(str, num);
    if (*head == NULL) {
        *head = newNode;
    } else {
        struct Users* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = newNode;
    }
}


// Function to print the elements of the linked list
void printList(struct Users *head, char* flag) {
    struct Users* current = head;
    struct Users* temp;
    struct Users* prev;

    if (strcmp(flag, "") == 0) {
        while (current != NULL) {
            int sum = 0;
            temp = current->next;
            prev = current;

            while (temp != NULL) {
                if (strcmp(current->str, temp->str) == 0) {
                    sum += temp->num;
                    prev->next = temp->next;
                    free(temp);
                    temp = prev->next;
                } else {
                    prev = temp;
                    temp = temp->next;
                }
            }
            printf("%s ", current->str);
            convertTime(current->num + sum);
            current = current->next;
        }
    } else {

        while (current != NULL) {
            int sum = 0;
            temp = current->next;
            prev = current;

            while (temp != NULL) {
                if (strcmp(current->str, temp->str) == 0) {
                    // Check for the specific flag before summing up
                    if (strcmp(temp->str, flag) == 0) {
                        sum += temp->num;
                    }
                    prev->next = temp->next;
                    free(temp);
                    temp = prev->next;
                } else {
                    prev = temp;
                    temp = temp->next;
                }
            }

            if (strcmp(current->str, flag) == 0) {
                printf("%s ", current->str);
                convertTime(current->num + sum);
                return;
            }

            current = current->next;
        }
    }
}



/* Struct for the linked list of utmpx records. */
struct utmp_list{
    struct utmpx ut;
    struct utmp_list *next;
    struct utmp_list *prev;
};

typedef struct utmp_list utlist;

void fatal_error(int errnum, const char *message) {
    perror(message);  // perror prints the error message along with the error string for the current errno
    exit(errnum);     // Terminate the program with the specified exit code
}


/** reads one utmpx structure from the current file and
 * offset  into the address ut.
    Returns 1 for successful read and 0 if it could not read.
*/
int get_prev_utrec(int fd, struct utmpx *ut, BOOL *finished )
{
    static off_t  saved_offset;    /* Where this call is about to read    */
    static BOOL   is_first = TRUE; /* Whether this is first time called   */
    size_t utsize = sizeof(struct utmpx); /* Size of utmpx struct */
    ssize_t       nbytes_read;     /* Number of bytes read                */

    /* Check if this is the first time it is called.
       If so, move the file offset to the last record in the file
       and save it in saved_offset. */
    if ( is_first ) {
        errno = 0;
        /* Move to utsize bytes before end of file. */
        saved_offset = lseek(fd, -utsize, SEEK_END);
        if ( -1 == saved_offset )  {
            fprintf(stderr, "error trying to move offset to last rec of file");
            return FALSE;
        }
        is_first = FALSE; /* Turn off flag. */
    }

    *finished = FALSE;       /* Assume we're not done yet. */
    if ( saved_offset < 0 ) {
        *finished = TRUE;   /* saved_offset < 0 implies we've read entire file. */
        return FALSE;    /* Return 0 to indicate no read took place.         */
    }
    /* File offset is at the correct place to read. */
    errno = 0;
    nbytes_read = read(fd, ut, utsize);
    if ( -1 == nbytes_read ) {
        /* read() error occurred; do not exit - let main() do that. */
        fprintf(stderr, "Read did not occur.\n");
        return FALSE;
    }
    else if ( nbytes_read < utsize ) {
        /* Full utmpx struct not read; do not exit - let main() do that. */
        fprintf(stderr, "The record utmpx struct was not fully read.\n");
        return FALSE;
    }
    else { /* Successful read of utmpx record */
        saved_offset = saved_offset - utsize; /* Reposition saved_offset. */
        if ( saved_offset >= 0 ) {
            /* Seek to preceding record to set up next read. */
            errno = 0;
            if ( -1 == lseek(fd, - (2*utsize), SEEK_CUR) )
                fatal_error(errno, "lseek()");
        }
        return TRUE;
    }
}


void print_one_line(struct utmpx *ut, time_t end_time, struct Users **users)
{
    time_t utrec_time;
    char duration[MAXLEN]; /* String representing session length */
    char start[50]; // start time
    char end[50];   // end time

    utrec_time = (ut->ut_tv).tv_sec; /* Get login time, in seconds */

    // Calculate total time in seconds
    time_t total_time = end_time - utrec_time;

    // Insert user name and total time into the Users linked list
    insertEnd(users, ut->ut_user, total_time);

    //printf("%-8.8s Start: %s | End: %s | Duration: %s | Total Time: %ld seconds\n", ut->ut_user, start, end, duration, (long)total_time);
}


void save_ut_to_list(struct utmpx *ut,  utlist **list)
{
    utlist* utmp_node_ptr;

    /* Allocate a new list node. */
    errno = 0;
    if ( NULL == (utmp_node_ptr = (utlist*) malloc(sizeof(utlist)) ) )
        fatal_error(errno, "malloc");

    /* Copy the utmpx record into the new node. */
    memcpy(&(utmp_node_ptr->ut), ut, sizeof(struct utmpx));

    /* Attached the node to the front of the list. */
    utmp_node_ptr->next  = *list;
    utmp_node_ptr->prev  = NULL;
    if (NULL != *list)
        (*list)->prev = utmp_node_ptr;
    (*list) = utmp_node_ptr;
}

void delete_utnode(utlist* p, utlist** list)
{
    if ( NULL != p->next )
        p->next->prev = p->prev;

    if ( NULL != p->prev )
        p->prev->next = p->next;
    else
        *list = p->next;
    free(p);
}

void erase_utlist(utlist **list)
{
    utlist *ptr = *list;
    utlist *next;

    while ( NULL != ptr ) {
        next = ptr->next;
        free(ptr);
        ptr = next;
    }
    *list = NULL;
}


int main( int argc, char* argv[] )
{
    /* Set the locale. */
    setlocale(LC_TIME, "");

    struct Users* users = NULL;

    struct utmpx  utmp_entry;              /* Read info into here             */
    size_t        utsize = sizeof(struct utmpx); /* Size of utmpx record      */
    int           fd_utmp;                 /* Read from this descriptor       */
    time_t        start_time;              /* When wtmp processing started    */
    utlist        *saved_ut_recs = NULL;   /* An initially empty list         */
    char options[] = ":af:";               // Option a (optional argument) and f (required argument)
    char          usage_msg[MAXLEN];       /* For error messages              */
    char*         wtmp_path = _PATH_WTMP;
    BOOL          done = FALSE;
    BOOL          found = FALSE;
    char          ch;
    utlist        *p, *next;
    int flag = 1;                           // Used to dictate which print will occur
    char* username = getlogin();            // Get the username

    /* Check options */
    opterr = 0;  /* Turn off error messages by getopt() */

    while  (TRUE) {

        /* Call getopt, passing argc and argv and the options string. */
        ch = getopt(argc, argv, options);
        if ( -1 == ch ) /* It returns -1 when it finds no more options.  */
            break;

        switch ( ch ) {
        case 'f':
            wtmp_path = optarg;
            if(4 == argc)
                username = argv[3];
            break;

        case 'a':
            if ( (fd_utmp = open(wtmp_path, O_RDONLY)) == -1 ) {
                fatal_error(errno, wtmp_path);
            }

            /* Read the first structure in the file to capture the time of the
            first entry. */
            errno = 0;
            if ( read(fd_utmp, &utmp_entry, utsize) != utsize )
                fatal_error(errno, "read");

            start_time = utmp_entry.ut_tv.tv_sec ;

            /* Process the wtmp file */
            while ( !done ) {
                errno = 0;
                if ( get_prev_utrec(fd_utmp, &utmp_entry, &done)  ) {
                    switch (utmp_entry.ut_type) {
                    case USER_PROCESS:
                        /* Find the logout entry for this login in the saved_ut_recs */
                        found = TRUE;
                        p = saved_ut_recs; /* start at beginning */
                        while ( NULL != p ) {
                            next = p->next;
                            if ( 0 == (strncmp(p->ut.ut_line, utmp_entry.ut_line,
                                sizeof(utmp_entry.ut_line)) ) ) {
                                print_one_line(&utmp_entry, p->ut.ut_tv.tv_sec, &users);
                                found = TRUE;
                                delete_utnode(p, &saved_ut_recs); /* Delete the node */
                            }
                            p = next;
                        }
                        break;
                    case DEAD_PROCESS:
                        if ( utmp_entry.ut_line[0] == 0 )
                            continue;
                        else
                            save_ut_to_list(&utmp_entry, &saved_ut_recs);
                        break;
                    }
                }
                else /* get_prev_utrec() did not read. */
                    if ( !done )
                        fatal_error(2, " read failed");
            }

            flag = 0;
            printList(users, "");

            break;

        case '?' :
        case ':' :
            fprintf(stderr,"Found invalid option %c\n", optopt);
            sprintf(usage_msg, "%s [-a] || [-f]", basename(argv[0]));
            flag = 0;
            break;
        }
    }


    if (1 == flag) {
        if ( (fd_utmp = open(wtmp_path, O_RDONLY)) == -1 ) {
            fatal_error(errno, wtmp_path);
        }

        /* Read the first structure in the file to capture the time of the
        first entry. */
        errno = 0;
        if ( read(fd_utmp, &utmp_entry, utsize) != utsize )
            fatal_error(errno, "read");

        start_time = utmp_entry.ut_tv.tv_sec ;

        /* Process the wtmp file */
        while ( !done ) {
            errno = 0;
            if ( get_prev_utrec(fd_utmp, &utmp_entry, &done)  ) {
                switch (utmp_entry.ut_type) {
                case USER_PROCESS:
                    /* Find the logout entry for this login in the saved_ut_recs */
                    found = TRUE;
                    p = saved_ut_recs; /* start at beginning */
                    while ( NULL != p ) {
                        next = p->next;
                        if ( 0 == (strncmp(p->ut.ut_line, utmp_entry.ut_line,
                            sizeof(utmp_entry.ut_line)) ) ) {
                            print_one_line(&utmp_entry, p->ut.ut_tv.tv_sec, &users);
                            found = TRUE;
                            delete_utnode(p, &saved_ut_recs); /* Delete the node */
                        }
                        p = next;
                    }
                    break;
                case DEAD_PROCESS:
                    if ( utmp_entry.ut_line[0] == 0 )
                        continue;
                    else
                        save_ut_to_list(&utmp_entry, &saved_ut_recs);
                    break;
                }
            }
            else /* get_prev_utrec() did not read. */
                if ( !done )
                    fatal_error(2, " read failed");
        }

        if (2 == argc){
            printList(users, argv[1]);
        } else {
            printList(users, username);
        }

        //printList(users, username);
    }

    erase_utlist(&saved_ut_recs);
    close(fd_utmp);
    return 0;
}