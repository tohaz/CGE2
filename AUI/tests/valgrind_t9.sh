make
valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t9_combobox 2>&1 | tee valgrind_output_t9.lst

