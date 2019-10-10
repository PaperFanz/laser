# Building Laser from Source

# Linux

Dependencies: `gcc, make`

```shell
git clone https://github.com/PaperFanz/laser.git
git checkout testing
cd laser/src
make
sudo make install
# alternatively, move the compiled binary to your preferred location
```

# Windows

There are two avenues to using Laser on Windows. The first (and recommended) 
was is to install Windows Subsystem Linux, in which case the instructions will
be exactly the same as on Linux. The second way is to download a zip file from
Github, extract the files, and use either MinGW or Cygwin to build Laser.

```shell
cd laser\\src
# invoke the version of make installed on your system
# add the resulting 'laser.exe' to your windows path
```

# Mac OSX

Dependencies: `xcode command line tools`

```shell
xcode-select --install
git clone https://github.com/PaperFanz/laser.git
git checkout testing
cd laser/src
make
sudo make install
# alternatively, move the compiled binary to your preferred location
```