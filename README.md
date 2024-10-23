# CPP Project

A tool designed for personal use to simplify the creation of C++ projects.

## Table of Contents
- [Build](#build)
- [Usage](#usage)
- [Flags](#flags)

## Build

To build the project, follow these steps:

1. Create a build directory and navigate into it:
   ```bash
   mkdir build
   cd build
   ```

2. Run CMake and make:
   ```bash
   cmake ..
   make
   ```

Alternatively, you can use the installation script:
   ```bash
   ./install.sh
   ```

## Usage

To create a new project, use the following command:

```bash
cpp_project <project_name> [flags]
```

### Flags

The following flags are available:

- `-h`, `--help`: Outputs a help message.
- `-n`, `--name=<name>`: Sets the project binary's name.
- `-d`, `--description=<desc>`: Sets the project's description.
- `-v`, `--version=<ver>`: Sets the project version.
- `-s`, `--shared`: Makes the project a shared library.

## Example

Here’s an example of how to create a new project:

```bash
cpp_project MyAwesomeProject -n="project" -d="This is an awesome project!" -v="1.0.0"
```

This is the expected output from the example given above:
```
Project Information:
        Name: project
        Verison: 1.0.0
        Description: This is an awesome project!
        Shared: 0
        Directory: ./MyAwesomeProject
```