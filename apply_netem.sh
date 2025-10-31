#!/bin/sh
# apply_netem.sh
# args: $1 = perte %, $2 = corruption %, $3 = duplication %
tc qdisc add dev eth0 root netem loss ${1:-0}% corrupt ${2:-0}% duplicate ${3:-0}%
echo "Netem applied: loss=$1%, corrupt=$2%, duplicate=$3%"
