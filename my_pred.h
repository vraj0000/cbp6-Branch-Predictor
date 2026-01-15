#ifndef __MY_PRED_H__
#define __MY_PRED_H__

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <climits>
#include <cstdlib>

#define HR_LEN 10
#define paBHT_LEN 1024  // pre-address Branch Hitory Tablen 
#define HR_MASK ((1ul << HR_LEN) - 1)
#define PAg_PHT_SIZE (1 << HR_LEN)

#define GHR_LEN 32  // Global history register length
#define GHR_MASK ((1ul << GHR_LEN) - 1)

#define EX_GHR_LEN 12
#define EX_GHR_MASK ((1ul << EX_GHR_LEN) - 1)
#define GAg_PHT_SIZE (1 << EX_GHR_LEN)

#define Address_Bits 10

#define PHR_LEN 16      //Path History Register
#define PHR_MASK ((1ul << PHR_LEN) - 1)

struct BranchMetadata {
    uint32_t g_idx;       // Index for global_perceptron
    uint32_t l_idx;       // Index for local_perceptron
    uint32_t p_idx;       // Index for path_perceptron
    uint64_t ghr_bits;    // Actual bits to decide +/- weight update
    uint32_t lhist_bits;  // Actual bits to decide +/- weight update
    uint32_t phr_bits;    // Actual bits to decide +/- weight update
    uint32_t ex_ghr_idx;  // Index for GAg_pht
    int y_at_predict;
    bool pag_pred;
    bool gag_pred;
};

class MyPred
{
private:
    uint8_t theta = (1.93 * (GHR_LEN + HR_LEN + 1)) + 14;      // Threshole to decide tranning usally 1.93*(ghr) + 14

    int TC;             // Threshold Counter
    const int TC_MAX = 31;   // When to increment theta
    const int TC_MIN = -32;  // When to decrement theta

    int8_t global_perceptron[(1 << Address_Bits)][GHR_LEN + 1];   // +1 is for bias and global weight

    int8_t local_perceptron[(1 << Address_Bits)][HR_LEN];

    int8_t path_perceptron[(1 << Address_Bits)][PHR_LEN];

    uint64_t pa_ht[paBHT_LEN];
    uint8_t PAg_pht[PAg_PHT_SIZE];
    int8_t w_pag;

    uint64_t ghr;
    uint8_t ex_ghr_start_count;
    bool ghr_threshold;
    uint32_t ex_ghr;
    int8_t w_gag;
    uint8_t GAg_pht[GAg_PHT_SIZE];

    uint64_t phr;


    // this metadata is only to keep track of ghr values
    // to properly update the BP structures at execute stage.
    // Technically, this information could have been stored
    // along with the instruction itself. But since we don't
    // have access to such APIs, we are storing it by ourselves.
    std::unordered_map<uint64_t, BranchMetadata> br_hist;

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
