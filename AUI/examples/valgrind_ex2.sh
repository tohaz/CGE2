make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex2_helloworld 2>&1 | tee valgrind_output_ex2.lst

