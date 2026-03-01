# Izalloc — A Custom Memory Allocator

A dependency-free, drop-in compatible memory allocator written in C. Implements `ft_alloc`, `ft_free`, and `ft_realloc` (following the standard 42 School nomenclature) using a static memory pool and an external sorted metadata structure. No OS calls at runtime, no external libraries, no headers beyond its own, following `The Norm` and passing the `Norminette`.

> **Context.** This is a personal project I made 2–3 months after I started learning to program. My only prior experience was Excel, it helped more than I expected, but there is still A LOT I need to learn and improve form here.

---

## Table of Contents

- [**Origin**](#origin)
- [**Design & Reflection**](#design--reflection)
  - [Design Evolution](#design-evolution)
  - [Allocation Strategies — Best-Fit vs First-Fit](#allocation-strategies--best-fit-vs-first-fit)
  - [Instincts That Proved Right](#instincts-that-proved-right)
  - [What I Learned Along the Way](#what-i-learned-along-the-way)
  - [Known Limitations](#known-limitations)
  - [What I Would Do Differently](#what-i-would-do-differently)
- [**Understanding izalloc**](#understanding-izalloc)
  - [How Memory Allocation Works](#how-memory-allocation-works)
  - [A Simple Visual Guide](#a-simple-visual-guide)
  - [Glossary](#glossary)
- [**Reference**](#reference)
  - [API Reference](#api-reference)
  - [Usage Examples](#usage-examples)
  - [Compatibility](#compatibility)
  - [How It Compares to System malloc](#how-it-compares-to-system-malloc)
  - [Configuration](#configuration)
  - [File Structure](#file-structure)
  - [Building](#building)
- [**Project History & Future**](#project-history--future)
  - [Changelog](#changelog)
  - [Roadmap](#roadmap)

---
---

# Origin

In 42 school there can't be any memory leaks, ever, under any condition. One leak means instant failure on any project. No exceptions.

I started thinking almost as a joke about workarounds, maybe make a function that stores every pointer allocated, then free them all in a cleanup function at the end of the program. After one of the first projects I learned about static variables, and an idea came to mind. That idea became this project.

So I traded 15 minutes of debugging for almost a month of learning and building something on the fly that I didn't fully understand at first. I wasn't really trying to reinvent the wheel, I wanted to pick it apart to understand how it worked. I ended up with a hexagon that rolls better than I expected when I first started.

Making it harder: this was written under a strict set of rules defined by 42 School, as I intended to use this allocator in future projects and sharing it with any other student who is willing to give it a try.

- No external libraries of any kind, unless explicitly allowed.
- Maximum 5 functions per file, 25 lines per function, 80 characters per line.
- Only `while` loops — no `for`, no `do while`, no `switch`.
- Variables must be declared and initialized on separate lines.
- Maximum 4 arguments per function.
- No global variables.

These draconian guidelines forced some out-of-the-box thinking. I knew structures would have made this simpler, but I hadn't used them yet, and I wanted to build it "at my level", make something that worked, even if it was a collection of permanent temporary patches. Later you will see the workaround I found: pseudo-structures built from arrays by recasting types in weird but clever ways (or so I try to convince myself).

Several design choices were arrived at from first principles before I later discovered they are established techniques with names. I would research on the side as I was developing the project, and then find out that what I had done already existed, but it was important to me not to spoil myself too much before progressing. I wanted to do something I could have done even if I were the first person to ever try it, not just follow a "build your own X" tutorial.

---
---

# Design & Reflection

This section is a small window into my process. I started without knowing exactly how things would turn out, what mattered was writing something first; I could always make it work later. The starting point was knowing I could use a static string as a memory pool and have it persist between calls. Soon I realized I needed to track used and unused regions, and I tried to trust my gut... then refactor when it wasn't working out.

## Design Evolution

### The Road Not Taken: Three-Field Metadata

The first idea was to have two equal strings: one with the actual data, and another with boolean flags on each byte signaling whether it was used or unused. It never made it to code, as it was obviously wasteful to double the size of all data just to track allocations.
Before settling on the current approach, an earlier design tracked three values per block: a flag for whether it was free or occupied, and its start and end. Every block, active or inactive, would always have an entry in the list. That felt clunky too, but was miles ahead of the first option I discarded.

```
 Early design (abandoned):
 [ FREE | 0→49 ]  [ USED | 50→149 ]  [ FREE | 150→299 ]  [ USED | 300→399 ]
```
This option would have brought a lot of bugs to the table, but I only realized that later, because I came up with a simpler option that fixed them all before they even appeared. One of those cases where simplicity doesn't get paid for later, but instead ripples down through the whole architecture. It was a bit of instinct and a bit of luck that I dodged this bullet.

The switch to tracking only *living* blocks was made before the three-field version was even fully implemented. Removing an entry entirely rather than marking it inactive meant free space was never fragmented in the metadata, coalescing became impossible to get wrong because it was no longer a concept, and the binary search I had planned became more straightforward. The atlas got smaller and simpler as the design improved, rather than the reverse — sometimes less is more.

### `[start, end]` vs `[start, size]`

An earlier version stored pairs as `[start, size]`. When implementing the binary search for free, the comparison at each step required computing `start + size - 1` to recover the end address. Switching to `[start, end]` made the range comparison direct and the logic noticeably cleaner throughout, it was also easier to debug and test the functions that worked over `atlas`.

### The Counter

The `counter` variable tracking active allocations was added mid-development after several code paths needed to know whether the atlas was empty, or how many partitions were alive at any moment. Without it, checking meant scanning. With it, checking is a single comparison. It costs one integer of storage and one update per operation. Obvious in retrospect; not obvious at the start, without structures, needing to track yet another static variable meant I was approaching a dangerous point with the 4-argument-per-function restriction.

### Alignment as an Afterthought

16-byte alignment was not part of the original design. It was added after research into how other allocators work revealed that returning arbitrary addresses can cause crashes or silent data corruption when used with certain data types. The fix was straightforward, but retrofitting it meant touching two separate functions. It should have been a first-class concern from the beginning, but that is exactly why I started the project.

### `ft_realloc` as an Emergent Feature

`ft_realloc` was not planned from the start. The original scope was just alloc and free. Realloc was going to be left for later and handled as a function that simply calls alloc and free, without touching the memory manager I had built. What changed was realizing mid-development that the atlas already held everything needed to check whether a block could grow in place: the end of the current block and the start of the next one are right there. In-place reallocation was a happy accident, I had read about it and was planning to implement it later, but it turned out I didn't need to change much about the structure itself to get it working. Not that it was without challenges: I had to build a pseudo-structure to pass everything needed without breaking the 4-argument rule. But I consider it a win on its own, I now know how to reimplement structures from scratch, just by casting variables back and forth between types and passing them as a pointer. It makes me feel more prepared to use real structures effectively in the future.
`ft_realloc` also turns some of this allocator's weak points into features to take advantage of, like the fact that freed memory blocks remain accessible, which enables a cleaner implementation and even a small edge-case optimization (I think you can tell I'm a bit too proud of this one — let me move on).

---

## Allocation Strategies: Best-Fit vs First-Fit

The allocator currently uses **best-fit**: find the smallest gap that satisfies the request. A planned alternative is **first-fit**: find the first gap large enough, regardless of size. This way you can choose what to build into your project depending on what you need — optimized processing, or a more efficient memory pool. It would also allow me to actually benchmark how much of a trade-off one is compared to the other.
I decided to implement best-fit first because it looked more interesting, and I valued efficient use of the memory pool more — that is one of the more limiting factors when comparing my allocator to typical heap allocators like malloc or jemalloc.

```
 Where they diverge:

 ┌──────┬────────────────────┬──────┬──────────────┬──────┐
 │  A   │     gap: 400b      │  B   │  gap: 100b   │  C   │
 └──────┴────────────────────┴──────┴──────────────┴──────┘

 Request: ft_alloc(80)

 First-fit:  takes the 400b gap immediately (first one found)
             → 320b wasted from the large gap ✗

 Best-fit:   scans all gaps, picks the 100b gap
             → 20b wasted, 400b gap preserved ✓
```

Best-fit is slower per call, it must scan all gaps before deciding. First-fit is faster per call but can burn large gaps unnecessarily. The right choice depends on the workload: programs that make thousands of small allocations per second may prefer first-fit; programs with long-lived mixed-size allocations are better served by best-fit. Some choices aren't better or worse — just different trade-offs.

---

## Instincts That Proved Right

Several decisions I made early, some intuitively, some by necessity, align with how real allocators are designed. Some of them surprised me: I came up with them independently, but they turned out to be how things are actually built even today.

**Tracking allocated blocks, not free ones.** The most consequential design choice in the project. It eliminates an entire category of maintenance by making free space structurally implicit. This was not taken from a reference; it emerged from thinking about what actually needs to be recorded. It turned out to be a recognized pattern.

**Keeping the atlas sorted at all times.** Lazy sorting was considered and rejected early: it would add a dirty flag, a check before every binary search, and yet another invariant to maintain. Eager sorting means an O(n) shift on every insert and delete, but makes every other operation simpler and more predictable. The tradeoff is real and worth it at any practical partition count.

**Best-fit with edge priority.** Checking pool edges before interior gaps to preserve large interior regions is sound allocation strategy. It was derived from a physical intuition, the mental model of packing a storage unit. At first I wanted to compact everything whenever I ran out of available blocks but still had enough total free space; that proved impossible without breaking everything the system was trying to do. So minimizing fragmentation was the way to go.

**Returning NULL from `ft_free`.** Designed with a specific use case in mind, clearing the pointer on the same line that frees the block, and it turned out to be genuinely useful. It doesn't break any standard usage pattern, and when you only have 25 lines to work with, every one counts. I later found out I wasn't alone: other implementations do similar things.

**`ft_realloc` as an emergent feature.** The function was not planned, realloc only got built when the atlas structure made in-place growth an obvious thing to check for first. The right architecture created the right opportunity, and the feature followed naturally from the data that was already there.

---

## What I Learned Along the Way

**Memory alignment** was entirely unknown to me when development started. It was added late as a patch after reading about how other allocators handle it. Without alignment, the allocator could return addresses that cause undefined behaviour or crashes when used with types like `double` or SIMD vectors on architectures that enforce aligned access. It was a bug I avoided by doing some research, even before I finished the program and started testing.

**`ft_realloc` was not on the roadmap.** Before noticing that the atlas made in-place growth trivial to check, the plan was to leave resizing to the caller entirely. The architecture revealed the feature — this was both planned and unplanned in a sense, as early decisions made a whole class of problems disappear before they could arise.

**The counter variable** should have been there from day one. The cost is trivial and the benefit was huge. Simplicity isn't just fewer lines, variables, or functions, sometimes more is more too.

**Coalescing is a solved problem if you never create it.** Moving away from the three-field design eliminated an entire class of correctness concerns before they could appear. Getting to the better idea before fully implementing the first one was fortunate.

---

## Known Limitations

**Bounded pool.** Cannot grow beyond `MAX_MEMORY`. This is by design, the static pool intentionally avoids OS-level memory calls.

**No memory zeroing.** Freed blocks retain their contents until overwritten. Do not treat freed memory as inaccessible or zeroed. This is also true of standard `free`, but here the error is less obvious.

**Not thread-safe.** The pool and atlas are shared state with no synchronization. Concurrent access will produce incorrect behaviour. This cannot be fixed without additional libraries, but a thread-safe version is planned for when it's needed.

**Fragmentation is possible.** Best-fit reduces fragmentation significantly but cannot eliminate it. A heavily fragmented pool may fail to satisfy a large request even when total free bytes would be sufficient, there is no good way to solve this.

---

## What I Would Do Differently

**A context struct.** A `t_allocator` struct passed by pointer would replace pool, atlas, and counter as separate arguments throughout. It solves the 4-argument limit, removes the (somewhat ugly and obfuscating, if clever) packing workaround, and makes the whole thing readable without needing the constraint context to understand it. This is the most important item on the refactor list — but I wanted to build something with the tools I already had, just to see if I could.

**One binary search.** `ft_seek` in `free.c` and `ft_search` in `reallo.c` were written at different points in development and do the same thing. They should be one shared function.

**`ssize_t` instead of `unsigned long long`.** Chosen partly for line length and partly because the required headers were off-limits. Signed returns are more idiomatic for error-signalling. Without the constraints, that would have been the quickest improvement for code cleanliness and making some of the logic easier to read.

**Alignment from day one.** A concept I don't think I would have heard of if not for this project, where I had to fine-tune memory usage beyond anything I'd done in the simpler functions from my first 42 School project.

---
---

# Understanding izalloc

## How Memory Allocation Works

> *This section is for anyone unfamiliar with memory management. I feel like an expert now, but I'm not — so don't take this as gospel. I'm just explaining why memory management is both more complex and easier than I first gave it credit for.*

When a program runs, it needs somewhere to physically store information, maybe a list of names, a level in a game, a block of text the user typed. That storage has to be reserved before use and returned when it is no longer needed. In C, this is done manually.

Think of a program's memory as a very long line of empty cells (like... in Excel). Every byte is blank when the program starts. When you need to store something, you pick a region of the line, draw a cell around it, write your data inside, and remember where the cell was. When you are done with it, you erase the cell and the space is available again, in reality you just mark it as available; the data is still there until overwritten.

The standard tool for this in C is `malloc`. You ask it for a chunk of memory, it finds a free region and hands you a number, a **pointer** representing where in memory your data now lives. When you are done, you call `free` to give it back. If you forget, that region stays reserved for the program, unavailable for anything else. That is a **memory leak**.

`izalloc` reimplements that entire system from scratch, inside a single fixed array, without asking the operating system for anything at runtime. It's really just pretending to be malloc, it subdivides what it already has rather than asking the OS for more whenever it needs space. You can close it at any moment and be debt-free. No leaks.

---

## A Simple Visual Guide

### The Pool

The pool is a long strip of memory. At program start, all of it is free.

```
 ┌────────────────────────────────────────────────────────────┐
 │                         FREE                               │
 └────────────────────────────────────────────────────────────┘
```

When something is allocated, a region gets reserved. When freed, that region opens back up.

---

### After a Few Allocations

```
 ft_alloc(A)  →  ft_alloc(B)  →  ft_alloc(C)
 (with some space between them due to alignment)

 ┌──────────┬────────┬──────────┬────────────────┬──────────┐
 │  BLOCK A │  free  │  BLOCK B │      free      │  BLOCK C │
 └──────────┴────────┴──────────┴────────────────┴──────────┘
```

The allocator records only what is occupied, in a sorted list called the **atlas**:

```
 Atlas:  [ A ]  [ B ]  [ C ]
          ↑       ↑      ↑
        start→end pairs, in address order
```

Free space is never written down. It is just the gaps between atlas entries. This means there is never a separate step to merge free regions, the moment a block is removed from the atlas, the gap it leaves is automatically as large as everything adjacent to it.

---

### Freeing a Block

```
 ft_free( B )

 BEFORE:
 ┌──────────┬────────┬──────────┬────────────────┬──────────┐
 │  BLOCK A │  free  │  BLOCK B │      free      │  BLOCK C │
 └──────────┴────────┴──────────┴────────────────┴──────────┘
 Atlas:  [ A ]  [ B ]  [ C ]

 AFTER:
 ┌──────────┬──────────────────────────────────┬──────────┐
 │  BLOCK A │              free                │  BLOCK C │
 └──────────┴──────────────────────────────────┴──────────┘
 Atlas:  [ A ]  [ C ]
```

The two gaps that existed around B became one larger gap the instant B's entry was removed. No merging step, removing the entry is all it takes. The gap simply is whatever lies between A and C.

---

### Allocating into a Gap: Best-Fit

When a new allocation is requested, the allocator finds the *smallest* gap that fits. This preserves larger gaps for future larger requests.

```
 State with two available gaps:
 ┌──────────┬────────────┬──────────┬──────────────────────┐
 │  BLOCK A │  gap: 20b  │  BLOCK B │      gap: 200b       │
 └──────────┴────────────┴──────────┴──────────────────────┘

 Request: ft_alloc(15)

   gap of  20b → fits ✓  (smaller, chosen)
   gap of 200b → fits ✓  (skipped — wasteful for 15 bytes)

 Result:
 ┌──────────┬───────────┬───┬──────────┬──────────────────────┐
 │  BLOCK A │  NEW (15b)│5b │  BLOCK B │      gap: 200b       │
 └──────────┴───────────┴───┴──────────┴──────────────────────┘

 5 bytes remain from the 20b gap — a small, accepted cost.
 The 200b gap is untouched and ready for something that needs it.
```

---

### Edge Priority: Why the Ends of the Pool Get Checked First

Interior gaps tend to be small and numerous. The regions before the first block and after the last one tend to grow large over time. The allocator checks these edge regions first, giving them priority when they offer the best fit.

```
 ┌──────────┬──────────┬──────────┬──────────┬──────────────┐
 │  edge    │  BLOCK A │  gap     │  BLOCK B │  edge        │
 │  free    │          │ (small)  │          │  (large)     │
 └──────────┴──────────┴──────────┴──────────┴──────────────┘
     ↑                                              ↑
  checked first                             checked first
```

Filling small gaps first preserves contiguous edge space for future large allocations that might not fit anywhere else.

---

### Growing a Block Without Moving It

```
 ft_realloc(A, larger_size)
 Block A needs more room. There is free space directly after it.

 BEFORE:
 ┌──────────────┬──────────────────────┬──────────┐
 │   BLOCK A    │         free         │  BLOCK B │
 └──────────────┴──────────────────────┴──────────┘

 The free space after A is large enough. Only the atlas is updated.

 AFTER:
 ┌───────────────────────────────┬───────┬──────────┐
 │         BLOCK A (grown)       │ free  │  BLOCK B │
 └───────────────────────────────┴───────┴──────────┘

 No data was copied. Cost: one number changed in the atlas.
```

---

### Growing a Block That Has No Room

```
 ft_realloc(A, much_larger_size)
 The free space after A is not large enough.

 BEFORE:
 ┌──────────────┬──────┬──────────┬──────────────────┐
 │   BLOCK A    │ free │  BLOCK B │       free       │
 └──────────────┴──────┴──────────┴──────────────────┘

 Step 1: free A. Its space merges with the gap after it.
 ┌────────────────────┬──────────┬──────────────────┐
 │       free         │  BLOCK B │       free       │
 └────────────────────┴──────────┴──────────────────┘

 Step 2: find the best fitting gap for the new size. Found after B.
 Step 3: copy A's original data there.
           Safe: freed memory keeps its contents until overwritten.

 AFTER:
 ┌────────────────────┬──────────┬──────────────────────────┐
 │       free         │  BLOCK B │     BLOCK A (moved)      │
 └────────────────────┴──────────┴──────────────────────────┘
```

---

### Shrinking a Block

```
 ft_realloc(A, smaller_size)

 BEFORE:
 ┌────────────────────────────┬──────────┐
 │          BLOCK A           │  BLOCK B │
 └────────────────────────────┴──────────┘

 AFTER:
 ┌──────────────┬─────────────┬──────────┐
 │   BLOCK A    │    free     │  BLOCK B │
 │ (new size)   │ (released)  │          │
 └──────────────┴─────────────┴──────────┘

 The tail of A is simply released. No data is moved or copied.
```

---

### Fragmentation and Recovery

Over time, many allocs and frees of varying sizes can leave the pool scattered with small gaps. This is called **fragmentation**.

```
 Heavily fragmented pool:
 ┌────┬──┬────┬──┬────┬────┬────┬──┬──┬────┬──┬────────────┐
 │ A  │  │ B  │  │ C  │ D  │ E  │  │  │ F  │  │    free    │
 └────┴──┴────┴──┴────┴────┴────┴──┴──┴────┴──┴────────────┘
      ↑       ↑       (many small gaps)

 A large request may still succeed if enough space exists at the edge.
 As allocations are freed, larger gaps reappear naturally.
 No defragmentation step is available for this.
```

Best-fit reduces fragmentation but does not eliminate it. It is the fundamental tradeoff of any fixed-pool allocator, space usage can never be 100% efficient.

---

### What Happens at the Limit

```
 ft_alloc called when no gap is large enough:

 ┌──────────┬──────────┬──────────┬──────────┬──────────────┐
 │  BLOCK A │  BLOCK B │  BLOCK C │  BLOCK D │  BLOCK E ... │
 └──────────┴──────────┴──────────┴──────────┴──────────────┘

 Returns NULL. No crash. The caller is responsible for checking.
```

NULL is the standard and safe response to an exhausted pool. The allocator never invents space it does not have, that would lead to overlapping pointers and corrupted memory.

---

## Glossary

> *For readers new to systems programming.*

**Pointer** — a number that represents a location in memory. When you allocate memory, you receive a pointer telling you where your data lives.

**Memory leak** — forgetting to free memory that is no longer needed. The memory stays reserved when the program ends, unavailable to anything else. In long-running programs, leaks accumulate and can exhaust available memory entirely.

**Fragmentation** — a state where total free memory is sufficient for a request, but no single contiguous region is large enough to satisfy it. Like trying to fit a large box into storage where the available space is scattered across many small gaps. Reorganizing the shelf so the big box could fit would be defragmentation, but then you would need to tell everyone where their things have moved.

**Coalescing** — the process of merging adjacent free regions into one larger region. Necessary in allocators that track free blocks explicitly. Not needed here because free space is never tracked as a separate thing.

**Binary search** — a fast way to find an item in a sorted list. Instead of checking every entry, it repeatedly cuts the search space in half. Finding one entry among 1024 takes at most 10 comparisons instead of up to 1024.

**Alignment** — the requirement that data be stored at memory addresses that are multiples of a given number. A 4-byte integer typically needs to live at an address divisible by 4. Violating alignment can cause crashes or silent data corruption on some hardware.

**Static array** — an array whose size is fixed at compile time and whose memory is reserved when the program starts. Unlike dynamically-allocated memory, it does not need to be requested from the operating system at runtime.

**Sentinel value** — a special value used to signal a specific condition, often "not found" or "end of data". Choosing a sentinel that cannot appear as a valid result is important for correctness.

**Atlas** — the name used in this project for the sorted metadata array that tracks active allocations. Not standard terminology; named for its role as a map of what regions are occupied.

---
---

# Reference

## API Reference

```c
void *ft_alloc(t_ull size);
void *ft_free(void *pointer);
void *ft_realloc(void *pointer, t_ull size);
```

### `ft_alloc`

Returns a pointer to at least `size` bytes of usable memory, or NULL if the pool is exhausted or `MAX_PARTITIONS` is reached. Returned addresses are always aligned to 16-byte boundaries.

### `ft_free`

Frees the block containing the given pointer. Differences from standard `free`:

- **Returns NULL** — enables `ptr = ft_free(ptr)` to free and clear in one line, eliminating dangling pointer risk
- **Accepts any pointer inside the block** — if a pointer has been incremented past the allocation start, it still works
- **Double-free, NULL, and invalid pointers are silent no-ops** — no crash, no undefined behaviour

### `ft_realloc`

Resizes a block. `size == 0` behaves as free. `pointer == NULL` behaves as alloc. Attempts in-place growth first; falls back to free-then-reallocate with a safe copy if the adjacent gap is too small.

---

## Usage Examples

Basic allocation and free:

```c
char *str = ft_alloc(64);
if (str == NULL)
    return ; // pool exhausted or partition limit reached
// ... use str ...
str = ft_free(str); // str is now NULL — freed and cleared in one line
```

Growing a buffer:

```c
int *arr = ft_alloc(10 * sizeof(int));
// later, need more room:
arr = ft_realloc(arr, 20 * sizeof(int));
if (arr == NULL)
    return ; // realloc failed
```

Allocating zeroed memory (until `ft_calloc` is implemented):

```c
void *block = ft_alloc(size);
if (block)
    memset(block, 0, size);
```

No cleanup needed at program exit. The pool is a static array, it is released automatically when the program ends. This is the property the project was originally built to guarantee. Freeing memory when it's no longer needed is still recommended, as it helps keep the memory pool from running out of space and builds good habits. But no errors will occur even if you skip a final cleanup before exiting — you can call `exit` whenever you want.

---

## Compatibility

Any code that runs correctly with standard `malloc` / `free` / `realloc` will run correctly with `ft_alloc` / `ft_free` / `ft_realloc`, provided `MAX_MEMORY` and `MAX_PARTITIONS` are not exceeded. The extended behaviours of `ft_free` are purely additive — using it exactly like standard `free` produces identical results.

---

## How It Compares to System malloc

The system allocator (glibc's `malloc` on Linux, for example) is a vastly more complex piece of software. Some differences worth knowing:

**Pool source.** The system allocator requests memory from the OS using `sbrk` or `mmap` and can grow as needed. `izalloc` works entirely within a fixed static array baked into the binary at compile time.

**Metadata location.** Most system allocators embed metadata headers directly before each allocation in the same memory region. `izalloc` keeps all metadata entirely separate in the atlas. User allocations are never adjacent to allocator internals, which eliminates a class of corruption bugs where a buffer overflow overwrites allocation headers.

**Thread safety.** System allocators include locking for concurrent use. `izalloc` does not.

**Size and scope.** The system allocator handles anything from 1 byte to gigabytes, adapts its strategy by size class, and has been tuned over decades. `izalloc` is a few hundred lines written over a month, three months into learning to program.

What `izalloc` offers that the system allocator does not: availability in environments where OS calls are forbidden, a fully predictable and configurable memory footprint, and no runtime dependency on anything external.

---

## Configuration

```c
#define MAX_MEMORY     1048576   // 1 MB pool
#define MAX_PARTITIONS 1024      // Maximum simultaneous allocations
```

Both are in `izalloc.h` and can be overridden at compile time:

```sh
cc -DMAX_MEMORY=65536 -DMAX_PARTITIONS=256 your_program.c
```

The defaults are intentionally generous. For small school projects, a few kilobytes and a few dozen partitions would be enough. The high defaults exist so the allocator does not need to be thought about until there is a specific reason to tune it.

---

## File Structure

| File | Contents |
|---|---|
| `izalloc.h` | Types, constants, and function declarations |
| `manager.c` | Public API and central dispatch |
| `allo.c` | Allocation logic |
| `free.c` | Deallocation logic |
| `reallo.c` | Reallocation logic |

Internal functions carry a `not_` prefix to distinguish them from the public API.
---

## Building

A Makefile is provided for convenience.

Build the static library (`izalloc.a`):
```sh
make
```
Clean object files:
```sh
make clean
```
Clean everything including the library:
```sh
make fclean
```
Rebuild from scratch:
```sh
make re
```

Then link the library into your project:
```sh
cc -Wall -Wextra -Werror your_program.c izalloc.a -o your_program
```

---
---

# Project History & Future

## Changelog

> *A record of meaningful changes to the project over time.*

#### v1.0 — Initial release
- `ft_alloc` and `ft_free` implemented with atlas-based metadata
- Best-fit allocation strategy with edge priority
- 16-byte alignment added as a late patch

#### v1.1 — Realloc
- `ft_realloc` added after recognizing the atlas made in-place growth trivial
- In-place growth as primary path; free-then-reallocate as fallback
- `memmove`-equivalent copy handles both overlap directions

---

## Roadmap

**Code quality**
- [ ] Unified binary search — merge `ft_seek` and `ft_search` into one shared function with a consistent sentinel value
- [ ] Context struct refactor — replace static state and packed arrays with a `t_allocator` struct; allows internal functions to become static and removes the 4-argument workarounds
- [ ] Consolidate alignment logic — currently split across `ft_extremes` and `ft_middle`; should live in one shared helper

**New allocation features**
- [ ] `ft_calloc` — allocate and zero-initialize; requires no internal changes, just `ft_alloc` followed by zeroing
- [ ] First-fit strategy — return the first gap large enough rather than the smallest; faster per call, more fragmentation over time; better for throughput-critical workloads where pool longevity matters less
- [ ] Allocation hints — let the caller suggest a preferred region of the pool; useful for cache-locality-sensitive workloads
- [ ] `ft_alloc_aligned(size, alignment)` — for callers that need stricter or custom alignment beyond the default 16 bytes

**Safety and debugging**
- [ ] **Canary values** — a known magic number written at the end of each allocated block, verified on free; a corrupted canary means something wrote past the end of its allocation; worth including in the standard build, not just a debug mode
- [ ] **Debug mode** — compile-flag-activated reporting: which operation failed, current pool state, partition utilization, canary violations, and the address that triggered the problem
- [ ] **Allocation statistics** — peak usage, total allocations over the program's lifetime, current fragmentation ratio; useful for tuning `MAX_MEMORY` and `MAX_PARTITIONS` to a specific program's actual needs

**Performance**
- [ ] **Benchmarks** — a test suite comparing allocation throughput and fragmentation against the system allocator across several workload patterns: many small allocs, few large allocs, random mixed sizes, high free rate, long-lived mixed allocations
- [ ] Gap size cache — track the largest currently available gap to immediately reject requests that cannot possibly fit, avoiding a full search

**Architecture**
- [ ] **Thread-safe version** — guarded API for concurrent use
- [ ] **Tiered pools** — three separate pools for small, medium, and large allocations, each independently sized and tunable; significantly better throughput and fragmentation characteristics for workloads with mixed allocation sizes
