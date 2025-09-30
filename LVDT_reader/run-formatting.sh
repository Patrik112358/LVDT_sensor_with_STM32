# Run clang-format-20 on ./Core/Src/*.c and ./Core/Inc/*.h

# This script is intended to be run from the root of the repository (the LVDT_reader directory).

clang-format-20 -i -style=file "$(dirname "$0")"/Core/Src/*.c
clang-format-20 -i -style=file "$(dirname "$0")"/Core/Inc/*.h
