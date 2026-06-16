make
valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t7_inputbox 2>&1 | tee valgrind_output_t7.lst

