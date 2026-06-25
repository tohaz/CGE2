#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'
#
#     "t7_inputbox" "t8_table" "t9_combobox" "t10_progressbar" "t11_menu" "t2_box" "t3_label" "t4_scrollbar" "t5_button" "t6_list"
tests=(
    "t1_aui" 
)

echo -e "${YELLOW}Building binaries...${NC}"

if ! make; then
    echo -e "${RED}Error: Build failed. Exiting script.${NC}"
    exit 1
fi

run_env() {
    local name="$1"
    local test_num="$2"
    local desc="$3"
    local use_valgrind="$4"
    local clear_wayland="$5"
    
    echo -e "${YELLOW}=== Running Test ${test_num} (${name}) [${desc}] ===${NC}"

    # Setup file paths
    local desc_slug="${desc// /-}"
    local final_log="output_${name}_${desc_slug}.lst"

    # 1. Construct and execute the command directly to avoid subshell masking
    if [ "$clear_wayland" = "true" ] && [ "$use_valgrind" = "true" ]; then
        env -u WAYLAND_DISPLAY valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/"$name" > "$final_log" 2>&1
    elif [ "$clear_wayland" = "true" ]; then
        env -u WAYLAND_DISPLAY ./bin/"$name" > "$final_log" 2>&1
    elif [ "$use_valgrind" = "true" ]; then
        valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/"$name" > "$final_log" 2>&1
    else
        ./bin/"$name" > "$final_log" 2>&1
    fi
    
    local exit_code=$?

    # 2. Dump the saved log to the screen immediately so you see it live
    cat "$final_log"

    echo -e "\n--------------------------------------------------"

    # 3. Evaluate the result
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}+++Test ${test_num} ${desc} Success+++${NC}\n"
        rm -f "$final_log" # Clean up on success
    else
        echo -e "${RED}Failed ${desc} Test ${test_num}==========================================================================================${NC}"
        echo -e "${RED}Log preserved at: ./${final_log}${NC}"
        exit 1 # Forces the loop to break instantly
    fi
}

cur_test=0
for name in "${tests[@]}"; do
    ((cur_test++))
    
    run_env "$name" "$cur_test" "Valgrind Wayland" true false
    run_env "$name" "$cur_test" "Valgrind XCB" true true
    run_env "$name" "$cur_test" "Raw Wayland" false false
    run_env "$name" "$cur_test" "Raw XCB" false true
done

echo -e "${GREEN}+++ALL Success+++${NC}"

