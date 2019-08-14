file="$1"
filename="${file%.*}"

gcc -D__hoc__ -P -E "$1" -o "${filename}_pp.c" &&
    ./gen_first/hoc "${filename}_pp.c" > "${filename}_1.s" &&
    rm "${filename}_pp.c"
