file="$1"
filename="${file%.*}"

gcc -I./include -D__hoc__ -P -E "$1" -o "${filename}_pp.c" &&
    ./hoc "${filename}_pp.c" > "${filename}.s" &&
    rm "${filename}_pp.c"
