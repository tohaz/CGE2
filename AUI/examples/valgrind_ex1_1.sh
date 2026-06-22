make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex1_1_minimum 2>&1 | tee valgrind_output_ex1_1.lst

