make
valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t11_menu 2>&1 | tee valgrind_output_t11.lst

