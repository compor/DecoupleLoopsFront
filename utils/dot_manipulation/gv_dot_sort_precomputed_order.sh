#!/usr/bin/env bash

# sorts the node definition and node relation statmenets of a graphviz DOT file
# based on the order of nodes provided in the second input file

# assumptions
# 1. all node names must start with "Node" as it is mainly used to sort graph
# descriptions output by LLVM
# 2. all node definition and node relation statements are each in separate
# lines, ignoring initial and trailing whitespace

# how it works
# this script tries to find the first and last occurrence of the node statements
# and outputs any statements outside that range verbatim, while sorting anything
# in between using the first column i.e. the node names

[[ -z $1 ]] && echo "missing input DOT file" && exit 1
[[ -z $2 ]] && echo "missing input node order file" && exit 2
[[ -z $3 ]] && echo "missing output file" && exit 2

total=$(awk 'END { print NR }' $1)
before_first=$(awk '/^[[:space:]]*Node/ { print NR-1; exit; }' $1)
first=$((before_first+1))
last=$(awk '/^[[:space:]]*Node/ { found=1; next; }{ if(found) { print NR-1; exit; } }' $1)
leftover=$((total-last))

echo "total lines: $total"
echo "location before first pattern occurrence: $before_first"
echo "line of first pattern occurrence: $first"
echo "location of last pattern occurrence: $last"
echo "leftover lines after last pattern occurrence location: $leftover"

head -n $before_first $1 > $3

temp_file=$$.txt
sed -n -e "${first},${last}p" $1 > $temp_file

readarray order < $2

for i in ${order[@]}; do 
  awk -v key=$i '{ len=length(key); if(substr($1, 1, len) == key) print }' $temp_file >> $3
done

tail -n $leftover $1 >> $3

rm -rf $temp_file

exit $?

