#pragma once
/**
 * ghost::CheckpointBank — the stack of banked checkpoints for the current
 * session. Owns our ExtendedSnapshot data alongside (Refs to) the vanilla
 * checkpoint objects.
 *
 * The vanilla m_checkpointArray on PlayLayer stays the engine's source of
 * truth for respawning; this bank is OUR mirror, extended with data the
 * engine does not keep, plus the percent history (needed to un-bank
 * correctly when the player deletes a checkpoint).
 */
#include <vector>
#include "state/Snapshot.hpp"

namespace ghost {

class CheckpointBank {
public:
    static CheckpointBank& get();

    void clear();
    size_t size() const { return m_bank.size(); }

    /// Create a new entry and return it (caller fills .extended / .vanilla).
    BankedCheckpoint& push();

    /// Remove the most recent entry (vanilla removeCheckpoint counterpart).
    void pop();

    /// Most recent entry or nullptr.
    BankedCheckpoint* last();

    /// Percent of the previous entry (for un-banking), or -1 if none.
    float previousPercent() const;

private:
    CheckpointBank() = default;
    std::vector<BankedCheckpoint> m_bank;
};

} // namespace ghost
