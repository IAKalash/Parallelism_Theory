#!/bin/bash

mkdir -p build && cd build
cmake ..
make -j $(nproc)
cd ..

EXEC="./build/task3_schedule_test"
N=1050
THREADS=8
RESULTS_FILE="schedule_results.csv"

echo "Schedule,Chunk_Size,Time_V1,Time_V2" > $RESULTS_FILE

export OMP_NUM_THREADS=$THREADS
echo "Testing schedules with N=$N, Threads=$THREADS..."

# Test STATIC
for chunk in "" "1" "10" "100" "500"; do
    if [ -z "$chunk" ]; then
        export OMP_SCHEDULE="static"
        name="static(default)"
    else
        export OMP_SCHEDULE="static,$chunk"
        name="static($chunk)"
    fi
    echo -n "Running $name... "
    output=$($EXEC $N)
    time_v1=$(echo $output | cut -d',' -f2)
    time_v2=$(echo $output | cut -d',' -f3)
    echo "$name,${chunk:-default},$time_v1,$time_v2" >> $RESULTS_FILE
    echo "Done."
done

# Test DYNAMIC
for chunk in "1" "10" "100" "500"; do
    export OMP_SCHEDULE="dynamic,$chunk"
    name="dynamic($chunk)"
    echo -n "Running $name... "
    output=$($EXEC $N)
    time_v1=$(echo $output | cut -d',' -f2)
    time_v2=$(echo $output | cut -d',' -f3)
    echo "$name,$chunk,$time_v1,$time_v2" >> $RESULTS_FILE
    echo "Done."
done

# Test GUIDED
for chunk in "1" "10" "100" "500"; do
    export OMP_SCHEDULE="guided,$chunk"
    name="guided($chunk)"
    echo -n "Running $name... "
    output=$($EXEC $N)
    time_v1=$(echo $output | cut -d',' -f2)
    time_v2=$(echo $output | cut -d',' -f3)
    echo "$name,$chunk,$time_v1,$time_v2" >> $RESULTS_FILE
    echo "Done."
done

echo "Tests completed. Results saved to $RESULTS_FILE"
