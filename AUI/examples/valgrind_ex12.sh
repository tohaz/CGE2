make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex12_decorations 2>&1 | tee valgrind_output_ex10.lst

