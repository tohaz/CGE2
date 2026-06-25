make
stdbuf -o0 valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t6_list 2>&1 | tee valgrind_output_t6.lst

