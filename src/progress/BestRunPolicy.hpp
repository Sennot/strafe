#pragma once
/**
 * ghost::BestRunPolicy — strategy for the "Best Run" value.
 *
 * Why a strategy class: the user spec says startpos sessions have "slightly
 * different logic" that is not fully pinned down yet. Instead of if/else in
 * SessionState, the rules live here and can be swapped/redefined without
 * touching session bookkeeping. This is the designated extension point.
 */
#include <Geode/Geode.hpp>

namespace ghost {

class BestRunPolicy {
public:
    virtual ~BestRunPolicy() = default;

    /// Displayed right after a checkpoint is banked.
    virtual float afterCheckpoint(float banked, float /*liveMax*/) {
        return banked;
    }

    /// Displayed continuously while playing (real-time best run).
    /// Default: follows live progress upward from the bank (looks like one
    /// continuous run reaching a new best every frame).
    virtual float duringRun(float banked, float liveMax) {
        return liveMax > banked ? liveMax : banked;
    }

    /// Displayed after an un-banked death: snap back to the bank.
    virtual float afterDeath(float banked, float /*liveMax*/) {
        return banked;
    }

    virtual const char* name() const = 0;
};

/// Default: checkpoints bank progress, deaths fall back to the bank.
class BankingPolicy : public BestRunPolicy {
public:
    static BankingPolicy& instance() {
        static BankingPolicy p;
        return p;
    }
    const char* name() const override { return "banking"; }
};

/**
 * Startpos sessions (practice begun from a startpos at >0%).
 *
 * TODO(P4, user decision needed): pick one of the two candidate semantics:
 *   (a) bank/live values are absolute percents (identical to BankingPolicy);
 *   (b) values are "earned" percents: shown = absolute - baseline
 *       (a startpos at 30 that plays to 60 displays 30).
 * Until decided, StartposPolicy == BankingPolicy. Do not guess — ask.
 */
class StartposPolicy : public BankingPolicy {
public:
    static StartposPolicy& instance() {
        static StartposPolicy p;
        return p;
    }
    const char* name() const override { return "startpos"; }
};

} // namespace ghost
