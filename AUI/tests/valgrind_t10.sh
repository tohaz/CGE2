make
valgrind --leak-check=full -s --show-leak-kinds=all --track-origins=yes ./bin/t10_progressbar 2>&1 | tee valgrind_output_t10.lst

