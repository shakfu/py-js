
# brew install zeromq
for i in 1 2 3 4
do
    gcc -o test_thread${i} -lpthread test_thread${i}.c
done





