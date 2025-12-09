# Oracle - AUR Helper Wrapper (C++ Implementation)

Oracle is a modern graphical user interface wrapper for AUR helpers and Pacman, designed to make package management on Arch Linux more accessible and user-friendly. This is a C++ implementation of the original [Python Oracle application](https://github.com/0xgingi/oracle).

![image](https://github.com/user-attachments/assets/4253408c-736b-4985-9f6a-00833cdc9bda)

## Prerequisites

- Arch Linux
- C++ compiler (GCC or Clang)
- Qt6
- CMake
- One of the following AUR helpers:
  - yay (recommended)
  - paru
  - pamac
  - aurman
  - pikaur

## Installation

### Building from Source

1. Clone the repository:
```bash
git clone https://github.com/ExilProductions/oracle-cpp.git
cd oracle-cpp
```

2. Install build dependencies:
```bash
# On Arch Linux
sudo pacman -S qt6-base cmake
```

3. Build the project:
```bash
mkdir build
cd build
cmake ..
make
```

The executable will be created in the `build` directory as `Oracle`.

4. Run the application:
```bash
./Oracle
```
