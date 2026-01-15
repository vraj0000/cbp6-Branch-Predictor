# #!/bin/bash
# 
# RESULT_DIR="results_pinned"
# TRACES=(
#     "cbp6_traces/web_1_trace.gz"
#     "cbp6_traces/int_0_trace.gz"
#     "cbp6_traces/int_1_trace.gz"
#     "cbp6_traces/int_2_trace.gz"
#     "cbp6_traces/int_3_trace.gz"
#     "cbp6_traces/int_4_trace.gz"
# )
# 
# # ⚠️ CHANGE THESE to your actual P-cores
# CORES=(0 1 2 3 4 5 6 7)
# 
# 
# 
# 
# mkdir -p "$RESULT_DIR"
# 
# START=$(date +%s)
# 
# echo "Running ${#TRACES[@]} jobs pinned to cores..."
# 
# for i in "${!TRACES[@]}"; do
#     FILE=${TRACES[$i]}
#     CORE=${CORES[$i]}
#     NAME=$(basename "$FILE" .gz)
#     OUT_FILE="$RESULT_DIR/${NAME}.txt"
# 
#     taskset -c "$CORE" ./cbp -S 10000000 "$FILE" > "$OUT_FILE" 2>&1 &
# done
# 
# wait
# 
# END=$(date +%s)
# echo "All pinned jobs complete."
# echo "Total wall time: $((END - START)) seconds"
### 85 sec

####
# 8 batch only 8 trace 77 for 10000000  
# 8 batch os sheduling = 12 sec
# 8 batch os sheduling and starting as soon as previous is done  = 1` sec


#!/bin/bash

RESULT_DIR="results_dynamic"
# Automatically find all .gz files in the cbp6_traces directory
TRACES=( cbp6_traces/*.gz )

# Safety check: ensure files actually exist
if [ ! -e "${TRACES[0]}" ]; then
    echo "Error: No .gz files found in cbp6_traces/"
    exit 1
fi

MAX_CONCURRENT=8
mkdir -p "$RESULT_DIR"

echo "Launching simulations with max $MAX_CONCURRENT concurrent jobs..."

running=0

START=$(date +%s)
for FILE in "${TRACES[@]}"; do
    NAME=$(basename "$FILE" .gz)
    OUT_FILE="$RESULT_DIR/${NAME}.txt"

    ./cbp -S 50000000 "$FILE" > "$OUT_FILE" 2>&1 &

    ((running++))

    # If we hit the limit, wait for any job to finish
    if ((running >= MAX_CONCURRENT)); then
        wait -n  # waits for *any* background job to finish
        ((running--))
    fi
done

# Wait for remaining jobs
wait
END=$(date +%s)
echo "Total wall time: $((END - START)) seconds"
echo "All simulations complete."



