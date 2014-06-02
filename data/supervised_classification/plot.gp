set term png
do for [f=1:9] {
  do for [c in "2 4"] {
    base = sprintf("exp%d_n%s", f, c)
    set output sprintf("%s.png", base)
    plot sprintf("%s.txt", base) using 2:3:1 with points palette title base
  }
}

