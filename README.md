# File Process Monitor

A lightweight security monitoring tool written in C that combines real-time file monitoring, 
hash-based malware detection and basic network inspection.
A simple file monitoring and analysis system developed in C.
It operates on basic antivirus logic.

After the user enters a file path and monitoring duration,
the system launches a three-stage analysis:

## Stage 1: File Monitoring
All changes in the specified directory are tracked in real time
for the given duration. Newly created, modified, and deleted files
are printed to the terminal with timestamps.
Detected file paths are passed to the next stage.

## Stage 2: Hash Analysis
Hash values of detected files are calculated and compared against
known malicious hash values defined in `zararli.txt`.
The analysis result is printed to the screen.

## Stage 3: Network Scan
The system performs a network scan and prints open ports to the screen.

## Usage
Run the program with a target directory and monitoring duration as arguments.

## Language
C

## Academic Context
This project was developed as a 1st year, 2nd semester assignment.
