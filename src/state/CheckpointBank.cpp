#include "state/CheckpointBank.hpp"

namespace ghost {

CheckpointBank& CheckpointBank::get() {
    static CheckpointBank instance;
    return instance;
}

void CheckpointBank::clear() {
    m_bank.clear();
}

BankedCheckpoint& CheckpointBank::push() {
    m_bank.emplace_back();
    return m_bank.back();
}

void CheckpointBank::pop() {
    if (!m_bank.empty()) m_bank.pop_back();
}

BankedCheckpoint* CheckpointBank::last() {
    return m_bank.empty() ? nullptr : &m_bank.back();
}

float CheckpointBank::previousPercent() const {
    if (m_bank.size() < 2) return -1.f;
    return m_bank[m_bank.size() - 2].extended.percent;
}

} // namespace ghost
