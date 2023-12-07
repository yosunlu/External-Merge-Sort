#!/bin/bash

# Directories to delete
DIR1="HDD"
DIR2="input"
DIR3="SSD-10GB"
DIR4="output"

# File to delete
FILE="trace0.txt"

# Function to delete a directory or file if it exists
delete_item() {
    if [ -d "$1" ]; then
        rm -rf "$1"
        echo "Deleted directory $1"
    elif [ -f "$1" ]; then
        rm -f "$1"
        echo "Deleted file $1"
    else
        echo "Item $1 not found."
    fi
}

# Delete the directories
delete_item "$DIR1"
delete_item "$DIR2"
delete_item "$DIR3"
delete_item "$DIR4"

# Delete the file
delete_item "$FILE"
