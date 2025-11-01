#!/bin/bash

if [ $# -lt 2 ]; then
    echo "Usage: $0 <file> <ip_pattern>"
    exit 1
fi

FILE="$1"
IP_PATTERN="$2"

if [ ! -f "$FILE" ]; then
    echo "Error: File '$FILE' not found!"
    exit 1
fi

REGEX=$(echo "$IP_PATTERN" | sed 's/\./\\./g' | sed 's/\*/[0-9]+/g')

echo "Searching for IP: $IP_PATTERN"
echo "--- Matched IPs ---"

grep -oE '([0-9]{1,3}\.){3}[0-9]{1,3}' "$FILE" | grep -E "^${REGEX}$" | sort -u

echo ""
echo "--- Lines containing matching IPs ---"
grep -E "$REGEX" "$FILE"
