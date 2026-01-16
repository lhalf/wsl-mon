set shell := ["bash", "-euc"]

test:
    cmake -S . -B build
    cmake --build build
    ./build/unit_tests

fmt:
    find src include -name "*.c" -o -name "*.h" | xargs clang-format -i

check:
    find src include -name "*.c" -o -name "*.h" | xargs clang-format --dry-run -Werror
    cppcheck --enable=all --error-exitcode=1 --suppress=missingInclude --suppress=unmatchedSuppression --suppress=missingIncludeSystem src/ include/
