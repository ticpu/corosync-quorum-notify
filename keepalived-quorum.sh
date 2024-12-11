#!/bin/bash
set -eu

# Usage: keepalived-quorum.sh quorate 3 1,2,3

result_file="${RUNTIME_DIRECTORY-/run}/corosync-quorum-status.txt"
quorum_state="$1"
current_node="$2"
node_list="${3},"

handle_error() {
    local exit_code=$?
    echo "Error occurred. Exit code: $exit_code"
    echo 1 > "$result_file"
    exit $exit_code
}

trap 'handle_error' ERR

if [[ "$quorum_state" != "quorate" ]]; then
    echo "Invalid quorum state: $quorum_state"
    echo 1 > "$result_file"
    exit 1
fi

contains_current_node=false

while read -d, node
do
    if [[ "$node" == "$current_node" ]]; then
        contains_current_node=true
        break
    fi
done <<< $node_list

if [[ "$contains_current_node" != "true" ]]; then
    echo "Current node $current_node missing from node list $node_list"
    echo 1 > "$result_file"
    exit 1
fi

echo 0 > "$result_file"
exit 0
