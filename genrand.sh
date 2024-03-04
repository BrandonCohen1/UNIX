#!/bin/bash


#  Title          : genrand.sh
#  Author         : Brandon Cohen
#  Created on     : October 2, 2023
#  Description    : A script that randomly shuffles each line. There are two parameters: an integer (N) and a file. The integer indicates the numbers from 1 to N will be written seperately on a line to the file. If the file doesn't exist, it will be created.
#  Purpose        : To familirize ourself with scripting and to shuffle lines of a file
#  Usage          : ./genrand.sh
#  Build with     : ./genrand [N] [filename]
#  Modifications  :


# Check if there are not 2 arguments
if [ "$#" -ne 2 ]; then
    # Check if parameters are >= 3 which will exit and print error
    if [ "$#" -ge 3 ]; then
        echo "Too many arguments (more than 3 arguments)."
        echo "./genrand.sh [N] [filename]"
        exit 1
    fi

    # Check if parameters are less than 2
    if [ "$#" -lt 2 ]; then
        # If there is no paramter, then exit and print error.
        if [ "$#" -eq 0 ]; then
            echo "There are no arguments. There needs to be 2 arguments."
            echo "./genrand.sh [N] [filename]"
            exit 1
        fi

        # If there is one paramter, then exit and print error.
        if [ "$#" -eq 1 ]; then
            echo "There was only 1 argument, but 2 arguments are required."
            echo "./genrand.sh [N] [filename]"
            exit 1
        fi
    fi
fi

# Check if the second paramter is empty, then exit and print error.
# The purpose of this if statemnt is that ./genrand [N] would still produce a file when it should not have. This prevented this from occuring.
if [ -z "$2" ]; then
    echo "You need to provide a filename"
    echo "./genrand.sh [N] [filename]"
    exit 1
fi

# Check if the first parameter is not positive integer. If not, exit and print error.
if [[ ! $1 =~ ^[0-9]+$ ]]; then
    echo "$1 is not a positive integer."
    echo "./genrand.sh [N] [filename]"
    exit 1
fi


# Check if the file already exists and if not, create it.

# File exist
if [ -e "$2" ]; then
    rm -f $2
fi

# Create file with [filename]
touch "$2"

# Loop to write numbers to the file
for ((i = 1; i <= $1; i++)); do
    echo "$i" >> "$2"
done

# Shuffle each line and rewrite to file
shuf -o "$2" "$2"

