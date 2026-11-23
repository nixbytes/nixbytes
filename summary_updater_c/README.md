# C Summary Updater

A lightweight tool written in C to analyze a Git repository and update the root `README.md` with project statistics.

## For Beginners: How it Works
This program acts as a "wrapper" around Git. Instead of implementing complex Git logic in C, it calls the `git` command-line tool using a function called `popen()`, which reads the output of the command just like you would in a terminal.

## How to Build and Run

### Prerequisites
You need a C compiler installed:
- **Linux:** `sudo apt install build-essential`
- **macOS:** `xapps install Xcode` (or `brew install gcc`)
- **Windows:** Install MinGW or use WSL.

### Building
Open your terminal in the `summary_updater_c` folder and run:
```bash
gcc -I src src/summary.c src/main.c -o summary_updater
```

### Running
Make sure you are in the root of a Git repository, then run:
```bash
./summary_updater
```
*Note: This will modify the `README.md` file in the root directory.*

## Project Structure
- `src/summary.h`: The "header" file. It tells the program what functions exist.
- `src/summary.c`: The "logic" file. It contains the actual code for gathering stats.
- `src/main.c`: The "entry point". It simply starts the update process.
- `tests/`: Contains a small program to verify the logic is correct.
