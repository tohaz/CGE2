make
valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./bin/t5_table
#callgrind_annotate $(ls -t callgrind.out.* | head -n1) > ./callgrind.lst
kcachegrind $(ls -t callgrind.out.* | head -n1)
rm -rf ./callgrind.out.*

