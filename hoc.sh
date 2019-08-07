file="$1"
filename="${file%.*}"

gcc -P -E "$1" -o "${filename}_pp.c" &&
    ./hoc "${filename}_pp.c" > "${filename}.s"
