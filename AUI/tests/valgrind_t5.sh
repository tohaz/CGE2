make
valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t5_abutton 2>&1 | tee valgrind_output_t5.lst

