# CPP Project
A tool created for personal use to create C++ projects more easy like.

## Build
```
$ mkdir build
$ cd build
$ cmake ..
$ make
```
or
```
$ ./install.sh
```

## Usage
```
cpp_project <project_name> [flags]c

Flags:
        -h, --help              : Outputs this message.
        -n, --name              : Sets the project's name.
        -d, --description       : Sets the project's description
        -v, --version           : Sets the project version.
        -s, --shared            : Makes the project a shared library.
```