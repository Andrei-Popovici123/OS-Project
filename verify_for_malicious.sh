#!/bin/bash

if grep -axv '.*' "$1" >/dev/null 2>&1; then
  echo "File $1 contains non-ASCII characters"
fi

# Check if the file contains key words associated with dangerous files
KEY_WORDS=("corrupted" "dangerous" "risk" "attack" "malware" "malicious")
for WORD in "${KEY_WORDS[@]}"; do
  if grep -q "$WORD" "$1"; then
    echo "File $1 contains the key word $WORD"
  fi
done
# Check if the file has fewer than 3 lines and the number of words exceeds 1000, and the number of characters exceeds 2000
if [ $(wc -l < "$1") -lt 3 ] && [ $(wc -w < "$1") -gt 1000 ] && [ $(wc -m < "$1") -gt 2000 ]; then
  echo "File $1 is suspicious"
fi
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