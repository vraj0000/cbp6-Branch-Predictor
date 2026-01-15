#include "my_pred.h"
#include <cassert>
#include <sstream>
#include <stdio.h>

// global variable
MyPred my_pred;

uint32_t MyPred::get_PAg_pht_index(uint64_t key)
{
    return key % PAg_PHT_SIZE;
}

uint32_t MyPred::get_GAg_pht_index(uint64_t key)
{
    return key % GAg_PHT_SIZE;
}

// Helper for saturation logic
int8_t sat_update(int8_t weight, int delta) {
    int val = weight + delta;
    if (val > 127) return 127;
    if (val < -128) return -128;
    return (int8_t)val;
}
uint32_t fold_pc_12bit(uint64_t pc) {
    uint32_t result = 0;
    result ^= (pc >> 0)  & 0xFFF;  // Bits 0-11
    result ^= (pc >> 12) & 0xFFF;  // Bits 12-23
    result ^= (pc >> 24) & 0xFFF;  // Bits 24-35
    result ^= (pc >> 36) & 0xFFF;  // Bits 36-47
    result ^= (pc >> 48) & 0xFFF;  // Bits 48-59
    result ^= (pc >> 60) & 0xF;    // Bits 60-63 (Remaining 4 bits)
    
    return (result & 0xFFF);       // Ensure final output is 12-bit
}

uint32_t fold_pc_10bit(uint64_t pc) {
    uint32_t result = 0;
    result ^= (pc >> 0) & 0x3FF;   // Bits 0-9
    result ^= (pc >> 10) & 0x3FF;  // Bits 10-19
    result ^= (pc >> 20) & 0x3FF;  // Bits 20-29
    result ^= (pc >> 30) & 0x3FF;  // Bits 30-39
    result ^= (pc >> 40) & 0x3FF;  // Bits 40-49
    result ^= (pc >> 50) & 0x3FF;  // Bits 50-59
    result ^= (pc >> 60) & 0xF;    // Bits 60-63 (only 4 bits left)
    return (result & 0x3FF);
}

uint8_t fold_pc_8bit(uint64_t pc) {
    uint64_t hash = pc;
    
    // Shift and XOR to compress 64 bits into 32
    hash ^= (hash >> 32);
    // Compress 32 bits into 16
    hash ^= (hash >> 16);
    // Compress 16 bits into 8
    hash ^= (hash >> 8);
    
    return (uint8_t)(hash & 0xFF);
}

void MyPred::init() 
{
    printf(" \n A pergeptron with GAG extending the history by 10 bits \n");
    printf("perceptron with gshare \n Address = %d \n GHR_LEN = %d \n PAg_HL_LEN = %d \n PHR_LEN = %d \n Theta = %d \n EX_GHR = %d", 
            Address_Bits, GHR_LEN, HR_LEN, PHR_LEN, theta, ex_ghr);
    w_pag = 0;
    for(int i = 0; i < paBHT_LEN; i++) pa_ht[i] = 0;
    for(int i = 0; i < PAg_PHT_SIZE; i++) PAg_pht[i] = 2; // Start at "Weakly Taken"

    ghr = 0;
    // Initialize perceptron weights
    for (int i = 0; i < (1 << Address_Bits); i++) {
        for (int j = 0; j <= GHR_LEN; j++) {
            global_perceptron[i][j] = 0;
        }
        for (int j = 0; j < HR_LEN; j++) {
            local_perceptron[i][j] = 0;
        }
        for (int j = 0; j < PHR_LEN; j++) {
            path_perceptron[i][j] = 0;
        }
    }
    phr = 0;

    ex_ghr_start_count = 0;
    ghr_threshold = false;
    w_gag = 0;
}

void MyPred::fini() {

    printf("\n Theta = %d \n", theta);
}

bool MyPred::predict(uint64_t seq_no, uint8_t piece, uint64_t pc) {
    if (!ghr_threshold) {
        ex_ghr_start_count++;
        if (ex_ghr_start_count >= 28) ghr_threshold = true;
    }

    // --- 1. PERSPECTIVE INDEXING (Fixing Aliasing) ---
    uint32_t pc_10 = fold_pc_10bit(pc);
    
    // Global Index: PC XORed with most recent GHR bits
    uint32_t g_idx = pc_10 ^ (ghr & 0x3FF);
    
    // Local Index: PC XORed with the local history bits
    uint32_t pa_ht_index = pc_10; 
    uint32_t local_history = pa_ht[pa_ht_index] & HR_MASK;
    uint32_t l_idx = pc_10 ^ local_history;
    
    // Path Index: PC XORed with the path history (Target addresses)
    uint32_t p_idx = pc_10 ^ (phr & 0x3FF);
    
    // Sub-expert Indices
    uint32_t PAg_index = get_PAg_pht_index(local_history);
    uint32_t GAg_index = get_GAg_pht_index(ex_ghr);

    // --- 2. NEURAL SUMMATION ---
    int8_t *g_w = global_perceptron[g_idx];
    int8_t *l_w = local_perceptron[l_idx];
    int8_t *p_w = path_perceptron[p_idx];
    
    int y = g_w[0]; // Global Bias
    
    for (int i = 0; i < GHR_LEN; i++) {
        int32_t mask = ((ghr >> i) & 1) - 1;
        y += (g_w[i + 1] ^ mask) - mask;
    }
    // for (int i = 0; i < HR_LEN; i++) {
    //     int32_t mask = ((local_history >> i) & 1) - 1;
    //     y += (l_w[i] ^ mask) - mask;
    // }
    // for (int i = 0; i < PHR_LEN; i++) {
    //     int32_t mask = ((phr >> i) & 1) - 1;
    //     y += (p_w[i] ^ mask) - mask;
    // }

    // // Sub-expert Votes
    // bool pag_pred = (PAg_pht[PAg_index] >= 2);
    // y += (pag_pred ? 1 : -1) * w_pag; // Simplified logic

    // bool gag_pred = (GAg_pht[GAg_index] >= 2);
    // if (ghr_threshold) {   
    //     y += (gag_pred ? 1 : -1) * w_gag;
    // }
    
    // --- 3. METADATA (Storing indices for consistent update) ---
    BranchMetadata meta;
    meta.g_idx = g_idx;
    meta.l_idx = l_idx;
    meta.p_idx = p_idx;
    meta.ghr_bits = ghr;
    meta.lhist_bits = local_history;
    meta.phr_bits = phr;
    meta.y_at_predict = y;
    // meta.pag_pred = pag_pred;
    // meta.gag_pred = gag_pred;
    meta.ex_ghr_idx = GAg_index;
    br_hist[seq_no] = meta;

    return (y >= 0);
}
void MyPred::spec_update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc)
{
    // Update Local History
    uint32_t pa_ht_index = fold_pc_10bit(pc);
    pa_ht[pa_ht_index] = ((pa_ht[pa_ht_index] << 1) | resolve_dir) & HR_MASK;

    // Capture overflow bit for the ancient history chain
    uint32_t oldest_bit = (ghr >> 27) & 1;

    // Update Global History
    ghr = ((ghr << 1) | resolve_dir) & GHR_MASK;

    // Shift oldest bit into extended history
    if (ghr_threshold) {
        ex_ghr = ((ex_ghr << 1) | oldest_bit) & EX_GHR_MASK;
    }

    // Update Path History
    phr = ((phr << 2) ^ ((pc >> 2) & 0x3)) & PHR_MASK;
}

void MyPred::update(uint64_t seq_no, uint8_t piece, uint64_t pc, bool resolve_dir, bool pred_dir, uint64_t next_pc) 
{
    auto it = br_hist.find(seq_no);
    if(it == br_hist.end()) return;
    BranchMetadata meta = it->second;

    bool mispredicted = (pred_dir != resolve_dir);
    int t = resolve_dir ? 1 : -1;

    if (mispredicted || abs(meta.y_at_predict) <= theta) {
        int8_t *g_w = global_perceptron[meta.g_idx];
        int8_t *l_w = local_perceptron[meta.l_idx];
        int8_t *p_w = path_perceptron[meta.p_idx];

        g_w[0] = sat_update(g_w[0], t);
        for (int i = 0; i < GHR_LEN; i++) {
            int x_i = ((meta.ghr_bits >> i) & 1) ? 1 : -1;
            g_w[i+1] = sat_update(g_w[i+1], x_i * t);
        }
        for (int i = 0; i < HR_LEN; i++) {
            int x_i = ((meta.lhist_bits >> i) & 1) ? 1 : -1;
            l_w[i] = sat_update(l_w[i], x_i * t);
        }
        for (int i = 0; i < PHR_LEN; i++) {
            int x_i = ((meta.phr_bits >> i) & 1) ? 1 : -1;
            p_w[i] = sat_update(p_w[i], x_i * t);
        }

        w_pag = sat_update(w_pag, (meta.pag_pred ? 1 : -1) * t);
        if(ghr_threshold) w_gag = sat_update(w_gag, (meta.gag_pred ? 1 : -1) * t);
    }
    // --- DYNAMIC THRESHOLD LOGIC ---
    if (mispredicted) {
        TC++;
        if (TC == TC_MAX) {
            TC = 0;
            theta++; // Increase threshold to train more aggressively
        }
    } else if (abs(meta.y_at_predict) <= theta) {
        TC--;
        if (TC == TC_MIN) {
            TC = 0;
            theta--; // Decrease threshold to avoid over-training
        }
    }

    uint32_t PAg_idx = get_PAg_pht_index(meta.lhist_bits);
    if (resolve_dir) {
        if (PAg_pht[PAg_idx] < 3) PAg_pht[PAg_idx]++;
    } else {
        if (PAg_pht[PAg_idx] > 0) PAg_pht[PAg_idx]--;
    }

    if(ghr_threshold)
    {
        if (resolve_dir) {
            if (GAg_pht[meta.ex_ghr_idx] < 3) GAg_pht[meta.ex_ghr_idx]++;
        } else {
            if (GAg_pht[meta.ex_ghr_idx] > 0) GAg_pht[meta.ex_ghr_idx]--;
        }
    }
}

void MyPred::commit(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    // std::string br_id = get_br_id(seq_no, piece, pc);
    br_hist.erase(seq_no);
}