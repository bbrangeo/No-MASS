# Compiling No-MASS    {#compile}

## Build Status
![Build](https://magnum.travis-ci.com/jacoblchapman/No-MASS.svg?token=hNH6EHukhSBPUpNQNYH3&branch=Master "build")

## Checkout

Clone the repository using git, ensure you do a recursive clone to receive all dependencies.
- Rapidxml for xml parsing
- googletest for testing

Use the following commands or alternatively use a GUI such as GitHub Desktop or SourceTree. You'll need to add your ssh key to your profile in github. See [here](https://help.github.com/en/articles/adding-a-new-ssh-key-to-your-github-account)

```sh
git clone https://github.com/jacoblchapman/No-MASS.git

cd No-MASS

git submodule update --init --recursive

```

## Guide to compiling on Linux

Use a compiler with at least the c++11 standard. Most modern linux systems have this by default, if not it can be installed easily.
CMake is used to build the make file.

### Compiling

Build the No-MASS using cmake and make

```sh
# Move it the FMU directory
cd FMU

# Create a directory to create the make files in and change to that directory
mkdir build && cd build

# Use cmake to create the make files for your system
cmake ../

# Compile the share library using make
make
```

### Testing

To run the tests enable testing in CMake, build, and run the test program

```sh
# Move into the build directory
cd FMU/build

# Enable debugging and testing in cake
cmake -DCMAKE_BUILD_TYPE=Debug -DTests=on ../

# Compile using make
make

# Run the tests
./tests/runUnitTests
```


## Guide to compiling on Windows

No-MASS can be built on windows using mingw-w64 or alternatively visual studio. This differs for each windows version, but the general process is:
- Ensure you know which EnergyPlus architecture you are targeting 32bit(i686) or 64bit(x86_64)
- Install [mingw-w64](http://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win32/Personal%20Builds/mingw-builds/installer/) or visual studio depending on preferred method of compiling. With mingw-w64 choose the correct architecture.
- Install [CMake](https://cmake.org/download/)
- Use the CMake GUI to create the visual studio project or mingw-w64 make files from the FMU directory in the checked out source folder.
- Open in visual studio to compile or use make command as in linux.

## Install WINDOWS

SITE [msys2](https://www.msys2.org/docs/terminals/#windows-terminal)
EXE [install](https://github.com/msys2/msys2-installer/releases/download/2024-12-08/msys2-x86_64-20241208.exe)
``
pacman -S git mingw-w64-x86_64-nlohmann-json mingw-w64-x86_64-asio mingw-w64-x86_64-cmake mingw-w64-x86_64-gcc
git clone https://github.com/bbrangeo/No-MASS.git
pacman -S mingw-w64-x86_64-toolchain
ssh-keygen -t rsa -b 4096
cat /home/Administrateur/.ssh/id_rsa.pub
git submodule update --init --recursive
mkdir build
cd build
cmake ..
./NoMASS2Server.exe -c ../../Configuration/HouseWindows-MD/SimulationConfig.xml -d ../../Configuration/HouseWindows-MD/modelDescription.xml
``

## Running

Take the compiled library (FMI.so or FML.dll) and place into the FMU zip file in the correct binary folder, depending on your architecture.
- win64
- win32
- linux64
- linux32

Create model description file and the SimulationConfig file for your simulation.
Place the FMU file in the EnergyPlus folder, define the variables in the IDF and run EnergyPlus.
