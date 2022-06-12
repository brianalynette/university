General Rules
=============

### Preparing the Build Environment

If you are on windows, follow these [detailed instructions](Windows.md).

The assignments will use CMake as a build system. Before we can begin, you must install CMake on your computer.
I recommend installing it with a package manager instead of the [CMake download page](http://www.cmake.org/download/). E.g. on Debian/Ubuntu: `sudo apt-get install cmake`, with HomeBrew on Mac: `brew install cmake`, and on Windows with [Chocolatey](https://chocolatey.org/): `choco install -y cmake`.

You must install a C++ compiler: `gcc/clang` on Linux, `clang` on Mac, [Visual Studio](https://www.visualstudio.com/) on Windows. If you are looking for an IDE to develop in C++, we recommend [Visual Studio Code](https://code.visualstudio.com) on Mac/Linux, and [Visual Studio](https://www.visualstudio.com/) on Windows.

### Compiling the Sample Projects

We will provide a folder for each assignment with some sample code to get you started. Included in each assignment folder is a CMake build system that should enable you to compile your code easily.
For each assignment, you will need to do the following:

1. Download the assignment code
2. Create a directory called `build` in the assignment directory, e.g.:
   ```
   cd assignment_X; mkdir build
   ```
3. Use CMake to generate the Makefile/project files needed for compilation inside the `build/` directory:
   ```
   cd build; cmake -DCMAKE_BUILD_TYPE=Debug ..
   ```
4. Compile and run the compiled executable by typing:
   ```
   make; ./assignmentX
   ```
