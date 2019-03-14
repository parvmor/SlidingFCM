#include <cinttypes>
#include <deque>
#include <unordered_map>
#include <utility>

#include "cvp.h"

using namespace std;

#define HISTORY_LEN 256
#define VERIFY_LEN 4
#define MAX_INFLIGHT 256

struct entry_t {
    bool eligible;
    InstClass insn;
    uint64_t occurence_count;
    uint64_t inflight_count;
    // same as occurence_count - inflight_count
    uint64_t committed_count;
    deque<uint64_t> val_hist;
    deque<uint64_t> seq_hist;
    // pc : {{predicted, correct/incorrect/unhecked: [1/0/-1]}, predicted_value}
    unordered_map<uint64_t, pair<pair<bool, int>, uint64_t>> inflight_info;
    uint64_t correct_pred;
    uint64_t incorrect_pred;
    uint8_t prediction_result;
};

unordered_map<uint64_t, entry_t> pc_map;
unordered_map<uint64_t, uint64_t> seq_pc;
