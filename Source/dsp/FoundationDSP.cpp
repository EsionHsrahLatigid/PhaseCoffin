#include "dsp/FoundationDSP.h"

#include <cmath>

namespace phasecoffin::dsp
{
void FoundationDSP::prepare(double sampleRate, int, int channels) noexcept
{
    sampleRate_ = std::isfinite(sampleRate) && sampleRate > 1000.0 ? sampleRate : 44100.0;
    channels_ = channels < 0 ? 0 : (channels > 2 ? 2 : channels);
    reset();
}

void FoundationDSP::reset() noexcept
{
    current_ = target_;
    for (auto& channel : stages_)
        for (auto& bank : channel)
            for (auto& stage : bank)
            {
                stage.z = 0.0f;
                stage.coefficient = 0.0f;
            }
    phase_[0] = 0.0f;
    phase_[1] = 0.25f;
    feedbackSample_[0] = 0.0f;
    feedbackSample_[1] = 0.0f;
}

void FoundationDSP::setTargets(const Parameters& parameters) noexcept
{
    target_.rateHz = clamp(sanitize(parameters.rateHz), 0.02f, 20.0f);
    target_.depth = clamp(sanitize(parameters.depth), 0.0f, 1.0f);
    target_.centerHz = clamp(sanitize(parameters.centerHz), 45.0f, 12000.0f);
    target_.spread = clamp(sanitize(parameters.spread), 0.0f, 1.0f);
    target_.stages = parameters.stages < 1 ? 1 : (parameters.stages > maxStages ? maxStages : parameters.stages);
    target_.feedback = clamp(sanitize(parameters.feedback), -0.88f, 0.88f);
    target_.barberDirection = parameters.barberDirection < 0 ? -1 : (parameters.barberDirection > 0 ? 1 : 0);
    target_.coffinSkew = clamp(sanitize(parameters.coffinSkew), 0.0f, 1.0f);
    target_.stereoPhase = clamp(sanitize(parameters.stereoPhase), 0.0f, 1.0f);
    target_.mix = clamp(sanitize(parameters.mix), 0.0f, 1.0f);
    target_.trimDb = clamp(sanitize(parameters.trimDb), -24.0f, 12.0f);
}

float FoundationDSP::processSample(float input, int channel) noexcept
{
    input = clamp(sanitize(input), -8.0f, 8.0f);
    const auto index = channel <= 0 ? 0 : 1;

    smoothParameters();

    const float phaseOffset = index == 0 ? 0.0f : current_.stereoPhase * 0.5f;
    phase_[index] = wrap01(phase_[index] + static_cast<float>(current_.rateHz / sampleRate_));
    const float lfo = wrap01(phase_[index] + phaseOffset);
    const float modInput = clamp(input + feedbackSample_[index] * current_.feedback, -8.0f, 8.0f);

    const float bankA = processBank(modInput, index, 0, lfo, 0.0f);
    const float bankB = processBank(modInput, index, 1, lfo, 0.5f);

    float allpass = bankA;
    if (current_.barberDirection != 0)
    {
        const float barber = wrap01(lfo * 2.0f * static_cast<float>(current_.barberDirection > 0 ? 1 : -1));
        const float theta = barber * 1.57079632679f;
        allpass = bankA * std::cos(theta) + bankB * std::sin(theta);
    }
    else
    {
        allpass = 0.5f * (bankA + bankB);
    }

    feedbackSample_[index] = clamp(allpass * 0.82f, -2.0f, 2.0f);
    const float notched = clamp(0.5f * (input - allpass), -2.0f, 2.0f);
    const float output = (input + (notched - input) * current_.mix) * dbToGain(current_.trimDb);
    return clamp(sanitize(output), -4.0f, 4.0f);
}

float FoundationDSP::processDebugAllpassBankSample(float input, int channel, int bank) noexcept
{
    input = clamp(sanitize(input), -8.0f, 8.0f);
    const auto index = channel <= 0 ? 0 : 1;
    const auto bankIndex = bank <= 0 ? 0 : 1;
    smoothParameters();
    phase_[index] = wrap01(phase_[index] + static_cast<float>(current_.rateHz / sampleRate_));
    return processBank(input, index, bankIndex, phase_[index], bankIndex == 0 ? 0.0f : 0.5f);
}

void FoundationDSP::smoothParameters() noexcept
{
    constexpr float smoothing = 0.0025f;
    current_.rateHz += (target_.rateHz - current_.rateHz) * smoothing;
    current_.depth += (target_.depth - current_.depth) * smoothing;
    current_.centerHz += (target_.centerHz - current_.centerHz) * smoothing;
    current_.spread += (target_.spread - current_.spread) * smoothing;
    current_.feedback += (target_.feedback - current_.feedback) * smoothing;
    current_.coffinSkew += (target_.coffinSkew - current_.coffinSkew) * smoothing;
    current_.stereoPhase += (target_.stereoPhase - current_.stereoPhase) * smoothing;
    current_.mix += (target_.mix - current_.mix) * smoothing;
    current_.trimDb += (target_.trimDb - current_.trimDb) * smoothing;
    current_.stages = target_.stages;
    current_.barberDirection = target_.barberDirection;
}

float FoundationDSP::processBank(float input, int channel, int bank, float lfo, float bankOffset) noexcept
{
    const float motion = 0.5f + 0.5f * std::sin(6.28318530718f * wrap01(lfo + bankOffset));
    const float skewed = std::pow(clamp(motion, 0.0f, 1.0f), 0.35f + current_.coffinSkew * 2.4f);
    const float depthOctaves = 0.25f + current_.depth * 5.25f;
    const float base = current_.centerHz * std::pow(2.0f, (skewed - 0.5f) * depthOctaves);
    const float spreadOctaves = current_.spread * 3.25f;

    float y = input;
    for (int stageIndex = 0; stageIndex < maxStages; ++stageIndex)
    {
        if (stageIndex < current_.stages)
        {
            const float normalized = (static_cast<float>(stageIndex) - 2.5f) / 5.0f;
            const float bankPush = bank == 0 ? -0.17f : 0.17f;
            const float frequency = base * std::pow(2.0f, normalized * spreadOctaves + bankPush);
            y = processAllpass(y, stages_[channel][bank][stageIndex], coefficientForFrequency(frequency, sampleRate_));
        }
    }
    return y;
}

float FoundationDSP::processAllpass(float input, AllpassStage& stage, float targetCoefficient) noexcept
{
    stage.coefficient += (targetCoefficient - stage.coefficient) * 0.015f;
    stage.coefficient = clamp(stage.coefficient, -0.985f, 0.985f);
    const float y = -stage.coefficient * input + stage.z;
    stage.z = clamp(input + stage.coefficient * y, -8.0f, 8.0f);
    return sanitize(y);
}

float FoundationDSP::sanitize(float value) noexcept
{
    return std::isfinite(value) ? value : 0.0f;
}

float FoundationDSP::clamp(float value, float lo, float hi) noexcept
{
    return value < lo ? lo : (value > hi ? hi : value);
}

float FoundationDSP::wrap01(float value) noexcept
{
    value -= std::floor(value);
    return value < 0.0f ? value + 1.0f : value;
}

float FoundationDSP::dbToGain(float db) noexcept
{
    return std::pow(10.0f, db / 20.0f);
}

float FoundationDSP::coefficientForFrequency(float frequencyHz, double sampleRate) noexcept
{
    const float nyquistSafe = static_cast<float>(sampleRate * 0.46);
    frequencyHz = clamp(frequencyHz, 20.0f, nyquistSafe);
    const float tangent = std::tan(3.14159265359f * frequencyHz / static_cast<float>(sampleRate));
    return clamp((tangent - 1.0f) / (tangent + 1.0f), -0.985f, 0.985f);
}
} // namespace phasecoffin::dsp
