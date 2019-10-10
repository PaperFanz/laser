# Laser Usage Guide

After installing Laser on your system using the instructions on the 
[README.md](README.md), invoke Laser by opening a terminal and typing in 
'laser' and hit enter. This will print an 'About' message. Typing in 
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
---
## Editors

It is a very good idea to find a code editor that you enjoy using- it will make 
you much more efficient and may even lessen the pain of editing. Below are some 
recommended editors along with instructions on how to set them up.

### VS Code

This is my current recommendation; it is what I used to develop Laser and it is 
what I will be using for the forseeable future. It's a competent text editor, 
but it stands out in its extensibility and hackability. I have written and 
published an LC3 Extension that should be all you need to get up and running, 
including syntax highlighting, tab completion, snippets, and build tasks.

You can download VS Code [here](https://code.visualstudio.com/) and my LC3 
Extension [here](https://marketplace.visualstudio.com/items?itemName=PaperFanz.lc3-assembly). 
Both are available for free on Windows, Linux, and Mac OS.

### Notepad++

*TODO, contributions appreciated*

<!-- Notepad++ is another very good editor, and is a huge improvement over the 
default Notepad in Windows. However, the downside is that it is only available 
for the Windows operating system and requires a bit of configuration to get to 
the level of productivity that VS Code affords. However, this is the editor 
that Professor Yeraballi uses, and he has syntax highlighting packs and build 
tasks available to copy and use for free. -->

Notepad++ can be found for free [here](https://notepad-plus-plus.org/).

### Atom

Atom is very similar to VS Code, and is built on the same underlying 
technologies. However, at this time there is no extension pack for the LC3 as 
there is on VS Code, so you will have to manually set things up if you prefer 
Atom. If you really love Atom, you can consider authoring your own extension 
pack.

Atom can be found for free [here](https://atom.io/) for Windows, Linux, and Mac 
OS.

### Sublime Text

Sublime Text is also very similar to VS Code, but like Atom lacks an LC3 
extension. Much like Atom, there are extensive APIs for you to author your own 
extension if you so wish.

Sublime Text is avalable on Windows, Linux, and Mac OS for free on their 
[website](https://www.sublimetext.com/).