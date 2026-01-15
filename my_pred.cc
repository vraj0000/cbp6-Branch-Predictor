#include "my_pred.h"

// global variable
MyPred my_pred;


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

uint32_t fold_10bit(uint64_t feture) {
    uint32_t result = 0;
    result ^= (feture >> 0) & 0x3FF;   // Bits 0-9
    result ^= (feture >> 10) & 0x3FF;  // Bits 10-19
    result ^= (feture >> 20) & 0x3FF;  // Bits 20-29
    result ^= (feture >> 30) & 0x3FF;  // Bits 30-39
    result ^= (feture >> 40) & 0x3FF;  // Bits 40-49
    result ^= (feture >> 50) & 0x3FF;  // Bits 50-59
    result ^= (feture >> 60) & 0xF;    // Bits 60-63 (only 4 bits left)
    return (result & 0x3FF);
}

uint8_t fold_pc_8bit(uint64_t pc) {
    uint64_t hash = pc;
    
    hash ^= (hash >> 32);
    hash ^= (hash >> 16);
    hash ^= (hash >> 8);
    
    return (uint8_t)(hash & 0xFF);
}

void MyPred::init() 
{
    printf("Feture Learning \n");
    printf("Feture Count = %d \n\n", FETURE_COUNT);


    _ghr    = 0;
    _phr    = 0;
    _theta  = (1.93 * FETURE_COUNT) + 14;
    _tc     = 0;


}

void MyPred::fini() {

    printf("\n Theta = %d \n", _theta);
}

bool MyPred::predict(uint64_t seq_no, uint8_t piece, uint64_t pc) {
    
    int y = 0;
    BranchMetadata meta;
    meta.ghr_at_predict = _ghr;
    meta.phr_at_predict = _phr;

    for (int i = 0; i < FETURE_COUNT; i++) {
        uint8_t len = _history_lengths[i];
        uint64_t mask = (len == 0) ? 0 : (0xFFFFFFFFFFFFFFFFULL >> (64 - len));
        
        // Index = PC XOR Folded(GHR_segment) XOR PathHistory
        // Adding PHR helps solve the "worse than Tournament" issue you saw earlier
        uint32_t idx = fold_10bit(pc ^ (_ghr & mask) ^ (_phr >> i));
        
        meta.fetures_at_perdict[i] = idx;
        y += _fetures[i][idx];
    }

    meta.y_at_predict = y;
    br_hist[seq_no] = meta;

    return (y>0);
}
void MyPred::spec_update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc)
{

    _ghr = ((_ghr << 1) | resolve_dir) & GHR_MASK;

    _phr = ((_phr << 2) ^ ((pc >> 2) & 0x3)) & PHR_MASK;
}

void MyPred::update(uint64_t seq_no, uint8_t piece, uint64_t pc, bool resolve_dir, bool pred_dir, uint64_t next_pc) 
{

    auto it = br_hist.find(seq_no);
    if(it == br_hist.end()) return;
    BranchMetadata meta = it->second;

    bool mispredicted = (pred_dir != resolve_dir);
    int t = resolve_dir ? 1 : -1;

    if (mispredicted || abs(meta.y_at_predict) <= _theta) {
        for (int i = 0; i < FETURE_COUNT; i++) {
            uint32_t idx = meta.fetures_at_perdict[i];
            _fetures[i][idx] = sat_update(_fetures[i][idx], t);
        }
    }

    if (mispredicted) {
        _tc++;
        if (_tc == 64) { _tc = 0; _theta++; }
    } else if (abs(meta.y_at_predict) <= _theta) {
        _tc--;
        if (_tc == -64) { _tc = 0; _theta--; }
    }

}

void MyPred::commit(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    // std::string br_id = get_br_id(seq_no, piece, pc);
    br_hist.erase(seq_no);
}