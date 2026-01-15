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

uint32_t MyPred::get_pht_index(uint64_t key)
{
    return key % PHT_SIZE;
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

void MyPred::init() 
{
    printf("PGa = %d \n", HR_LEN);
    for(int i = 0; i < paBHT_LEN; i++) pa_ht[i] = 0;
    for(int i = 0; i < PHT_SIZE; i++) pht[i] = 2; // Start at "Weakly Taken"
    printf("HR_LEN = %d \n", HR_LEN);
}

void MyPred::fini() {}

bool MyPred::predict(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    uint32_t pa_ht_index = fold_pc_10bit(pc);
    uint32_t index = get_pht_index(pa_ht[pa_ht_index]);
    assert(index < PHT_SIZE);

    std::string br_id = get_br_id(seq_no, piece, pc);
    br_hist.insert(std::pair<std::string, uint64_t>(br_id, pa_ht[pa_ht_index]));

    return (pht[index] >= 2);
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
}

void MyPred::update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc)
{
    std::string br_id = get_br_id(seq_no, piece, pc);
    auto it = br_hist.find(br_id);
    assert(it != br_hist.end());

    uint64_t ghr_to_use = it->second;
    uint32_t index = get_pht_index(ghr_to_use);
    assert(index < PHT_SIZE);

    if (resolve_dir)
    {
        if (pht[index] < 3)
            pht[index]++;
    }
    else
    {
        if (pht[index])
            pht[index]--;
    }
}

void MyPred::commit(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    std::string br_id = get_br_id(seq_no, piece, pc);
    br_hist.erase(br_id);
}