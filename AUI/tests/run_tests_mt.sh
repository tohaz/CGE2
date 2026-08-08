#!/bin/bash

START_TIME=$SECONDS

YELLOW='\033[0;33m'
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

print_elapsed_time() {
    local elapsed=$((SECONDS - START_TIME))
    echo -e "${YELLOW}Total execution time: $((elapsed / 60))m $((elapsed % 60))s${NC}"
}

tests=(
 "t1_aui" "t2_box" "t3_label" "t4_scrollbar" "t5_button" "t6_list" "t7_inputbox" "t8_table" "t9_combobox" "t10_progressbar" 
)

echo -e "${YELLOW}Building binaries...${NC}"
if ! make -j"$(nproc)"; then
    echo -e "${RED}Error: Build failed. Exiting script.${NC}"
    print_elapsed_time
    exit 1
fi

LOG_DIR=$(mktemp -d)
trap 'rm -rf "$LOG_DIR"' EXIT

run_single_test() {
    local name="$1"
    local env_type="$2"
    local test_num="$3"
    local log_file="$LOG_DIR/${test_num}_${name}_${env_type}.log"

    local cmd=""
    case "$env_type" in
        "valgrind_wayland")
            cmd="stdbuf -o0 valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/$name"
            ;;
        "valgrind_xcb")
            cmd="AUI_FORCE_X11=1 stdbuf -o0 valgrind --error-exitcode=1 --leak-check=full --errors-for-leak-kinds=all ./bin/$name"
            ;;
        "raw_wayland")
            cmd="./bin/$name"
            ;;
        "raw_xcb")
            cmd="AUI_FORCE_X11=1 ./bin/$name"
            ;;
    esac

    {
        echo "=== Running Test ${test_num} (${name}) [${env_type}] ==="
        eval "$cmd" 2>&1
        local exit_code=$?
        echo "--------------------------------------------------"
        exit $exit_code
    } > "$log_file" 2>&1
}

# Limit maximum parallel workers to CPU thread count
MAX_JOBS="$(nproc)"
pids=()
test_num=0

echo -e "${YELLOW}Running tests in parallel (max ${MAX_JOBS} concurrent jobs)...${NC}"

for name in "${tests[@]}"; do
    ((test_num++))
    for env in "valgrind_wayland" "valgrind_xcb" "raw_wayland" "raw_xcb"; do
        
        # Run test in background
        run_single_test "$name" "$env" "$test_num" &
        pids+=($!)

        # Throttling: If running jobs reach MAX_JOBS, wait for one to finish
        if [ "${#pids[@]}" -ge "$MAX_JOBS" ]; then
            wait -n
            # Filter out finished PIDs
            new_pids=()
            for pid in "${pids[@]}"; do
                if kill -0 "$pid" 2>/dev/null; then
                    new_pids+=("$pid")
                fi
            done
            pids=("${new_pids[@]}")
        fi
    done
done

# Wait for all remaining background processes
FAILED=0
for pid in "${pids[@]}"; do
    if ! wait "$pid"; then
        FAILED=1
    fi
done

# Output concatenation and cleanup handling
final_file="all_tests_output.lst"
cat "$LOG_DIR"/*.log > "$final_file" 2>/dev/null

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}+++ALL Success+++${NC}"
    echo -e "All test output saved to: ${final_file}"
    print_elapsed_time
else
    echo -e "${RED}Error: One or more tests failed.${NC}"
    echo -e "${RED}Error output saved to: ${final_file}${NC}"
    print_elapsed_time
    exit 1
fi
