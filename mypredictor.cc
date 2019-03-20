#include <cassert>
#include <iostream>
#include <vector>

#include "mypredictor.h"

using namespace std;

entry_t new_entry()
{
    entry_t info;
    info.eligible = true;
    info.occurence_count = 1;
    info.inflight_count  = 1;
    info.committed_count = 0;
    info.correct_pred = 0;
    info.incorrect_pred = 0;
    info.prediction_result = 2;
    info.timestamp = timestamp++;
    return info;
}


bool get_inflight_pred(entry_t &info, uint64_t seq_no)
{
    uint64_t val_last[VERIFY_LEN];
    int i = VERIFY_LEN - 1;
    deque<uint64_t>::reverse_iterator rit = info.val_hist.rbegin();
    while (i >= 0) {
        if (rit == info.val_hist.rend()) {
            cerr << "rit was at the end of value history\n";
            exit(EXIT_FAILURE);
        }
        val_last[i--] = *rit;
        rit++;
    }
    deque<uint64_t>::iterator bit = info.val_hist.begin();
    i = int(info.val_hist.size()) - VERIFY_LEN - int(info.inflight_count) - 2;
    if (info.val_hist.size() - int(info.inflight_count) - 2 > HISTORY_LEN) {
        cerr << info.val_hist.size() << ' ' << HISTORY_LEN << ' ' << info.inflight_count << '\n';
    }
    bit += i;
    // do a naive search as of now
    // can be optimized later on
    while (i >= 0) {
        deque<uint64_t>::iterator it = bit;
        bool match = true;
        int j = 0;
        while (j < VERIFY_LEN) {
            if (it == info.val_hist.end()) {
                cerr << "it was at the end of value history\n";
                exit(EXIT_FAILURE);
            }
            if (val_last[j] != *it) {
                match = false;
                break;
            }
            j++;
            it++;
        }
        if (match) {
            it += info.inflight_count;
            if (it >= info.val_hist.end()) {
                cerr << "it was at the end of value history\n";
                exit(EXIT_FAILURE);
            }
            info.inflight_info[seq_no] = {{true, -1}, *it};
            return true;
        }
        i--;
        bit--;
    }
    return false;
}


bool getPrediction(uint64_t seq_no, uint64_t pc, uint8_t piece, uint64_t& predicted_value)
{
    pc += piece;
    seq_pc[seq_no] = pc;
    // pc encountered for the first time
    if (pc_map.find(pc) == pc_map.end()) {
        entry_t info = new_entry();
        info.inflight_info[seq_no].first = {false, -1};
        pc_map[pc] = info;
        return false;
    }

    entry_t &info = pc_map[pc];
    // pc not eligible for value prediction
    // eg. store, branch etc.
    if (!pc_map[pc].eligible) {
        goto finish;
    }

    // still building the sliding window
    if (info.committed_count < HISTORY_LEN) {
        goto finish;
    }

    // predict using the maintained val_history (the sliding window)
    // prediction is done if the sliding window is matched
    // an alternative would be to take into account number of correct and incorrect predictions
    if (get_inflight_pred(info, seq_no)) {
        predicted_value = info.inflight_info[seq_no].second;
        if (info.inflight_info[seq_no].first.first) {
            info.occurence_count++;
            info.inflight_count++;
            info.inflight_info[seq_no] = {{true, -1}, predicted_value};
            return true;
        }
    }
finish:
    info.occurence_count++;
    info.inflight_count++;
    info.inflight_info[seq_no].first = {false, -1};
    return false;
}


void speculativeUpdate(uint64_t seq_no, bool eligible, uint8_t prediction_result,
                       uint64_t pc, uint64_t next_pc, InstClass insn, uint8_t piece,
                       uint64_t src1, uint64_t src2, uint64_t src3, uint64_t dst)
{
    pc += piece;
    assert(pc_map.find(pc) != pc_map.end());
    if (!eligible) {
        if (prediction_result != 2) {
            cerr << "Predicted a non-predictable pc\n";
            exit(EXIT_FAILURE);
        }
        pc_map.erase(pc);
        return;
    }
    // @Arpit had some predictable logic
    // I don't understand the point because `eligible` is already telling that
    entry_t &info = pc_map[pc];
    info.insn = insn;
    info.prediction_result = prediction_result;
    info.eligible = eligible;
    switch (prediction_result) {
    case 0:
        // incorrect prediction
        assert(info.inflight_info[seq_no].first.first);
        info.incorrect_pred++;
        break;
    case 1:
        // correct prediction
        assert(info.inflight_info[seq_no].first.first);
        info.correct_pred++;
        break;
    case 2:
        // chose not to predict
        assert(!info.inflight_info[seq_no].first.first);
        break;
    default:
        cerr << "Unknown value of result\n";
        exit(EXIT_FAILURE);
    }
}


void updatePredictor(uint64_t seq_no, uint64_t actual_addr,
                     uint64_t actual_value, uint64_t actual_latency)
{
    uint64_t pc = seq_pc[seq_no];
    if (pc_map.find(pc) == pc_map.end()) {
        seq_pc.erase(seq_no);
        return;
    }
    entry_t &info = pc_map[pc];
    info.inflight_count--;
    info.committed_count++;
    info.timestamp = timestamp++;
    info.val_hist.push_back(actual_value);
    info.seq_hist.push_back(seq_no);
    if (info.val_hist.size() > HISTORY_LEN) {
        info.val_hist.pop_front();
        info.seq_hist.pop_front();
    }
    info.prediction_result = 2;
    info.inflight_info.erase(seq_no);
    seq_pc.erase(seq_no);
    if (info.incorrect_pred >= 3 && info.correct_pred < 50000 * info.incorrect_pred) {
        pc_map.erase(pc);
    }
    if ((timestamp % AGE_PERIOD) == 0) {
        vector<uint64_t> keys;
        for (auto &kv: pc_map) {
            entry_t &info = kv.second;
            if (timestamp - info.timestamp > AGE_THRESHOLD) {
                keys.push_back(kv.first);
            }
        }
        for (uint64_t pc: keys) {
            pc_map.erase(pc);
        }
    }
}


void beginPredictor(int argc_other, char **argv_other)
{
    timestamp = 0;
}


void endPredictor()
{
}
