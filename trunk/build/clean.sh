ls | xargs -I % echo % | grep -v .sh | grep -v android.toolchain.cmake | xargs rm -rf
