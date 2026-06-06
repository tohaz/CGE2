make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex9_second_window 2>&1 | tee valgrind_output_ex9.lst

