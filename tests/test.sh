#!/bin/sh

#Automated tests for myls on NetBSD
#Run from the project root: sh tests/test.sh

PROGRAM="./myls"
PASS=0
FAIL=0
TEST_DIR="./tests/test_data"

#Check that the program exists

if [ ! -x "$PROGRAM" ]; then
echo "ERROR: $PROGRAM not found or not executable."
echo "Run 'make' first."
exit 1
fi

#Create test data

rm -rf "$TEST_DIR"
mkdir -p "$TEST_DIR/subdir"

printf "small\n" > "$TEST_DIR/small.txt"
printf "This is a larger test file.\n" > "$TEST_DIR/large.txt"
printf "hidden\n" > "$TEST_DIR/.hidden"
printf "child\n" > "$TEST_DIR/subdir/child.txt"

#Helper: run a test and check the exit status

run_test() {
name="$1"
shift

if "$PROGRAM" "$@" > /dev/null 2>&1; then
    echo "PASS: $name"
    PASS=$((PASS + 1))
else
    echo "FAIL: $name"
    FAIL=$((FAIL + 1))
fi

}

#Helper: run myls against the test directory and check for a name

check_contains() {
name="$1"
expected="$2"
shift 2

if "$PROGRAM" "$@" "$TEST_DIR" 2>/dev/null | grep -F "$expected" >/dev/null; then
    echo "PASS: $name"
    PASS=$((PASS + 1))
else
    echo "FAIL: $name"
    FAIL=$((FAIL + 1))
fi

}

#Helper: run myls against the test directory and check that a name is absent

check_absent() {
name="$1"
unexpected="$2"
shift 2

if "$PROGRAM" "$@" "$TEST_DIR" 2>/dev/null | grep -F "$unexpected" >/dev/null; then
    echo "FAIL: $name"
    FAIL=$((FAIL + 1))
else
    echo "PASS: $name"
    PASS=$((PASS + 1))
fi

}

echo "===== myls automated tests ====="

#1. Default listing

check_contains "Default listing shows normal files" "small.txt"
check_absent "-a is not enabled by default" ".hidden"

#2. Hidden files

check_contains "-a shows hidden files" ".hidden" -a
check_contains "-A shows hidden files" ".hidden" -A
check_absent "-A excludes dot entry" "./" -A

#3. Listing options

run_test "-c runs successfully" -c "$TEST_DIR"
run_test "-d runs successfully" -d "$TEST_DIR"
run_test "-F runs successfully" -F "$TEST_DIR"
run_test "-f runs successfully" -f "$TEST_DIR"
run_test "-h runs successfully" -h "$TEST_DIR"
run_test "-i runs successfully" -i "$TEST_DIR"
run_test "-k runs successfully" -k "$TEST_DIR"
run_test "-l runs successfully" -l "$TEST_DIR"
run_test "-n runs successfully" -n "$TEST_DIR"
run_test "-q runs successfully" -q "$TEST_DIR"
run_test "-r runs successfully" -r "$TEST_DIR"
run_test "-R runs successfully" -R "$TEST_DIR"
run_test "-S runs successfully" -S "$TEST_DIR"
run_test "-s runs successfully" -s "$TEST_DIR"
run_test "-t runs successfully" -t "$TEST_DIR"
run_test "-u runs successfully" -u "$TEST_DIR"

#4. Recursive listing

check_contains "-R lists files in subdirectories" "child.txt" -R

#5. Combined options

run_test "Combined -la options" -l -a "$TEST_DIR"
run_test "Combined -lS options" -l -S "$TEST_DIR"
run_test "Combined -ir options" -i -r "$TEST_DIR"

#6. Summary

echo
echo "===== Test summary ====="
echo "Passed: $PASS"
echo "Failed: $FAIL"

#Remove generated test data

rm -rf "$TEST_DIR"

if [ "$FAIL" -eq 0 ]; then
echo "All tests passed."
exit 0
else
echo "Some tests failed."
exit 1
fi