#!/bin/bash

#  Title          : bkupfiles.sh
#  Author         : Brandon Cohen
#  Created on     : October 2, 2023
#  Description    : A script that creates a backup file with the ending .bck. Paramters are unlimited, but they must be a file.
#  Purpose        : To familirize ourself with scripting and to backup files
#  Usage          : ./bkupfiles.sh
#  Build with     : ./bkupfiles [file1] [file2] [file3] ... [fileN]
#  Modifications  :

# Check if at least one argument is provided
if [ "$#" -lt 1 ]; then
    echo "No arguments."
    echo "bkupfiles [file1] [file2] [file3] ... [fileN]"
    exit 1
fi

# Iterate through the command-line arguments
for file in "$@"; do
    if [ -e "$file" ] || [ -e "file.bck" ]; then
        # Create a copy with the ending .bck suffix
        cp -f "$file" "$file.bck"
    else
        echo "$file does not exist."
    fi
done
