#!/bin/bash

# Directories to delete
DIR1="HDD"
DIR2="input"
DIR3="SSD-10GB"

# Function to delete a directory if it exists
delete_directory() {
    if [ -d "$1" ]; then
        rm -rf "$1"
        echo "Deleted $1"
    else
        echo "Directory $1 not found."
    fi
}

# Delete the directories
delete_directory "$DIR1"
delete_directory "$DIR2"
delete_directory "$DIR3"
