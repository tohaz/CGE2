make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/t4_scrollbar 2>&1 | tee valgrind_output_t4.lst

