#!/usr/bin/env bash

[[ -z $1 ]] && echo "missing input file" && exit 1

CUR_DIR=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)

DOT_PREFIX=${1%.dot}

TMP_ORDER_FILE=${DOT_PREFIX}".order"
TMP_DOT_FILE1=${DOT_PREFIX}".tmp1.dot"
TMP_DOT_FILE2=${DOT_PREFIX}".tmp2.dot"

DOT_OUTPUT=${DOT_PREFIX}".final.dot"
PDF_OUTPUT=${DOT_PREFIX}".final.pdf"

trap "rm -rf ${TMP_DOT_FILE1} ${TMP_DOT_FILE2} ${TMP_ORDER_FILE}" EXIT

${CUR_DIR}/gv_dot_get_order.sh ${1} ${TMP_ORDER_FILE} && \
${CUR_DIR}/gv_dot_iterator_colorize.py ${1} ${TMP_DOT_FILE1} && \
${CUR_DIR}/gv_dot_node_linearize.sh ${TMP_DOT_FILE1} > ${TMP_DOT_FILE2} && \
${CUR_DIR}/gv_dot_sort_precomputed_order.sh ${TMP_DOT_FILE2} ${TMP_ORDER_FILE} ${DOT_OUTPUT} && \
dot -Tpdf ${DOT_OUTPUT} -o ${PDF_OUTPUT}

exit $?

