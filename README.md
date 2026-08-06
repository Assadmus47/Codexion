*This project has been created as part of the 42 curriculum by mkacemi.*

# Codexion

## Description

Codexion is a C simulation of a classic concurrency problem — a variant of
Dijkstra's Dining Philosophers — reframed as coders sharing USB dongles to
compile quantum code. Each coder alternates between three states (compiling,
debugging, refactoring) and needs two dongles (shared with their two neighbors
in a circular co-working hub) to compile. The simulation must avoid deadlock,
respect a mandatory cooldown after each dongle release, arbitrate access
fairly according to a chosen scheduling policy (FIFO or EDF), and detect
coder "burnout" (starvation) with millisecond precision.

The project demonstrates thread creation and lifecycle management with
POSIX threads, mutex-protected shared resources, a hand-written binary
min-heap (no standard library priority queue), and a dedicated monitor
thread that watches every coder without interfering with their execution.

## Instructions

### Compilation

```bash
make
```

### Usage

```bash
./codexion number_of_coders time_to_burnout time_to_compile time_to_debug \
           time_to_refactor number_of_compiles_required dongle_cooldown scheduler
```

| Argument | Description |
|---|---|
| `number_of_coders` | Number of coders (and dongles) |
| `time_to_burnout` | Max time (ms) before a coder burns out if not compiling |
| `time_to_compile` | Time (ms) spent compiling |
| `time_to_debug` | Time (ms) spent debugging |
| `time_to_refactor` | Time (ms) spent refactoring |
| `number_of_compiles_required` | Stop condition: min compiles per coder |
| `dongle_cooldown` | Time (ms) a dongle stays unavailable after release |
| `scheduler` | `fifo` or `edf` |

### Example

```bash
./codexion 4 800 200 200 200 5 50 fifo
```

```
0 1 has taken a dongle
0 2 has taken a dongle
0 3 has taken a dongle
0 3 has taken a dongle
0 3 is compiling
205 3 is debugging
...
```

The simulation stops as soon as every coder has compiled at least
`number_of_compiles_required` times, or as soon as any coder burns out
(whichever happens first).

## Blocking cases handled

- **Deadlock prevention:** each coder always attempts to acquire its two
  neighboring dongles in a fixed, consistent order — the dongle with the
  lower `id` first, regardless of whether it is the coder's "left" or
  "right" dongle. This breaks the circular-wait condition (one of
  Coffman's four conditions for deadlock): it is no longer possible for
  every coder to simultaneously hold one dongle while waiting on another
  held by a neighbor, since two competing coders always contend for the
  same dongle first rather than two different ones.
- **Cooldown handling:** each dongle stores the timestamp of its last
  release. Before granting a dongle, `use_dongle()` checks that
  `current_time - dongle->timestamp >= dongle_cooldown` under the dongle's
  own mutex, so the check and the state change happen atomically.
- **Precise burnout detection:** a dedicated monitor thread polls every
  coder's `last_compile_start` timestamp every 5ms (well under the 10ms
  tolerance required by the subject) and immediately logs `burned out`
  and signals a stop as soon as a coder exceeds `time_to_burnout`.
- **Clean shutdown:** coders check a shared stop flag between every phase
  (after releasing dongles, after debugging, after refactoring) rather
  than only once per full loop, and also while busy-waiting for a
  dongle — so a burnout or a completed simulation stops every thread
  promptly instead of letting them finish an extra, stale cycle.
- **Log serialization:** all log output goes through `log_message()`,
  which locks a single shared mutex around the `printf` call, so two
  threads can never interleave partial lines. The timestamp is read
  *after* acquiring the lock, so log ordering always matches lock
  acquisition order.

> **Known limitation (to be improved):** dongle contention is currently
> resolved with a simple lock-and-retry loop rather than routing waiting
> coders through the binary heap scheduler. FIFO and EDF therefore compute
> different priority values (`get_priority()`), but this priority is not
> yet used to decide who is served first when a dongle frees up, and
> starvation is not yet formally prevented under high contention. This is
> the next planned change, using the heap together with a
> `pthread_cond_t` per dongle.

## Thread synchronization mechanisms

- **pthread_mutex_t usage:**
  - One mutex per dongle (`t_dongle.mutex`), protecting `is_taken` and the
    cooldown `timestamp`.
  - One mutex per coder (`t_coder.compile_mutex`), protecting
    `last_compile_start`, which is written by the coder's own thread and
    read concurrently by the monitor thread.
  - One global log mutex (`t_simulation.log_mutex`), serializing all
    terminal output.
  - One global flag mutex (`t_simulation.flag_mutex`), protecting the
    shared stop flag written by the monitor and read by every coder.
- **pthread_cond_t usage:** not used yet in the current implementation —
  waiting is done with a short `usleep`-based retry loop. Planned for the
  heap-based scheduling described above.
- **Race condition prevention (example):** without `compile_mutex`, the
  monitor thread could read `last_compile_start` at the exact moment a
  coder's thread is in the middle of writing it via
  `get_timestamp_ms()`, potentially reading a torn or stale value. Both
  the write (in `coder_routine`) and the read (in `monitor_routine`) are
  wrapped in `pthread_mutex_lock`/`unlock` on the same mutex to prevent
  this.
- **Thread-safe communication between coders and the monitor:** the
  monitor never touches coder state directly to stop the simulation —
  it only sets `sim->flag` under `flag_mutex`. Every coder thread
  independently polls this same flag (via `get_simulation_flag()`)
  between phases and reacts by breaking out of its loop, so no direct
  signaling or thread cancellation is needed.

## Resources

- Dijkstra's Dining Philosophers problem (classic reference for the
  underlying concurrency model)
- `man pthread_create`, `man pthread_mutex_init`, `man pthread_cond_wait`,
  `man gettimeofday`, `man usleep`
- Binary heap / priority queue: standard array-based min-heap
  (parent at `(i-1)/2`, children at `2i+1` / `2i+2`)

### AI usage

This project was built with Claude as a step-by-step guide, not as a code
generator:

- Claude explained concepts (dining philosophers analogy, mutexes,
  `gettimeofday`, binary heaps, deadlock/Coffman's conditions) with
  concrete examples *before* any code was written.
- All C code was written by hand by the author. Claude reviewed each
  function after it was written, pointed out Norm violations, logic bugs
  (e.g. an unprotected race on a burnout timestamp, a missing stop-check
  causing stale extra cycles after burnout), and edge cases (e.g. the
  `number_of_coders == 1` case).
- Claude did not design the overall architecture unprompted; each
  struct, function signature, and algorithm was proposed by the author
  first and corrected/refined through discussion.