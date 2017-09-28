#!/usr/bin/env bash

# preserves the initial order of node definition and node relation statmenets 
# of a graphviz DOT file, specifically useful for LLVM dot utilities output

# assumptions
# 1. all node names must start with "Node" as it is mainly used to sort graph
# descriptions output by LLVM
# 2. all node definition and node relation statements are each in separate
# lines, ignoring initial and trailing whitespace

# how it works
# this script tries to find the first and last occurrence of the node statements
# and outputs any statements outside that range verbatim, while sorting anything
# in between using the first column i.e. the node names

[[ -z $1 ]] && echo "missing input file" && exit 1
[[ -z $2 ]] && echo "missing output file" && exit 2

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

sed -n -e "${first},${last}p" $1 | awk '{ print $1 }' | awk 'BEGIN { FS=":" } { print $1 }' | awk '!x[$0]++' > $2

exit $?

