# Install

There are two ways to get the executable file. If you are an user, there is no necessity to build the executable file from the source code by yourself. We recommend you to use our pre-built files, see [Pre-Built](#Pre-Built). However, if you are a developer, please look at [Build From Source](#Build From Source).

---

## Pre-Built (Recommended)

[https://github.com/JueXiuHuang/MapArtist/releases/latest](https://github.com/JueXiuHuang/MapArtist/releases/latest)

Just download the zip file from the Github Release, and unzip it.

![release](site:images/release.png)

---

## Build From Source

### Preparation

To successfully build the executable file, we need the following tools:

- [Git](https://git-scm.com/): download dependencies from Github.
- [MSVC](https://visualstudio.microsoft.com/downloads/) (Microsoft Visual C++): the compiler used to compile the whole project.
- [CMake](https://cmake.org/): configure and manage the build system.

### Steps

1. Clone the project.

    ``` bash
    git clone https://github.com/JueXiuHuang/MapArtist
    ```

2. Configure

    ``` bash
    cd MapArtist
    cmake -B build -S .
    ```

3. Build

    ``` bash
    cmake --build build
    ```

    The executable file will be generated in `MapArtist/bin`
