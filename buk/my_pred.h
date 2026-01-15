#ifndef __MY_PRED_H__
#define __MY_PRED_H__

#include <cstdint>
#include <string>
#include <unordered_map>

#define GHR_LEN 18
#define GHR_MASK ((1ul << GHR_LEN) - 1)
#define PHT_SIZE (1 << GHR_LEN)

//--------------------------------------------------------//
// This implements a two-level global branch predictor
// as we covered in the lecture.
//--------------------------------------------------------//
class MyPred
{
private:
    uint64_t ghr;
    uint8_t pht[PHT_SIZE];

    // this metadata is only to keep track of ghr values
    // to properly update the BP structures at execute stage.
    // Technically, this information could have been stored
    // along with the instruction itself. But since we don't
    // have access to such APIs, we are storing it by ourselves.
    std::unordered_map<std::string, uint64_t> br_hist;

    std::string get_br_id(uint64_t, uint8_t, uint64_t);
    uint32_t get_pht_index(uint64_t);

public:
    MyPred() {}
    ~MyPred() {}

    // interface functions
    void init();
    void fini();
    bool predict(uint64_t seq_no, uint8_t piece, uint64_t pc);
    void spec_update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc);
    void update(uint64_t seq_no, uint8_t piece, uint64_t pc, const bool resolve_dir, const bool pred_dir, const uint64_t next_pc);
    void commit(uint64_t seq_no, uint8_t piece, uint64_t pc);
};

#endif

// global variable
extern MyPred my_pred;
