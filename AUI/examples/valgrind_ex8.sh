make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex8_table 2>&1 | tee valgrind_output_ex8.lst

