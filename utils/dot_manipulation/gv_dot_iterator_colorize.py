#!/usr/bin/env python

#
#
#

import sys
import os
import networkx as nx
from networkx.drawing.nx_agraph import read_dot
from networkx.drawing.nx_agraph import write_dot
from networkx.drawing.nx_agraph import graphviz_layout


def gv_dot_iterator_colorize(infile, outfile):
    if os.path.exists(outfile):
        raise ValueError("Output file already exists")

    dot_graph = read_dot(infile)

    for n in dot_graph.nodes():
        if dot_graph.node[n]['label'].startswith('{it_'):
            dot_graph.node[n]['style'] = 'filled'
            dot_graph.node[n]['fillcolor'] = 'aquamarine'
        elif dot_graph.node[n]['label'].startswith('{pd_'):
            dot_graph.node[n]['style'] = 'filled'
            dot_graph.node[n]['fillcolor'] = 'coral'
        else:
            dot_graph.node[n]['style'] = 'filled'
            dot_graph.node[n]['fillcolor'] = 'gray'

    write_dot(dot_graph, outfile)


if __name__ == '__main__':
    if len(sys.argv) < 3:
        sys.stderr.write("Usage: %s [infile] [outfile]\n" % sys.argv[0])

    gv_dot_iterator_colorize(sys.argv[1], sys.argv[2])
