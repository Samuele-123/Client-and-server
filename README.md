# MultiThreaded-FileStats-Server

## Description
This C program is designed to calculate the arithmetic mean and standard deviation for each ".dat" file in a directory and its subdirectories. The program utilizes a master process that spawns worker threads to perform calculations for each file and communicates the results to a collector process using sockets. The collector process aggregates the results and prints them in a tabular format.

## Project Structure

### File Hierarchy
The directory structure is assumed to have files and subdirectories containing ".dat" files. Files may contain integer or decimal numbers, one per line, possibly separated by empty lines. Numbers may have leading or trailing whitespace and/or tab characters.

Example directory structure:
```
- ./prova1.dat
- ./prova2.dat
- ./script.sh
- ./provadir
  - ./provadir/provadir.dat
  - ./provadir/provadir1
    - ./provadir/provadir1/provadir1.dat
  - ./provadir/provadir2
    - ./provadir/provadir2/provadir2-1.dat
    - ./provadir/provadir2/provadir2-2.dat
    - ./provadir/provadir2/provadir3
      - ./provadir/provadir2/provadir3/provadir3-1.dat
      - ./provadir/provadir2/provadir3/provadir3-2.dat
      - ./provadir/provadir2/provadir3/provadir3-3.dat
```

### Output Format
The program generates a tabular output with the following columns: `n` (total valid numbers), `avg` (arithmetic mean), `std` (standard deviation), and `file` (file name). The table is printed in the order of data arrival through the socket.

Example output:
```
n    avg      std      file
------------------------------
3    3.00     0.81     ./prova1.dat
7    30.71    29.81    ./provadir/provadir2/provadir3/provadir3-3.dat
7    30.71    29.81    ./provadir/provadir2/provadir2-1.dat
13   4.61     2.55     ./provadir/provadir1/provadir1.dat
1    1.00     0.00     ./prova2.dat
8    178.59   402.10   ./provadir/provadir.dat
532  3.00     3.08     ./provadir/provadir2/provadir2-2.dat
1318 18.92    10.57    ./provadir/provadir2/provadir3/provadir3-1.dat
6    50.83    67.58    ./provadir/provadir2/provadir3/provadir3-2.dat
```

## Program Execution

### Master Process
The master process takes the initial directory name and the number of worker threads (`W`) as command-line parameters. It recursively traverses all subdirectories, obtains the names of ".dat" files, and communicates them to worker threads via an unbounded buffer (implemented as `unboundedqueue.h`).

### Worker Threads
Each worker thread, after reading a filename from the shared buffer, opens and calculates the required values for the file. The results (average, standard deviation, and filename) are then communicated to the collector process through a socket.

### Collector Process
The collector process acts as a server in the socket communication, and worker threads act as clients. Results from multiple worker threads are sent through a single or separate connections. The collector prints the aggregated results in the specified tabular format.

## Build and Run Instructions

### Makefile Targets
- `all` (default): Compiles the C program.
- `test1`: Executes the program with W=1 on a test directory.
- `test2`: Executes the program with W=5 on a test directory.
- `test3`: Executes valgrind with W=5 on a test directory.

### Example Commands
```bash
make           # Compile the program
make test1     # Run the program with W=1 on a test directory
make test2     # Run the program with W=5 on a test directory
make test3     # Run valgrind with W=5 on a test directory
```
