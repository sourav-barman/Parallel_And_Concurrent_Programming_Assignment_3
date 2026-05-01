# Parallel_And_Concurrent_Programming_Assignment_3
===============================================================================
  CS5300 - Parallel and Concurrent Programming
  MRSW Regular Register Construction With and Without Atomic Bool
===============================================================================

AUTHOR
------
  Name   : SOURAV BARMAN

FILE LISTING
------------
  mrsw.cpp    - Main source file (submit this)
  inp-params.txt       - Input parameter file (must be in same directory)
  readme.txt           - This file
  report.pdf           - Assignment report (PDF compiled from report.tex)

-------------------------------------------------------------------------------
COMPILATION
-------------------------------------------------------------------------------

  g++ -std=c++17 -o mrsw mrsw.cpp

-------------------------------------------------------------------------------
INPUT FILE FORMAT  (inp-params.txt)
-------------------------------------------------------------------------------

  Create a plain-text file named exactly "inp-params.txt" in the same
  directory as the compiled binary. The file uses key-value pairs, one per
  line. Order does not matter. Example:

  ┌─────────────────────────────┐
  │ N         15                │
  │ numReads  1000              │
  │ numWrites 5000              │
  │ lamdaR    5                 │
  │ lamdaW    10                │
  └─────────────────────────────┘

  Parameter descriptions:
    N          Number of reader threads (the program creates N+1 threads total)
    numReads   Number of read operations each reader thread performs
    numWrites  Number of write operations the single writer thread performs
    lamdaR     Mean inter-read delay in microseconds (exponential distribution)
    lamdaW     Mean inter-write delay in microseconds (exponential distribution)

-------------------------------------------------------------------------------
EXECUTION
-------------------------------------------------------------------------------

  ./regular-test

  The program will:
    1. Read inp-params.txt from the current directory.
    2. Run the experiment WITHOUT atomic bool (plain bool array).
    3. Run the experiment WITH atomic bool (std::atomic<bool> array).
    4. Print per-thread cumulative execution times to stdout for both variants.
    5. Generate log files for every reader and writer thread (see below).

  Expected stdout output (example, N=2):
    All threads completed.
    Thread execution times: Without Atomic Bool
    Thread 0 : 2143 micro sec
    Thread 1 : 2198 micro sec
    Thread 2 : 48211 micro sec

    All threads completed.
    Thread execution times: With Atomic Bool
    Thread 0 : 3421 micro sec
    Thread 1 : 3389 micro sec
    Thread 2 : 76834 micro sec

-------------------------------------------------------------------------------
OUTPUT LOG FILES
-------------------------------------------------------------------------------

  After execution, the following log files are created in the current directory:

  Without Atomic Bool:
    Reader_Log_File-0(Without_Atomic_Bool).log
    Reader_Log_File-1(Without_Atomic_Bool).log
    ...
    Reader_Log_File-{N-1}(Without_Atomic_Bool).log
    Writer_Log_File(Without_Atomic_Bool).log

  With Atomic Bool:
    Reader_Log_File-0(With_Atomic_Bool).log
    Reader_Log_File-1(With_Atomic_Bool).log
    ...
    Reader_Log_File-{N-1}(With_Atomic_Bool).log
    Writer_Log_File(With_Atomic_Bool).log

  Each reader log entry format:
    Reader <id> started Read <k> at <timestamp_microseconds>
    Reader <id> completed Read <k> at <timestamp_microseconds> returning value: <val>

  Each writer log entry format:
    Writer  started Write <k> at <timestamp_microseconds>
    Writer  completed Write <k> at <timestamp_microseconds> with the value: <val>

  Correctness check: For any completed read that is NOT overlapping with a
  write (i.e., startTime > last write endTime), the returned value must equal
  the value written in the most recent preceding write. This can be verified
  by post-processing the log files.

-------------------------------------------------------------------------------







