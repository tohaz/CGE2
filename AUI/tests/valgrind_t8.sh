make
stdbuf -o0 valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t8_table 2>&1 | tee valgrind_output_t8.lst

