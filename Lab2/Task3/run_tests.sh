#!/bin/bash

mkdir -p build && cd build
cmake ..
make -j $(nproc)
cd ..

EXEC="./build/task3"

N=1040

RESULTS_FILE="results.csv"
echo "" > $RESULTS_FILE

echo "Running scaling experiments for N=$N..."
echo "Results will be saved to $RESULTS_FILE"

export OMP_NUM_THREADS=1
echo -n "Running with $OMP_NUM_THREADS thread... "
$EXEC $N >> $RESULTS_FILE
echo "Done."

for t in 2 4 7 8 16 20 40 80; do
    export OMP_NUM_THREADS=$t
    echo -n "Running with $OMP_NUM_THREADS threads... "
    $EXEC $N >> $RESULTS_FILE
    echo "Done."
done
