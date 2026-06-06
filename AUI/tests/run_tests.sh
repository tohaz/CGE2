#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
NC='\033[0m'

tests=("t1_aui" "t2_abox" "t3_alabel" "t4_ascrollbar" "t5_abutton" "t6_alist")
cur_test=0

run_env() {
    local name=$1
    local test_num=$2
    local desc=$3
    local cmd_prefix=$4
    local env_cmd=$5

    if [ -n "$env_cmd" ]; then
        eval $env_cmd $cmd_prefix ./bin/$name
    else
        $cmd_prefix ./bin/$name
    fi

    if [ $? -eq 0 ]; then
        echo -e "${GREEN}+++Test ${test_num} ${desc} Success+++${NC}"
    else
        echo -e "${RED}Failed ${desc} Test ${test_num}==========================================================================================${NC}"
        exit 1
    fi
}

make

for name in "${tests[@]}"; do
    ((cur_test++))
    run_env "$name" "$cur_test" "Valgrind Wayland" "valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all" ""
    run_env "$name" "$cur_test" "Valgrind XCB" "valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all" "env -u WAYLAND_DISPLAY"
    run_env "$name" "$cur_test" "Raw Wayland" "" ""
    run_env "$name" "$cur_test" "Raw XCB" "" "env -u WAYLAND_DISPLAY"
done

echo -e "${GREEN}+++ALL Success+++${NC}"

