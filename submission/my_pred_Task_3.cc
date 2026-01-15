#include "my_pred.h"
#include <cassert>
#include <sstream>
#include <stdio.h>

// global variable
MyPred my_pred;

std::string MyPred::get_br_id(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    std::stringstream ss;
    ss << seq_no << piece << pc;
    return ss.str();
}

uint32_t MyPred::get_PAg_pht_index(uint64_t key)
{
    return key % PAg_PHT_SIZE;
}
uint32_t MyPred::get_GAg_pht_index(uint64_t key)
{
    return key % GAg_PHT_SIZE;
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
uint32_t fold_ghr_12bit(uint64_t ghr) {
    uint32_t result = 0;
    result ^= (ghr >> 0) & 0xFFF;  // Bits 0-11
    result ^= (ghr >> 12) & 0xFFF; // Bits 12-23
    return (result & 0xFFF);       // Ensure final output is 12-bit
}
void MyPred::init() 
{
    printf("PGa = %d \n", HR_LEN);
    for(int i = 0; i < paBHT_LEN; i++) pa_ht[i] = 0;
    for(int i = 0; i < PAg_PHT_SIZE; i++) PAg_pht[i] = 2; // Start at "Weakly Taken"
    
    ghr = 0;
    for(int i = 0; i < GAg_PHT_SIZE; i++) GAg_pht[i] = 2; // Start at "Weakly Taken"

    for(int i = 0; i < GAg_PHT_SIZE; i++) choice_table[i] = 2;

}

void MyPred::fini() {}

bool MyPred::predict(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    uint32_t pa_ht_index = fold_pc_10bit(pc);
    uint32_t PAg_index = get_PAg_pht_index(pa_ht[pa_ht_index]);
    assert(PAg_index < PAg_PHT_SIZE);

    uint32_t GAg_index = (ghr & GHR_MASK) ^ fold_pc_12bit(pc);
    assert(GAg_index < GAg_PHT_SIZE);

    uint32_t choice_index = GAg_index;
    // choice_index = choice_index & 0xFFF;
    assert(choice_index < GAg_PHT_SIZE);
    
    bool pag_pred = (PAg_pht[PAg_index] >= 2);
    bool gag_pred = (GAg_pht[GAg_index] >= 2);
    
    bool final_decision;
    
    if (pag_pred == gag_pred) {
        // They agree! Choice table doesn't matter.
        final_decision = pag_pred;
    } else {
        // They disagree! The Choice Table decides who to trust.
        bool trust_gag = (choice_table[choice_index] >= 2);
        final_decision = trust_gag ? gag_pred : pag_pred;
    }

    // SAVE METADATA
    BranchMetadata meta;
    meta.ghr_at_predict = GAg_index;
    meta.lhist_at_predict = PAg_index;
    meta.choice_table_at_predict = choice_index;
    meta.gag_pred = gag_pred;
    meta.pag_pred = pag_pred;

    std::string br_id = get_br_id(seq_no, piece, pc);
    br_hist[br_id] = meta; 

    return final_decision;
}

void MyPred::spec_update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc)
{
    //---------------------------------------------------------------------------------------//
    // Remember that the spec_update function is called right after the BP predicted for
    // the branch. In a real processor, you WON'T know the real outcome of the branch
    // (i.e., `resolve_dir` and `next_pc` arguments), at this point. It will only be
    // known AFTER the branch is executed (when the `update` function is called).
    // Then why do we provide you the `resolve_dir` and `next_pc` arguments here?
    // This is to update any path history structures that you may use to make
    // subsequent predictions, without taking complex branch recovery code into account.
    // For example, here we use the `resolve_dir` argument to update the GHR, which
    // will be used to make subsequent branch predictions.
    // But observe that: we *WILL NOT* use any of this information to update the PHT
    // at this stage. That will only happen at the `update` function call.
    //
    // If you are unsure whether your usage of `resolve_dir` and `next_pc`
    // in this spec_update function is valid or not, please email us.
    //---------------------------------------------------------------------------------------//
    uint32_t pa_ht_index = fold_pc_10bit (pc);
    pa_ht[pa_ht_index] <<= 1;
    pa_ht[pa_ht_index] &= HR_MASK;
    pa_ht[pa_ht_index] |= resolve_dir;

    ghr <<= 1;
    ghr &= GHR_MASK;
    ghr |= resolve_dir;
}

void MyPred::update(uint64_t seq_no, uint8_t piece, uint64_t pc, bool resolve_dir, bool pred_dir, uint64_t next_pc) 
{
    auto it = br_hist.find(get_br_id(seq_no, piece, pc));
    assert(it != br_hist.end());
    BranchMetadata meta = it->second;

    uint32_t GAg_idx = meta.ghr_at_predict;
    uint32_t PAg_idx = meta.lhist_at_predict;
    uint32_t choice_index = meta.choice_table_at_predict;

    // 1. Update GAg PHT
    if (resolve_dir) {
        if (GAg_pht[GAg_idx] < 3) GAg_pht[GAg_idx]++;
    } else {
        if (GAg_pht[GAg_idx] > 0) GAg_pht[GAg_idx]--;
    }

    // 2. Update PAg PHT
    if (resolve_dir) {
        if (PAg_pht[PAg_idx] < 3) PAg_pht[PAg_idx]++;
    } else {
        if (PAg_pht[PAg_idx] > 0) PAg_pht[PAg_idx]--;
    }

    // 3. Update Choice Table (The Tournament Logic)
    // Only update if the two predictors gave DIFFERENT results
    if (meta.gag_pred != meta.pag_pred) {
        if (meta.gag_pred == resolve_dir) {
            // GAg was right, PAg was wrong -> Increment toward GAg
            if (choice_table[choice_index] < 3) choice_table[choice_index]++;
        } else {
            // PAg was right, GAg was wrong -> Decrement toward PAg
            if (choice_table[choice_index] > 0) choice_table[choice_index]--;
        }
    }
}

void MyPred::commit(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    std::string br_id = get_br_id(seq_no, piece, pc);
    br_hist.erase(br_id);
}