#!/bin/bash

FILE_PATH="$1"
ISOLATED_SPACE_DIR="$2"

# Check if the file exists
if [! -f "$FILE_PATH" ]; then
    echo "File not found: $FILE_PATH"
    exit 1
fi

# Perform syntactic analysis
NUM_LINES=$(wc -l < "$FILE_PATH")
NUM_WORDS=$(wc -w < "$FILE_PATH")
NUM_CHARS=$(wc -m < "$FILE_PATH")

# Check for keywords associated with corrupted or malicious files
if grep -q "corrupted\|dangerous\|risk\|attack\|malware\|malicious" "$FILE_PATH"; then
    echo "Malicious keywords found in $FILE_PATH"
    mv "$FILE_PATH" "$ISOLATED_SPACE_DIR"
    exit 1
fi

# Check for non-ASCII characters
if grep -q "[^[:ascii:]]" "$FILE_PATH"; then
    echo "Non-ASCII characters found in $FILE_PATH"
    mv "$FILE_PATH" "$ISOLATED_SPACE_DIR"
    exit 1
fi

# If no issues are found, print the results
echo "File $FILE_PATH has $NUM_LINES lines, $NUM_WORDS words, and $NUM_CHARS characters."