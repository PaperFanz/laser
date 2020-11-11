# LASER - [**L**]c3 [**AS**]sembl[**ER**]

## Description

Laser is a cross-platform, command-line-based assembler for the LC3 assembly 
language implemented in pure C. Version 2 extends the functionality of LC3Tools 
and LC3Edit through new pseudoops, along with a project assembly mode.

## Installing

Under the releases tab, there are pre-compiled binaries for Windows, Mac, and 
Linux. In addition, it is highly recommended that you use this program with an 
extensible editor like VSCode or Notepad++, and to add Laser as a shortcut in 
your editor. If all this installation stuff is not up your alley or you'd like 
to learn more about how Laser was created, take a look at 
[BUILDING.md](BUILDING.md) for instructions on how to build Laser from source.

### Linux/Mac OS X

Once you've downloaded the precompiled binary (building from source is actually 
recommended on Linux due to all the different distrubutions and their 
differences, but Laser has practically 0 dependencies so it should be fine), 
copy the file to either `/usr/bin` or `/usr/local/bin` and make it executable. 
Alternatively, put it wherever and add it to your PATH environment variable.

### Windows

Download the `laser-x-x-x.exe` from the releases tab and place it somewhere 
permanent (I.E. not your Downloads folder), and add it to your Windows PATH.

To add the program to your PATH, open the file explorer, right click on This PC,
and select 'properties'. Then, click on 'advanced system settings' and click on 
the 'environment variables' button. In the window that pops up, highlight the 
'Path' variable and select 'edit'. From there, click 'new' and input the file 
path of where you placed the downloaded laser executable. Afterwards, click 
'ok' and then 'apply'. You should then be able to invoke laser from the command 
line.

## Usage

See [USAGE.md](USAGE.md)

## Building from Source

see [BUILDING.md](BUILDING.md)

## Copyright

Laser- a command line utility to assemble LC3 assembly code

Copyright Notice:

    Copyright 2018, 2019 Zhiyuan Fan

License Notice:

    Laser is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Laser is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Laser.  If not, see <https://www.gnu.org/licenses/>.
