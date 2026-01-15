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

# --- CONFIGURATION ---
RESULT_DIR="results_dynamic"
MAX_CONCURRENT=8
SIM_INSTRUCTIONS=20000000
TRACES=( cbp6_traces/*.gz )

# --- CLEANUP LOGIC ---
# This function kills all background processes started by this script if you Ctrl+C
cleanup() {
    echo -e "\n\n[!] Interrupt detected. Killing all background simulations..."
    # pkill -P $$ kills all child processes of the current script PID
    pkill -P $$
    echo "[!] Cleanup complete. Exiting."
    exit 1
}

# Trap SIGINT (Ctrl+C) and SIGTERM
trap cleanup SIGINT SIGTERM

# --- INITIALIZATION ---
if [ ! -e "${TRACES[0]}" ]; then
    echo "Error: No .gz files found in cbp6_traces/"
    exit 1
fi

mkdir -p "$RESULT_DIR"
TOTAL_TRACES=${#TRACES[@]}
running=0
completed=0

echo "Launching $TOTAL_TRACES simulations (Max $MAX_CONCURRENT concurrent)..."
START=$(date +%s)

# --- MAIN LOOP ---
for FILE in "${TRACES[@]}"; do
    NAME=$(basename "$FILE" .gz)
    OUT_FILE="$RESULT_DIR/${NAME}.txt"

    # Launch simulation in background
    ./cbp -S "$SIM_INSTRUCTIONS" "$FILE" > "$OUT_FILE" 2>&1 &
    
    ((running++))
    ((completed++))

    echo "[$completed/$TOTAL_TRACES] Started $NAME (Current jobs: $running)"

    # If we hit the max limit, wait for ANY one job to finish before continuing
    if ((running >= MAX_CONCURRENT)); then
        wait -n
        ((running--))
    fi
done

# Wait for the very last batch to finish
wait

END=$(date +%s)
DURATION=$((END - START))

echo "--------------------------------------"
echo "All simulations complete."
echo "Total wall time: $DURATION seconds"
echo "Results saved to: $RESULT_DIR/"