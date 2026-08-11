#!/bin/bash
#
# run_helgrind.sh - lance valgrind --tool=helgrind sur une batterie
# de tests Codexion pour detecter les race conditions / mauvais
# usage des mutex. Ecrit tout dans un fichier rapport.
#
# Usage:
#   ./run_helgrind.sh /path/to/codexion [rapport.txt]
#
# NOTE: helgrind est beaucoup plus lent que memcheck (souvent 20-50x
# plus lent que l'execution normale). Les cas de test ici sont donc
# volontairement plus courts / avec moins de coders que le script
# memcheck, pour rester dans un temps raisonnable tout en gardant
# une vraie pression de concurrence sur les mutex/dongles.

set -u

BINARY="${1:-}"
REPORT="${2:-helgrind_report.txt}"
TIMEOUT=120

if [ -z "$BINARY" ] || [ ! -x "$BINARY" ]; then
    echo "Usage: $0 /path/to/codexion [rapport.txt]"
    exit 1
fi

if ! command -v valgrind >/dev/null 2>&1; then
    echo "valgrind n'est pas installe ou pas dans le PATH."
    exit 1
fi

# name;args
# Choisis expres des cas ou plusieurs coders se disputent le meme
# dongle (voisins proches, peu de dongles, cooldown court) pour
# maximiser les chances de declencher une vraie race si elle existe.
TESTS=(
    "TWO coders tight loop;2 5000 50 50 50 20 5 fifo"
    "THREE coders tight loop;3 5000 50 50 50 15 5 fifo"
    "FOUR coders fifo contention;4 5000 40 40 40 15 5 fifo"
    "FOUR coders edf contention;4 5000 40 40 40 15 5 edf"
    "FIVE coders high contention;5 4000 30 30 30 10 2 fifo"
    "EIGHT coders medium load;8 6000 60 60 60 8 10 edf"
    "SINGLE coder self dongle;1 3000 100 100 100 10 20 fifo"
    "BURNOUT race window;4 300 100 189 110 4 95 fifo"
    "ZERO cooldown max contention;4 3000 50 50 50 10 1 fifo"
)

: > "$REPORT"
{
    echo "Codexion - rapport valgrind --tool=helgrind (race conditions)"
    echo "Binaire  : $BINARY"
    echo "Date     : $(date)"
    echo "========================================================"
} >> "$REPORT"

total=0
clean=0
races=0
timeouts=0
other=0

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

    hg_out=$(mktemp)

    timeout "$TIMEOUT" valgrind \
        --tool=helgrind \
        --history-level=full \
        --conflict-cache-size=100000000 \
        $BINARY $args > /dev/null 2> "$hg_out"
    rc=$?

    n_errors=$(grep -oE "^==[0-9]+== ERROR SUMMARY: [0-9]+" "$hg_out" \
        | grep -oE "[0-9]+$" || echo "0")

    if [ $rc -eq 124 ]; then
        echo "STATUS: TIMEOUT after ${TIMEOUT}s" >> "$REPORT"
        timeouts=$((timeouts + 1))
    elif [ "$n_errors" != "0" ] && [ -n "$n_errors" ]; then
        echo "STATUS: RACE/LOCK ORDER ISSUE DETECTED ($n_errors errors)" >> "$REPORT"
        races=$((races + 1))
    elif [ $rc -ne 0 ] && grep -q "^Error:" "$hg_out"; then
        echo "STATUS: REJECTED (expected input validation error, code $rc)" >> "$REPORT"
        clean=$((clean + 1))
    elif [ $rc -ne 0 ]; then
        echo "STATUS: ABNORMAL EXIT (code $rc)" >> "$REPORT"
        other=$((other + 1))
    else
        echo "STATUS: CLEAN" >> "$REPORT"
        clean=$((clean + 1))
    fi

    echo "" >> "$REPORT"
    cat "$hg_out" >> "$REPORT"
    rm -f "$hg_out"
done

{
    echo ""
    echo "========================================================"
    echo "RESUME"
    echo "  Total tests : $total"
    echo "  CLEAN       : $clean"
    echo "  RACE/LOCK   : $races"
    echo "  TIMEOUT     : $timeouts"
    echo "  AUTRE       : $other"
    echo "========================================================"
} >> "$REPORT"

echo "Rapport ecrit dans : $REPORT"
echo "Resume : $clean clean / $races race-lock / $timeouts timeout / $other autre (total $total)"