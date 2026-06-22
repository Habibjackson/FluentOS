# Fluent

## Requirements

- CMake 3.24 or newer
- A C++20 compiler
- Qt 6.11 or newer with the Core, Gui, Qml, Quick, and DBus components

## Build

```sh
cmake -S . -B build -DCMAKE_PREFIX_PATH=/path/to/Qt/6.11/<platform>
cmake --build build --parallel
```

Or use the helper script:

```sh
QT_PREFIX_PATH=/path/to/Qt/6.11/<platform> ./build.sh
```

If `QT_PREFIX_PATH` is not set, `build.sh` will try to find a local Qt 6.11
install automatically.
