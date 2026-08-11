#!/bin/bash

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[0;33m"
CYAN="\033[0;36m"
BOLD="\033[1m"
RESET="\033[0m"

PASS=0
FAIL=0
BINARY="./codexion"

print_header() {
	echo -e "\n${BOLD}${CYAN}══════════════════════════════════════════${RESET}"
	echo -e "${BOLD}${CYAN}  $1${RESET}"
	echo -e "${BOLD}${CYAN}══════════════════════════════════════════${RESET}\n"
}

print_section() {
	echo -e "\n${YELLOW}── $1 ──${RESET}"
}

# Strip ANSI color codes from output
strip_colors() {
	sed 's/\x1b\[[0-9;]*m//g'
}

# Test that expects an error on stderr + non-zero exit
test_error() {
	local desc="$1"
	shift
	local stderr_out
	stderr_out=$($@ 2>&1 1>/dev/null | strip_colors)
	local ret=${PIPESTATUS[0]}
	# Re-run to get exit code properly
	$@ >/dev/null 2>/dev/null
	ret=$?
	stderr_out=$($@ 2>&1 1>/dev/null | strip_colors)
	if [ $ret -ne 0 ] && echo "$stderr_out" | grep -qi "error\|failed"; then
		echo -e "  ${GREEN}✓${RESET} $desc"
		echo -e "      ${YELLOW}→ $(echo "$stderr_out" | head -1)${RESET}"
		((PASS++))
	elif [ $ret -ne 0 ]; then
		echo -e "  ${GREEN}✓${RESET} $desc (exit=$ret, no stderr message)"
		((PASS++))
	else
		echo -e "  ${RED}✗${RESET} $desc (expected error, got exit 0)"
		((FAIL++))
	fi
}

# Test that expects burnout (check stdout only)
test_burnout() {
	local desc="$1"
	shift
	local stdout_out
	stdout_out=$($@ 2>/dev/null | strip_colors)
	if echo "$stdout_out" | grep -q "burned out"; then
		echo -e "  ${GREEN}✓${RESET} $desc"
		((PASS++))
	else
		echo -e "  ${RED}✗${RESET} $desc (expected burnout, none found)"
		((FAIL++))
	fi
}

# Test that expects NO burnout (check stdout only)
test_no_burnout() {
	local desc="$1"
	shift
	local stdout_out
	stdout_out=$($@ 2>/dev/null | strip_colors)
	if echo "$stdout_out" | grep -q "burned out"; then
		echo -e "  ${RED}✗${RESET} $desc (unexpected burnout)"
		echo "    $(echo "$stdout_out" | grep "burned out")"
		((FAIL++))
	else
		echo -e "  ${GREEN}✓${RESET} $desc"
		((PASS++))
	fi
}

# Test format: timestamps increasing, no interleaving, correct format
# Only checks stdout log lines (filters out "End of simulation")
test_format() {
	local desc="$1"
	shift
	local output
	output=$($@ 2>/dev/null | strip_colors | grep -v "End of simulation")

	if [ -z "$output" ]; then
		echo -e "  ${RED}✗${RESET} $desc (no output)"
		((FAIL++))
		return
	fi

	# Check timestamps are non-decreasing
	local prev=0
	while IFS= read -r line; do
		local ts=$(echo "$line" | awk '{print $1}')
		if [[ "$ts" =~ ^[0-9]+$ ]]; then
			if [ "$ts" -lt "$prev" ]; then
				echo -e "  ${RED}✗${RESET} $desc (timestamp decreased: $prev -> $ts)"
				((FAIL++))
				return
			fi
			prev=$ts
		fi
	done <<<"$output"

	# Check no log message after burned out
	local found_burnout=0
	while IFS= read -r line; do
		if [ $found_burnout -eq 1 ]; then
			echo -e "  ${RED}✗${RESET} $desc (message after burnout: $line)"
			((FAIL++))
			return
		fi
		if echo "$line" | grep -q "burned out"; then
			found_burnout=1
		fi
	done <<<"$output"

	# Check format: each line matches "number number text"
	while IFS= read -r line; do
		if ! echo "$line" | grep -qE '^[0-9]+ [0-9]+ '; then
			echo -e "  ${RED}✗${RESET} $desc (bad format: $line)"
			((FAIL++))
			return
		fi
	done <<<"$output"

	# Check each "is compiling" is preceded by exactly 2 "has taken a dongle"
	# Track dongle takes per coder independently (handles interleaved logs)
	declare -A dongle_counts
	while IFS= read -r line; do
		local coder=$(echo "$line" | awk '{print $2}')
		local action=$(echo "$line" | cut -d' ' -f3-)
		if [ "$action" = "has taken a dongle" ]; then
			dongle_counts[$coder]=$((${dongle_counts[$coder]:-0} + 1))
		elif [ "$action" = "is compiling" ]; then
			local count=${dongle_counts[$coder]:-0}
			if [ "$count" -lt 2 ]; then
				echo -e "  ${RED}✗${RESET} $desc (compiling without 2 dongles: coder $coder, got $count)"
				((FAIL++))
				return
			fi
			dongle_counts[$coder]=0
		elif echo "$action" | grep -qE "is debugging|is refactoring|burned out"; then
			# Reset orphan dongle takes (retry mechanism)
			dongle_counts[$coder]=0
		fi
	done <<<"$output"

	echo -e "  ${GREEN}✓${RESET} $desc"
	((PASS++))
}

# Run a test N times and check consistency
test_stability() {
	local desc="$1"
	local runs="$2"
	shift 2
	local burnouts=0
	local clean=0
	for i in $(seq 1 $runs); do
		local output
		output=$($@ 2>/dev/null | strip_colors)
		if echo "$output" | grep -q "burned out"; then
			((burnouts++))
		else
			((clean++))
		fi
	done
	echo -e "  ${CYAN}○${RESET} $desc: ${burnouts}/${runs} burnouts, ${clean}/${runs} clean"
}

# ═══════════════════════════════════════════
#              PARSING TESTS
# ═══════════════════════════════════════════
print_header "PARSING / INVALID ARGUMENTS"

test_error "No arguments" $BINARY
test_error "Missing scheduler" $BINARY 4 800 200 100 100 5 50
test_error "Too many arguments" $BINARY 4 800 200 100 100 5 50 edf extra
test_error "Negative number" $BINARY -1 800 200 100 100 5 50 edf
test_error "Zero coders" $BINARY 0 800 200 100 100 5 50 edf
test_error "Invalid scheduler (rr)" $BINARY 4 800 200 100 100 5 50 rr
test_error "Non-integer (abc)" $BINARY abc 800 200 100 100 5 50 edf
test_error "Wrong case (FIFO)" $BINARY 4 800 200 100 100 5 50 FIFO
test_error "Int overflow" $BINARY 2147483648 800 200 100 100 5 50 edf
test_error "Zero compiles required" $BINARY 4 800 200 100 100 0 50 edf
test_error "Burnout at 0" $BINARY 4 0 200 100 100 5 50 edf
test_error "Wrong case (EDF)" $BINARY 4 800 200 100 100 5 50 EDF
test_error "Negative burnout" $BINARY 4 -800 200 100 100 5 50 edf
test_error "Negative compile" $BINARY 4 800 -200 100 100 5 50 edf
test_error "Empty string arg" $BINARY "" 800 200 100 100 5 50 edf

# ═══════════════════════════════════════════
#            EDGE CASES
# ═══════════════════════════════════════════
print_header "EDGE CASES"

test_burnout "1 coder (can never compile)" $BINARY 1 800 200 100 100 5 50 fifo
test_no_burnout "2 coders fifo" $BINARY 2 800 200 100 100 5 50 fifo
test_no_burnout "2 coders edf" $BINARY 2 800 200 100 100 5 50 edf

# ═══════════════════════════════════════════
#          EXPECTED BURNOUT
# ═══════════════════════════════════════════
print_header "EXPECTED BURNOUT"

test_burnout "4 coders, burnout=310 fifo" $BINARY 4 310 200 100 100 5 50 fifo
test_burnout "4 coders, burnout=200 edf" $BINARY 4 200 200 100 100 5 50 edf
test_burnout "5 coders (odd) fifo" $BINARY 5 400 200 100 100 5 50 fifo
test_burnout "3 coders edf tight" $BINARY 3 600 200 100 100 5 50 edf

# ═══════════════════════════════════════════
#         NO BURNOUT EXPECTED
# ═══════════════════════════════════════════
print_header "NO BURNOUT EXPECTED"

test_no_burnout "4 coders, burnout=2000 fifo" $BINARY 4 2000 200 100 100 5 50 fifo
test_no_burnout "4 coders, burnout=2000 edf" $BINARY 4 2000 200 100 100 5 50 edf
test_no_burnout "2 coders, easy fifo" $BINARY 2 1000 200 100 100 3 50 fifo
test_no_burnout "4 coders, cooldown=0 edf" $BINARY 4 800 200 100 100 5 0 edf

# ═══════════════════════════════════════════
#          DONGLE COOLDOWN
# ═══════════════════════════════════════════
print_header "DONGLE COOLDOWN"

test_no_burnout "2 coders, no cooldown fifo" $BINARY 2 800 200 100 100 5 0 fifo
test_no_burnout "2 coders, cooldown=200 fifo (generous)" $BINARY 2 1200 200 100 100 5 200 fifo
test_burnout "2 coders, cooldown=500 edf" $BINARY 2 800 200 100 100 5 500 edf
test_no_burnout "4 coders, cooldown=debug fifo" $BINARY 4 800 200 100 100 5 100 fifo
test_stability "2 coders, cooldown=200 fifo (tight)" 10 $BINARY 2 800 200 100 100 5 200 fifo

# ═══════════════════════════════════════════
#           FORMAT VALIDATION
# ═══════════════════════════════════════════
print_header "FORMAT VALIDATION"

test_format "Format check 4 coders edf" $BINARY 4 800 200 100 100 5 50 edf
test_format "Format check 2 coders fifo" $BINARY 2 1000 200 100 100 3 50 fifo
test_format "Format check 10 coders edf" $BINARY 10 2000 200 100 100 3 50 edf
test_format "Format check burnout case" $BINARY 4 310 200 100 100 5 50 fifo

# ═══════════════════════════════════════════
#         STRESS TESTS
# ═══════════════════════════════════════════
print_header "STRESS TESTS"

print_section "Short burnout stress (200 coders)"
test_stability "200 coders, burnout=182, edf" 5 $BINARY 200 182 60 60 60 20 0 edf
test_stability "200 coders, burnout=185, edf" 5 $BINARY 200 185 60 60 60 20 0 edf
test_stability "200 coders, burnout=190, edf" 5 $BINARY 200 190 60 60 60 20 0 edf
test_stability "200 coders, burnout=190, cd=5 edf" 5 $BINARY 200 190 60 60 60 20 5 edf
test_stability "200 coders, burnout=185, fifo" 5 $BINARY 200 185 60 60 60 20 0 fifo
test_stability "200 coders, burnout=190, fifo" 5 $BINARY 200 190 60 60 60 20 0 fifo
test_stability "200 coders, burnout=190, cd=5 fifo" 5 $BINARY 200 190 60 60 60 20 5 fifo

print_section "Tiny timings"
test_stability "4 coders, tiny times edf" 10 $BINARY 4 100 10 10 10 5 5 edf
test_stability "4 coders, tiny times fifo" 10 $BINARY 4 100 10 10 10 5 5 fifo

print_section "Large timings"
test_no_burnout "2 coders, large times fifo" $BINARY 2 10000 1000 1000 1000 2 100 fifo

print_section "Consistency (same test x20)"
test_stability "4 coders edf x20" 20 $BINARY 4 800 200 100 100 5 50 edf
test_stability "10 coders edf x10" 10 $BINARY 10 400 100 100 100 5 50 edf
test_stability "10 coders fifo x10" 10 $BINARY 10 400 100 100 100 5 50 fifo

# ═══════════════════════════════════════════
#         FIFO vs EDF COMPARISON
# ═══════════════════════════════════════════
print_header "FIFO vs EDF COMPARISON"

test_stability "5 coders, burnout=600 FIFO" 10 $BINARY 5 600 200 100 100 5 50 fifo
test_stability "5 coders, burnout=600 EDF" 10 $BINARY 5 600 200 100 100 5 50 edf
test_stability "10 coders, burnout=800 FIFO" 10 $BINARY 10 800 200 100 100 3 50 fifo
test_stability "10 coders, burnout=800 EDF" 10 $BINARY 10 800 200 100 100 3 50 edf

# ═══════════════════════════════════════════
#         SANITIZER TESTS
# ═══════════════════════════════════════════
print_header "SANITIZER TESTS"

print_section "ThreadSanitizer (recompiling...)"
make re CFLAGS="-Wall -Wextra -Werror -pthread -fsanitize=thread -g" >/dev/null 2>&1
if [ $? -eq 0 ]; then
	output=$($BINARY 4 8000 200 100 100 3 50 edf 2>&1)
	if echo "$output" | grep -qi "WARNING.*ThreadSanitizer\|data race"; then
		echo -e "  ${RED}✗${RESET} ThreadSanitizer: data race detected"
		echo "    $(echo "$output" | grep -i "WARNING\|data race" | head -3)"
		((FAIL++))
	else
		echo -e "  ${GREEN}✓${RESET} ThreadSanitizer: no data race"
		((PASS++))
	fi
else
	echo -e "  ${YELLOW}⚠${RESET} ThreadSanitizer: compilation failed"
fi

print_section "AddressSanitizer (recompiling...)"
make re CFLAGS="-Wall -Wextra -Werror -pthread -fsanitize=address -g" >/dev/null 2>&1
if [ $? -eq 0 ]; then
	output=$($BINARY 4 2000 200 100 100 3 50 edf 2>&1)
	if echo "$output" | grep -qi "ERROR.*Sanitizer\|LeakSanitizer\|AddressSanitizer"; then
		echo -e "  ${RED}✗${RESET} AddressSanitizer: issue detected"
		echo "    $(echo "$output" | grep -i "ERROR\|leak\|overflow" | head -3)"
		((FAIL++))
	else
		echo -e "  ${GREEN}✓${RESET} AddressSanitizer: clean"
		((PASS++))
	fi
else
	echo -e "  ${YELLOW}⚠${RESET} AddressSanitizer: compilation failed"
fi

# Recompile normally
print_section "Restoring normal build..."
make re >/dev/null 2>&1

# ═══════════════════════════════════════════
#              SUMMARY
# ═══════════════════════════════════════════
print_header "RESULTS"

TOTAL=$((PASS + FAIL))
echo -e "  ${GREEN}Passed: $PASS${RESET}"
echo -e "  ${RED}Failed: $FAIL${RESET}"
echo -e "  Total: $TOTAL"
echo ""

if [ $FAIL -eq 0 ]; then
	echo -e "  ${GREEN}${BOLD}All tests passed!${RESET}"
else
	echo -e "  ${RED}${BOLD}Some tests failed.${RESET}"
fi
echo ""