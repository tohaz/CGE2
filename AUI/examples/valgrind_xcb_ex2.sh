make
env -u WAYLAND_DISPLAY stdbuf -o0 valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes ./bin/ex2_abox 2>&1 | tee valgrind_output_ex2.lst

