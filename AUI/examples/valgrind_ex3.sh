make
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex3_button 2>&1 | tee valgrind_output_ex3.lst

