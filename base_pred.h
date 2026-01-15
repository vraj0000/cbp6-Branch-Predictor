#ifndef __FOCA_BIMODAL_H__
#define __FOCA_BIMODAL_H__

#include <cstdint>
#include <cassert>
#include <stdio.h>

#define BIMODAL_TABLE_SIZE (16 * 1024)
#define MAX_COUNTER_VAL 3

//-------------------------------------------------------------------//
// This is a sample hash function. This takes a 64-b key,
// creates two halves of 32-b each, and XORs the halves.
// This simple hash function is often used in many real processors
// to hash down big values to index tables, structures, etc.
//-------------------------------------------------------------------//
uint32_t folded_xor(uint64_t value, uint32_t num_folds)
{
    assert(num_folds > 1);
    assert((num_folds & (num_folds - 1)) == 0); /* has to be power of 2 */
    uint32_t mask = 0;
    uint32_t bits_in_fold = 64 / num_folds;
    if (num_folds == 2)
    {
        mask = 0xffffffff;
    }
    else
    {
        mask = (1ul << bits_in_fold) - 1;
    }
    uint32_t folded_value = 0;
    for (uint32_t fold = 0; fold < num_folds; ++fold)
    {
        folded_value = folded_value ^ ((value >> (fold * bits_in_fold)) & mask);
    }
    return folded_value;
}

class BimodalPred
{
private:
    uint8_t bimodal_table[BIMODAL_TABLE_SIZE];

    uint32_t get_index(uint64_t, uint8_t, uint64_t);

public:
    BimodalPred() {}
    ~BimodalPred() {}

    // interface functions
    void init();
    void fini();
    bool predict(uint64_t seq_no, uint8_t piece, uint64_t pc);
    void spec_update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc);
    void update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc);
};

uint32_t BimodalPred::get_index(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    return folded_xor(pc, 2) % BIMODAL_TABLE_SIZE;
}

void BimodalPred::init() 
{
    printf("Base Bimodal resolve_dir %d \n", BIMODAL_TABLE_SIZE);
}
void BimodalPred::fini() {}

bool BimodalPred::predict(uint64_t seq_no, uint8_t piece, uint64_t pc)
{
    uint32_t index = get_index(seq_no, piece, pc);
    assert(index < BIMODAL_TABLE_SIZE);
    bool pred = bimodal_table[index] >= ((MAX_COUNTER_VAL + 1) / 2) ? true : false;
    return pred;
}

void BimodalPred::spec_update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc)
{
    // nothing
}

void BimodalPred::update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc)
{
    uint32_t index = get_index(seq_no, piece, pc);
    assert(index < BIMODAL_TABLE_SIZE);

    if (resolve_dir)
    {
        bimodal_table[index] = bimodal_table[index] < MAX_COUNTER_VAL ? bimodal_table[index] + 1 : bimodal_table[index];
    }
    else
    {
        bimodal_table[index] = bimodal_table[index] ? bimodal_table[index] - 1 : bimodal_table[index];
    }
}

#endif

// global variable
static BimodalPred base_bp;