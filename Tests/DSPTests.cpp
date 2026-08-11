#include "TestSupport.h"
#include "dsp/FoundationDSP.h"

#include <cmath>
#include <limits>

namespace
{
using phasecoffin::dsp::FoundationDSP;

constexpr float pi = 3.14159265359f;

FoundationDSP::Parameters harshDefaults()
{
    FoundationDSP::Parameters p;
    p.rateHz = 0.7f;
    p.depth = 0.9f;
    p.centerHz = 680.0f;
    p.spread = 0.8f;
    p.stages = 6;
    p.feedback = 0.55f;
    p.barberDirection = 1;
    p.coffinSkew = 0.7f;
    p.stereoPhase = 0.5f;
    p.mix = 0.82f;
    p.trimDb = -3.0f;
    return p;
}

float sine(int sample, float frequency, float sampleRate)
{
    return std::sin(2.0f * pi * frequency * static_cast<float>(sample) / sampleRate);
}

void requireFinite(float value, const char* message)
{
    test_support::check(std::isfinite(value), message);
    test_support::check(std::abs(value) < 4.01f, message);
}
} // namespace

int main()
{
    return test_support::run("phasecoffin_dsp_tests", [] {
        {
            FoundationDSP dsp;
            auto p = harshDefaults();
            p.feedback = 0.0f;
            p.mix = 1.0f;
            p.trimDb = 0.0f;
            p.rateHz = 0.02f;
            dsp.setTargets(p);
            dsp.prepare(48000.0, 512, 2);

            double inputEnergy = 0.0;
            double outputEnergy = 0.0;
            for (int i = 0; i < 8192; ++i)
            {
                const float input = i == 0 ? 1.0f : 0.0f;
                const float output = dsp.processDebugAllpassBankSample(input, 0, 0);
                inputEnergy += static_cast<double>(input) * input;
                outputEnergy += static_cast<double>(output) * output;
            }
            test_support::check(std::abs(outputEnergy - inputEnergy) < 0.08, "six-stage allpass bank preserves impulse energy");
        }

        {
            FoundationDSP staticDsp;
            FoundationDSP movingDsp;
            auto p = harshDefaults();
            p.feedback = 0.0f;
            p.barberDirection = 0;
            p.mix = 1.0f;
            p.trimDb = 0.0f;
            auto staticParams = p;
            staticParams.depth = 0.0f;
            staticParams.rateHz = 0.02f;
            p.rateHz = 4.0f;
            staticDsp.setTargets(staticParams);
            movingDsp.setTargets(p);
            staticDsp.prepare(48000.0, 128, 2);
            movingDsp.prepare(48000.0, 128, 2);

            double difference = 0.0;
            for (int i = 0; i < 16000; ++i)
            {
                const float input = 0.25f * sine(i, 680.0f, 48000.0f) + 0.18f * sine(i, 1900.0f, 48000.0f);
                difference += std::abs(movingDsp.processSample(input, 0) - staticDsp.processSample(input, 0));
            }
            test_support::check(difference > 20.0, "moving sweep changes notch response against static allpass setting");
        }

        {
            FoundationDSP dsp;
            auto p = harshDefaults();
            p.feedback = 0.0f;
            p.barberDirection = 1;
            p.stereoPhase = 1.0f;
            p.mix = 1.0f;
            p.trimDb = 0.0f;
            dsp.setTargets(p);
            dsp.prepare(48000.0, 256, 2);

            double divergence = 0.0;
            for (int i = 0; i < 4096; ++i)
            {
                const float input = 0.2f * sine(i, 440.0f, 48000.0f);
                divergence += std::abs(dsp.processSample(input, 0) - dsp.processSample(input, 1));
            }
            test_support::check(divergence > 4.0, "stereo phase offset creates deterministic channel divergence");
        }

        {
            FoundationDSP dsp;
            auto p = harshDefaults();
            p.feedback = 0.88f;
            p.mix = 1.0f;
            p.trimDb = 6.0f;
            dsp.setTargets(p);
            dsp.prepare(48000.0, 512, 2);

            double energy = 0.0;
            for (int i = 0; i < 96000; ++i)
            {
                const float input = i < 1024 ? 0.4f * sine(i, 913.0f, 48000.0f) : 0.0f;
                const float output = dsp.processSample(input, i & 1);
                requireFinite(output, "maximum feedback render remains finite and bounded");
                if (i > 48000)
                    energy += std::abs(output);
            }
            test_support::check(energy < 1000.0, "feedback tail remains bounded");
        }

        {
            FoundationDSP dsp;
            auto p = harshDefaults();
            p.feedback = 0.0f;
            p.mix = 0.0f;
            p.trimDb = 0.0f;
            dsp.setTargets(p);
            dsp.prepare(44100.0, 64, 1);
            for (int i = 0; i < 256; ++i)
            {
                const float input = 0.25f * sine(i, 220.0f, 44100.0f);
                test_support::check(std::abs(dsp.processSample(input, 0) - input) < 0.0001f, "mix zero is dry bypass");
            }
        }

        {
            FoundationDSP dsp;
            auto p = harshDefaults();
            p.feedback = 0.2f;
            dsp.setTargets(p);
            dsp.prepare(48000.0, 0, 2);
            test_support::check(dsp.preparedChannels() == 2, "stereo prepare accepted");

            float firstPass[128] {};
            for (int i = 0; i < 128; ++i)
                firstPass[i] = dsp.processSample(i == 0 ? 0.5f : 0.0f, 0);
            dsp.reset();
            for (int i = 0; i < 128; ++i)
                test_support::check(std::abs(firstPass[i] - dsp.processSample(i == 0 ? 0.5f : 0.0f, 0)) < 0.000001f, "reset is deterministic");

            dsp.reset();
            for (int i = 0; i < 256; ++i)
                test_support::check(dsp.processSample(0.0f, 0) == 0.0f, "silence remains silence");

            const auto bad = dsp.processSample(std::numeric_limits<float>::quiet_NaN(), 1);
            requireFinite(bad, "non-finite input is sanitized");

            dsp.prepare(48000.0, 32, 1);
            test_support::check(dsp.preparedChannels() == 1, "mono prepare accepted");
            requireFinite(dsp.processSample(0.1f, 0), "mono processing finite");
        }
    });
}
