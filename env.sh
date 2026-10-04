if [[ -f "./nob.c" ]]
then
    cc nob.c -o nob
    alias nob=$(pwd)/nob
fi
