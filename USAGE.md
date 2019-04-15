# Laser Usage Guide

After installing Laser on your system, invoke Laser by opening a terminal and 
typing in 'laser' and hit enter. This will print an 'About' message. Typing in 
'laser -h' will display the following help message:

```
Usage:
    -v: 'laser -v' displays version number
    -h: 'laser -h' displays this message again
    -q: 'laser -q' suppresses all warning messages during assembly
    -s: 'laser -s' suppresses all warnings and errors during assembly
    -a: 'laser -a <file>' assembles the specified file
        'laser -a *.asm' assembles all assembly files in current directory
    -p: 'laser -p <file1> <file2>' assembles the specified files as a project
        'laser -p *.asm' assembles all files in current directory as a project
    -c: 'laser -c <file>' removes all files generated during assembly
```