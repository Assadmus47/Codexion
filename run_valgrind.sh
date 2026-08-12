#!/bin/bash
BINARY="${1:-./codexion}"
timeout 180 valgrind \
    --leak-check=full \
    --show-leak-kinds=all \
    --track-origins=yes \
    --errors-for-leak-kinds=all \
    $BINARY 2000 200 2 2 2 3 0 edf > /dev/null
echo "exit: $?"