valgrind --tool=callgrind --callgrind-out-file=callgrind.out.tmp --collect-jumps=yes --branch-sim=yes --cache-sim=yes --simulate-wb=yes --cacheuse=yes --simulate-hwpref=yes ./build/testapp/testapp

# --I1=32768,2,64 --D1=32768,2,64 --LL=1048576,16,64
