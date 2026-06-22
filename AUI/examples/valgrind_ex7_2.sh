#make clean
make
stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex7_table_2 2>&1 | tee valgrind_output_ex7_2.lst

