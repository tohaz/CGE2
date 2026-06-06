make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex1_minimum 2>&1 | tee valgrind_output_ex1.lst

