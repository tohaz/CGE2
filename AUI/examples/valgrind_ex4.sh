make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex4_two_buttons 2>&1 | tee valgrind_output_ex4.lst

