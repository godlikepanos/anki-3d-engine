find ./src ./include ./tests ./testapp ./tools -name '*.h' -o -name '*.hpp' -o -name '*.c' -o -name '*.cpp' | xargs -I % ./thirdparty/bin/clang-format -sort-includes=false -i %
