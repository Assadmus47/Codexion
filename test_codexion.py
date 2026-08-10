#!/usr/bin/env python3
"""
Automated test harness for Codexion.

Usage:
    python3 test_codexion.py /path/to/codexion

For each test case, this script:
  - runs the binary with a generous timeout (flags infinite loops / hangs)
  - checks every log line matches the exact required format
  - replays each coder's own subsequence of events and checks it follows
    the expected state machine: take, take, compiling, debugging,
    refactoring, (repeat) - with an optional "burned out" that must be
    the LAST event for that coder
  - checks there is at most one "burned out" line in the whole run
  - if nobody burned out, checks every coder reached at least
    number_of_compiles_required compiles
"""

import re
import subprocess
import sys

LINE_RE = re.compile(
    r"^(\d+) (\d+) (has taken a dongle|is compiling|is debugging|is refactoring|burned out)$"
)

# (name, args_list)
TESTS = [
    ("HARD 25% - burnout expected sometimes",
     ["4", "1000", "100", "300", "200", "2", "400", "edf"]),
    ("HARD 35% - burnout expected sometimes",
     ["4", "1000", "300", "100", "100", "2", "200", "edf"]),
    ("HARD 16% - burnout expected sometimes",
     ["4", "400", "100", "189", "110", "4", "95", "fifo"]),

    ("MID - should pass",
     ["4", "557", "128", "222", "115", "10", "150", "edf"]),
    ("MID - should pass",
     ["4", "1000", "300", "100", "100", "2", "199", "edf"]),
    ("MID - should pass",
     ["4", "1000", "100", "300", "200", "2", "399", "edf"]),

    ("BASIC", ["4", "800", "200", "200", "200", "5", "10", "fifo"]),
    ("BASIC", ["4", "800", "200", "200", "200", "5", "10", "edf"]),
    ("BASIC", ["10", "10000", "100", "100", "100", "5", "40", "fifo"]),
    ("BASIC", ["20", "5000", "500", "500", "10", "100", "50", "edf"]),
    ("BASIC", ["5", "2000", "100", "100", "100", "20", "1", "fifo"]),
    ("BASIC", ["3", "10000", "2000", "2000", "2000", "2", "100", "fifo"]),
    ("BASIC", ["100", "10000", "66", "24", "87", "10", "10", "fifo"]),

    ("STARVE/EXIT", ["3", "1000", "600", "10", "10", "5", "100", "fifo"]),
    ("STARVE/EXIT", ["1", "1000", "200", "200", "200", "5", "50", "fifo"]),
    ("STARVE/EXIT", ["5", "1000", "200", "200", "200", "0", "10", "fifo"]),
    ("STARVE/EXIT", ["2", "1", "200", "200", "200", "5", "10", "fifo"]),
    ("STARVE/EXIT", ["2", "1000", "100", "100", "100", "5", "2000", "fifo"]),
]


def estimate_timeout(args):
    n, burnout, tc, td, tr, req, cooldown, sched = args
    tc, td, tr, req, cooldown = int(tc), int(td), int(tr), int(req), int(cooldown)
    cycle_ms = tc + td + tr + cooldown
    est_s = cycle_ms * (req + 4) / 1000.0 + 5
    return max(5, min(est_s, 45))


def validate_output(output, required_compiles):
    """Returns (ok: bool, reasons: list[str])"""
    reasons = []
    lines = [l for l in output.splitlines() if l.strip()]

    # 1. format check
    parsed = []
    for l in lines:
        m = LINE_RE.match(l.strip())
        if not m:
            reasons.append(f"malformed line: {l!r}")
            continue
        ts, cid, msg = m.groups()
        parsed.append((int(ts), int(cid), msg))

    if reasons:
        return False, reasons

    # 2. per-coder state machine
    coder_state = {}   # id -> state
    coder_takes = {}   # id -> take count in current cycle
    coder_compiles = {}  # id -> compile count
    coder_done = set()  # ids that burned out (terminal)
    burnout_count = 0

    STATE_TAKE1, STATE_TAKE2, STATE_COMPILE, STATE_DEBUG, STATE_REFACTOR = range(5)

    for ts, cid, msg in parsed:
        if cid in coder_done:
            reasons.append(f"coder {cid}: event {msg!r} after burned out (id already terminal)")
            continue

        state = coder_state.get(cid, STATE_TAKE1)

        if msg == "burned out":
            burnout_count += 1
            coder_done.add(cid)
            continue

        if msg == "has taken a dongle":
            if state == STATE_TAKE1:
                coder_state[cid] = STATE_TAKE2
            elif state == STATE_TAKE2:
                coder_state[cid] = STATE_COMPILE
            else:
                reasons.append(f"coder {cid}: unexpected 'has taken a dongle' in state {state}")
        elif msg == "is compiling":
            if state == STATE_COMPILE:
                coder_state[cid] = STATE_DEBUG
                coder_compiles[cid] = coder_compiles.get(cid, 0) + 1
            else:
                reasons.append(f"coder {cid}: unexpected 'is compiling' in state {state}")
        elif msg == "is debugging":
            if state == STATE_DEBUG:
                coder_state[cid] = STATE_REFACTOR
            else:
                reasons.append(f"coder {cid}: unexpected 'is debugging' in state {state}")
        elif msg == "is refactoring":
            if state == STATE_REFACTOR:
                coder_state[cid] = STATE_TAKE1
            else:
                reasons.append(f"coder {cid}: unexpected 'is refactoring' in state {state}")

    if burnout_count > 1:
        reasons.append(f"more than one 'burned out' line ({burnout_count})")

    if burnout_count == 0:
        for cid, n in coder_compiles.items():
            if n < required_compiles:
                reasons.append(
                    f"coder {cid} only compiled {n} times "
                    f"(required >= {required_compiles}) but no burnout occurred"
                )

    return (len(reasons) == 0), reasons


def run_test(binary, name, args):
    timeout = estimate_timeout(args)
    cmd = [binary] + args
    try:
        result = subprocess.run(
            cmd, capture_output=True, text=True, timeout=timeout
        )
    except subprocess.TimeoutExpired:
        return False, [f"TIMEOUT after {timeout:.1f}s (possible infinite loop/deadlock)"]

    if result.returncode < 0:
        return False, [f"CRASHED (signal {-result.returncode}) - stderr: {result.stderr.strip()[:300]}"]

    ok, reasons = validate_output(result.stdout, int(args[5]))
    return ok, reasons


def main():
    if len(sys.argv) != 2:
        print("Usage: python3 test_codexion.py /path/to/codexion")
        sys.exit(1)
    binary = sys.argv[1]

    passed = 0
    failed = 0
    for name, args in TESTS:
        args_str = " ".join(args)
        ok, reasons = run_test(binary, name, args)
        status = "PASS" if ok else "FAIL"
        print(f"[{status}] {name}: ./codexion {args_str}")
        if not ok:
            for r in reasons[:8]:
                print(f"        - {r}")
            if len(reasons) > 8:
                print(f"        ... and {len(reasons) - 8} more issues")
        if ok:
            passed += 1
        else:
            failed += 1

    print()
    print(f"Summary: {passed} passed, {failed} failed, {len(TESTS)} total")


if __name__ == "__main__":
    main()