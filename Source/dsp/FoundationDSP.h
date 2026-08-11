#pragma once

#include <array>
#include <cstddef>

namespace phasecoffin::dsp
{
class FoundationDSP
{
public:
    struct Parameters
    {
        float rateHz { 0.38f };
        float depth { 0.74f };
        float centerHz { 620.0f };
        float spread { 0.62f };
        int stages { 6 };
        float feedback { 0.42f };
        int barberDirection { 0 };
        float coffinSkew { 0.58f };
        float stereoPhase { 0.35f };
        float mix { 0.75f };
        float trimDb { -3.0f };
    };

    void prepare(double sampleRate, int maxBlockSize, int channels) noexcept;
    void reset() noexcept;
    void setTargets(const Parameters& parameters) noexcept;
    float processSample(float input, int channel) noexcept;
    float processDebugAllpassBankSample(float input, int channel, int bank) noexcept;
    int preparedChannels() const noexcept { return channels_; }

private:
    static constexpr int maxChannels = 2;
    static constexpr int bankCount = 2;
    static constexpr int maxStages = 6;

    struct AllpassStage
    {
        float z { 0.0f };
        float coefficient { 0.0f };
    };

    static float sanitize(float value) noexcept;
    static float clamp(float value, float lo, float hi) noexcept;
    static float wrap01(float value) noexcept;
    static float dbToGain(float db) noexcept;
    static float coefficientForFrequency(float frequencyHz, double sampleRate) noexcept;

    void smoothParameters() noexcept;
    float processBank(float input, int channel, int bank, float lfo, float bankOffset) noexcept;
    float processAllpass(float input, AllpassStage& stage, float targetCoefficient) noexcept;

    double sampleRate_ { 44100.0 };
    int channels_ { 0 };
    Parameters current_;
    Parameters target_;
    std::array<std::array<std::array<AllpassStage, maxStages>, bankCount>, maxChannels> stages_ {};
    std::array<float, maxChannels> phase_ {};
    std::array<float, maxChannels> feedbackSample_ {};
};
} // namespace phasecoffin::dsp
