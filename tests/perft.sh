#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'  # Reset color

echo "This test uses perftsuite.epd obtained from http://www.rocechess.ch/perft.html supplemented with some deeper perft values and more positions."
echo "Testing perft for windmolen..."

ENGINE="../src/windmolen"
EPD_FILE="perftsuite.epd"

total=0
passed=0

# Record total start time
TOTAL_START=$(date +%s%N)

while IFS= read -r line; do
    ((total++))

    # Extract FEN (everything before first ';')
    FEN=$(echo "$line" | cut -d';' -f1 | xargs)

    # Extract all "D#" entries (e.g. "D1 20; D2 400; ...")
    DVALUES=$(echo "$line" | grep -o "D[0-9] [0-9]*")

    PASS=true
    FAIL_DEPTH=0
    FAIL_EXPECTED=0
    FAIL_ACTUAL=0

    echo -n "Testing FEN: \"$FEN\"... "

    # Start per-position timer
    POS_START=$(date +%s%N)

    # Start engine for this FEN
    coproc ENGINEPROC ( "$ENGINE" )

    # Send UCI initialization commands
    echo "uci" >&"${ENGINEPROC[1]}"
    echo "isready" >&"${ENGINEPROC[1]}"

    # Wait for "readyok"
    while read -r REPLY <&"${ENGINEPROC[0]}"; do
        [[ "$REPLY" == "readyok" ]] && break
    done

    while read -r DVAL; do
        [ -z "$DVAL" ] && continue  # Skip empty lines

        DEPTH=$(echo "$DVAL" | awk '{print substr($1,2)}')
        EXPECTED=$(echo "$DVAL" | awk '{print $2}')

        # Send perft command
        echo "position fen $FEN" >&"${ENGINEPROC[1]}"
        echo "go perft $DEPTH" >&"${ENGINEPROC[1]}"

        # Wait for engine output line containing "Nodes searched:"
        ACTUAL=""
        while read -r LINE <&"${ENGINEPROC[0]}"; do
            if [[ "$LINE" == Nodes\ searched:* ]]; then
                ACTUAL=$(echo "$LINE" | awk '{print $3}')
                break
            fi
        done

        if [ "$ACTUAL" != "$EXPECTED" ]; then
            PASS=false
            FAIL_DEPTH=$DEPTH
            FAIL_EXPECTED=$EXPECTED
            FAIL_ACTUAL=$ACTUAL
            break
        fi
    done <<< "$DVALUES"

    # Quit engine
    echo "quit" >&"${ENGINEPROC[1]}"
    exec {ENGINEPROC[1]}>&-
    exec {ENGINEPROC[0]}<&-

    POS_END=$(date +%s%N)
    ELAPSED=$((POS_END - POS_START))
    ELAPSED_MS=$((ELAPSED / 1000000))  # Convert to milliseconds

    if [ "$PASS" = true ]; then
        echo -e "${GREEN}passed${NC} (${ELAPSED_MS} ms)"
        ((passed++))
    else
        echo -e "${RED}failed${NC} (${ELAPSED_MS} ms)"
        echo "Incorrect value at depth $FAIL_DEPTH: expected $FAIL_EXPECTED, got $FAIL_ACTUAL"
    fi

done < "$EPD_FILE"

# Record total end time
TOTAL_END=$(date +%s%N)
TOTAL_ELAPSED=$((TOTAL_END - TOTAL_START))
TOTAL_ELAPSED_MS=$((TOTAL_ELAPSED / 1000000))  # Convert to milliseconds

echo
if [ "$passed" = "$total" ]; then
    echo -e "Passed ${GREEN}$passed${NC}/$total (${TOTAL_ELAPSED} ms)"
else
    echo -e "Passed ${RED}$passed${NC}/$total (${TOTAL_ELAPSED} ms)"
fi
