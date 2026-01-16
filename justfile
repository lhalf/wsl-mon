set shell := ["bash", "-euc"]

fmt:
    clang-format -i src/*.c include/*.h

check:
    find src include -name "*.c" -o -name "*.h" | xargs clang-format --dry-run -Werror
    cppcheck --enable=all --error-exitcode=1 --suppress=missingInclude src/ include/
