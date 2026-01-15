#ifndef __MY_PRED_H__
#define __MY_PRED_H__

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <climits>
#include <cstdlib>
#include <cassert>
#include <sstream>
#include <stdio.h>


#define GHR_LEN 64                          // Global history register length
#define GHR_MASK ((1ul << GHR_LEN) - 1)

#define PHR_LEN 64                          //Path History
#define PHR_MASK ((1ul << PHR_LEN) - 1)

#define FETURE_SIZE 1024
#define FETURE_COUNT 8


struct BranchMetadata {

    uint64_t    ghr_at_predict;
    uint64_t    phr_at_predict;
    int8_t      y_at_predict;

    uint64_t fetures_at_perdict[FETURE_COUNT];

};

class MyPred
{
private:

    uint64_t    _ghr;
    uint64_t    _phr;

    int8_t _history_lengths[FETURE_COUNT] = {0, 2, 4, 8, 10, 16, 24, 48};
    int8_t _fetures[FETURE_COUNT][FETURE_SIZE];


    uint8_t     _theta;
    int8_t      _tc;

    
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
