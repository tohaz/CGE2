make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex6_inputbox 2>&1 | tee valgrind_output_ex6.lst

