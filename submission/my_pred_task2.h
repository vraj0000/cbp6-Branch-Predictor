#ifndef __MY_PRED_H__
#define __MY_PRED_H__

#include <cstdint>
#include <string>
#include <unordered_map>

#define HR_LEN 10
#define paBHT_LEN 1024  // pre-address Branch Hitory Tablen 
#define HR_MASK ((1ul << HR_LEN) - 1)
#define PAg_PHT_SIZE (1 << HR_LEN)

#define GHR_LEN 12
#define GHR_MASK ((1ul << GHR_LEN) - 1)
#define GAg_PHT_SIZE (1 << GHR_LEN)

#define Choice_Pre_Size 4096

struct BranchMetadata {
    uint32_t ghr_at_predict;      // To index GAg_pht
    uint32_t lhist_at_predict;    // To index PAg_pht
    bool gag_pred;                // What GAg predicted
    bool pag_pred;                // What PAg predicted
};

//--------------------------------------------------------//
// This implements a two-level global branch predictor
// as we covered in the lecture.
//--------------------------------------------------------//
class MyPred
{
private:
    uint64_t pa_ht[paBHT_LEN];
    uint8_t PAg_pht[PAg_PHT_SIZE];

    uint64_t ghr;
    uint8_t GAg_pht[GAg_PHT_SIZE];

    uint8_t choice_table[Choice_Pre_Size];



    // this metadata is only to keep track of ghr values
    // to properly update the BP structures at execute stage.
    // Technically, this information could have been stored
    // along with the instruction itself. But since we don't
    // have access to such APIs, we are storing it by ourselves.
    std::unordered_map<std::string, BranchMetadata> br_hist;

    std::string get_br_id(uint64_t, uint8_t, uint64_t);
    uint32_t get_PAg_pht_index(uint64_t);
    uint32_t get_GAg_pht_index(uint64_t);
    uint32_t get_pa_ht_index(uint64_t, uint8_t, uint64_t);


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
