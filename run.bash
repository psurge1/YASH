make clean
make
valgrind ./yash --trace-children=yes --leak-check=full