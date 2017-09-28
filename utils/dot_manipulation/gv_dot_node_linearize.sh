#!/usr/bin/awk -f

# places all node definition and node relation statements of a graphviz DOT file
# each on a separate line

# assumptions
# 1. each input node statement (definition or relation) is the first string on a 
# line, ignoring leading whitespace
# 2. nothing follows an input node statement (definition or relation) that ends 
# with a semicolon

BEGIN {
  key_len=length("Node")
}

{
  if(substr($1, 0, key_len) ~ /Node/)
    in_node=1

  if(!in_node)
    print $0
  else {
    for(i=1; i <= NF; ++i)
      hold=hold" "$i

    lastfield_len=length($NF)
    if(substr($NF, lastfield_len, 1) == ";")
      has_term=1
  }

  if(in_node && has_term) {
    print hold
    hold=""
    in_node=0
    has_term=0
  }
}
  
