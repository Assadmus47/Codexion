#!/bin/bash
#
# run_valgrind.sh - lance valgrind (memcheck) sur une grosse batterie
# de tests Codexion et ecrit tout dans un fichier rapport.
#
# Usage:
#   ./run_valgrind.sh /path/to/codexion [rapport.txt]
#
# Pour chaque test, le rapport contient :
#   - la commande exacte utilisee
#   - le resume valgrind complet (leaks, erreurs, etc.)
#   - un statut LEAK/CLEAN/TIMEOUT/CRASH en tete de bloc pour scanner vite

set -u

BINARY="${1:-}"
REPORT="${2:-valgrind_report.txt}"
TIMEOUT=90

if [ -z "$BINARY" ] || [ ! -x "$BINARY" ]; then
    echo "Usage: $0 /path/to/codexion [rapport.txt]"
    exit 1
fi

if ! command -v valgrind >/dev/null 2>&1; then
    echo "valgrind n'est pas installe ou pas dans le PATH."
    exit 1
fi

# name;args (args separes par des espaces)
TESTS=(
    "BASIC fifo;4 800 200 200 200 5 10 fifo"
    "BASIC edf;4 800 200 200 200 5 10 edf"
    "BASIC 10 coders;10 10000 100 100 100 5 40 fifo"
    "BASIC 20 coders edf;20 5000 500 500 10 30 50 edf"
    "BASIC low cooldown;5 2000 100 100 100 20 1 fifo"
    "BASIC slow cycle;3 10000 2000 2000 2000 2 100 fifo"
    "STRESS 100 coders;100 10000 66 24 87 10 10 fifo"

    "MID pass edf;4 557 128 222 115 10 150 edf"
    "MID pass edf 2;4 1000 300 100 100 2 199 edf"
    "MID pass edf 3;4 1000 100 300 200 2 399 edf"

    "HARD burnout 25pct;4 1000 100 300 200 2 400 edf"
    "HARD burnout 35pct;4 1000 300 100 100 2 200 edf"
    "HARD burnout 16pct;4 400 100 189 110 4 95 fifo"

    "EDGE single coder;1 1000 200 200 200 5 50 fifo"
    "EDGE two coders;3 1000 600 10 10 5 100 fifo"
    "EDGE zero compiles required;5 1000 200 200 200 0 10 fifo"
    "EDGE burnout time = 1ms;2 1 200 200 200 5 10 fifo"
    "EDGE huge cooldown;2 1000 100 100 100 5 2000 fifo"
    "EDGE two coders minimal;2 500 50 50 50 3 5 fifo"
    "EDGE many coders zero compiles;20 5000 100 100 100 0 10 edf"
    "EDGE single coder edf;1 2000 200 200 200 10 30 edf"
)

: > "$REPORT"
{
    echo "Codexion - rapport valgrind (memcheck)"
    echo "Binaire  : $BINARY"
    echo "Date     : $(date)"
    echo "========================================================"
} >> "$REPORT"

total=0
leaks=0
clean=0
timeouts=0
crashes=0

for entry in "${TESTS[@]}"; do
    name="${entry%%;*}"
    args="${entry#*;}"
    total=$((total + 1))

    {
        echo ""
        echo "--------------------------------------------------------------"
        echo "TEST: $name"
        echo "CMD : $BINARY $args"
        echo "--------------------------------------------------------------"
    } >> "$REPORT"

    vg_out=$(mktemp)

    timeout "$TIMEOUT" valgrind \
        --leak-check=full \
        --show-leak-kinds=all \
        --track-origins=yes \
        --errors-for-leak-kinds=all \
        --error-exitcode=99 \
        $BINARY $args > /dev/null 2> "$vg_out"
    rc=$?

    if [ $rc -eq 124 ]; then
        echo "STATUS: TIMEOUT after ${TIMEOUT}s (deadlock/infinite loop suspecte)" >> "$REPORT"
        timeouts=$((timeouts + 1))
    elif [ $rc -eq 99 ]; then
        echo "STATUS: VALGRIND ERROR DETECTED" >> "$REPORT"
        leaks=$((leaks + 1))
    else
        # Meme sans --error-exitcode declenche, on verifie quand meme
        # le resume des leaks au cas ou (definitely/indirectly lost).
        if grep -qE "definitely lost: [1-9]|indirectly lost: [1-9]" "$vg_out"; then
            echo "STATUS: LEAK DETECTED (definitely/indirectly lost > 0)" >> "$REPORT"
            leaks=$((leaks + 1))
        elif [ $rc -ne 0 ] && grep -q "^Error:" "$vg_out"; then
            echo "STATUS: REJECTED (expected input validation error, code $rc)" >> "$REPORT"
            clean=$((clean + 1))
        elif [ $rc -ne 0 ]; then
            echo "STATUS: CRASH/ABNORMAL EXIT (code $rc)" >> "$REPORT"
            crashes=$((crashes + 1))
        else
            echo "STATUS: CLEAN" >> "$REPORT"
            clean=$((clean + 1))
        fi
    fi

    echo "" >> "$REPORT"
    cat "$vg_out" >> "$REPORT"
    rm -f "$vg_out"
done

{
    echo ""
    echo "========================================================"
    echo "RESUME"
    echo "  Total tests : $total"
    echo "  CLEAN       : $clean"
    echo "  LEAK/ERROR  : $leaks"
    echo "  TIMEOUT     : $timeouts"
    echo "  CRASH       : $crashes"
    echo "========================================================"
} >> "$REPORT"

echo "Rapport ecrit dans : $REPORT"
echo "Resume : $clean clean / $leaks leak-error / $timeouts timeout / $crashes crash (total $total)"