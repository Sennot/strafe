#include "core/SessionState.hpp"

namespace ghost {

SessionState& SessionState::get() {
    static SessionState instance;
    return instance;
}

void SessionState::begin(float baselinePercent) {
    m_active = true;
    m_baselinePercent = baselinePercent;
    m_bankedPercent = baselinePercent;
    m_liveMaxPercent = baselinePercent;
    m_diedUnbanked = false;
    m_checkpointCount = 0;

    log::info("[GhostPractice] session started (baseline {}%)", baselinePercent);
}

void SessionState::end() {
    if (!m_active) return;
    m_active = false;
    log::info(
        "[GhostPractice] session ended (banked {}%, checkpoints {})",
        m_bankedPercent, m_checkpointCount
    );
}

void SessionState::onTick(float currentPercent) {
    if (!m_active) return;
    if (currentPercent > m_liveMaxPercent) m_liveMaxPercent = currentPercent;
}

void SessionState::onCheckpoint(float percent) {
    if (!m_active) return;
    m_bankedPercent = percent;
    m_liveMaxPercent = percent;
    m_diedUnbanked = false;
    ++m_checkpointCount;
    log::info("[GhostPractice] checkpoint #{} banked at {}%", m_checkpointCount, percent);
}

void SessionState::onCheckpointRemoved() {
    if (!m_active || m_checkpointCount == 0) return;
    --m_checkpointCount;
    // The engine un-stacks the vanilla checkpoint; our bank follows suit but
    // cannot recover the previous percent from the engine, so we clamp to
    // baseline. VERIFY(runtime, P3): track a stack of banked percents in
    // CheckpointBank and restore the previous one here instead.
    m_bankedPercent = m_baselinePercent;
    m_diedUnbanked = false;
}

void SessionState::onDeath(float /*percent*/) {
    if (!m_active) return;
    m_diedUnbanked = true;
}

void SessionState::onRestored(float fromPercent) {
    if (!m_active) return;
    m_liveMaxPercent = fromPercent;
}

void SessionState::onFinalRestart() {
    if (!m_active) return;
    m_bankedPercent = 0.f;
    m_liveMaxPercent = 0.f;
    m_diedUnbanked = false;
    m_checkpointCount = 0;
    log::info("[GhostPractice] FINAL RESTART — session wiped, run restarts from 0%");
}

float SessionState::bestRun() const {
    if (!m_active) return 0.f;
    if (m_diedUnbanked) return m_policy->afterDeath(m_bankedPercent, m_liveMaxPercent);
    return m_policy->duringRun(m_bankedPercent, m_liveMaxPercent);
}

float SessionState::spoofedNormalPercent() const {
    return bestRun();
}

void SessionState::setPolicy(BestRunPolicy* policy) {
    if (policy != nullptr) m_policy = policy;
}

} // namespace ghost
