#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

#     "t7_inputbox" "t8_table" "t9_combobox" "t10_progressbar" "t11_menu"   "t4_scrollbar" "t5_button" "t6_list"
tests=(
   "t1_aui" "t2_box" "t3_label"
)

# Global accumulator for all output (will be written to a file on success)
ALL_OUTPUT=""

echo -e "${YELLOW}Building binaries...${NC}"

if ! make; then
    echo -e "${RED}Error: Build failed. Exiting script.${NC}"
    exit 1
fi

# Run a single test environment, capture output in memory
run_env() {
    local name="$1"
    local test_num="$2"
    local desc="$3"
    local use_valgrind="$4"
    local clear_wayland="$5"

    echo -e "${YELLOW}=== Running Test ${test_num} (${name}) [${desc}] ===${NC}"

    # Build the command line as a string
    local cmd
    if [ "$clear_wayland" = "true" ] && [ "$use_valgrind" = "true" ]; then
        cmd="env -u WAYLAND_DISPLAY valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/$name"
    elif [ "$clear_wayland" = "true" ]; then
        cmd="env -u WAYLAND_DISPLAY ./bin/$name"
    elif [ "$use_valgrind" = "true" ]; then
        cmd="valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/$name"
    else
        cmd="./bin/$name"
    fi

    # Execute the command and capture stdout+stderr
    local output
    output=$(eval "$cmd" 2>&1)
    local exit_code=$?

    # Immediately print the captured output to the console (like the original 'cat')
    echo "$output"
    echo -e "\n--------------------------------------------------"

    # Accumulate the output (with headers) for the final dump
    ALL_OUTPUT+="=== Running Test ${test_num} (${name}) [${desc}] ===\n"
    ALL_OUTPUT+="$output\n"
    ALL_OUTPUT+="--------------------------------------------------\n"

    # Check result
    if [ $exit_code -ne 0 ]; then
        echo -e "${RED}Failed ${desc} Test ${test_num}==========================================================================================${NC}"
        # Do NOT write the combined file; exit immediately
        exit 1
    fi
}

# Run all tests
cur_test=0
for name in "${tests[@]}"; do
    ((cur_test++))
    run_env "$name" "$cur_test" "Valgrind Wayland" true false
    run_env "$name" "$cur_test" "Valgrind XCB" true true
    run_env "$name" "$cur_test" "Raw Wayland" false false
    run_env "$name" "$cur_test" "Raw XCB" false true
done

# If we reached here, all tests passed – dump all accumulated output to one file
echo -e "${GREEN}+++ALL Success+++${NC}"

# Write the combined output to a single file
final_file="all_tests_output.lst"
echo -e "$ALL_OUTPUT" > "$final_file"
echo -e "All test output saved to: ${final_file}"