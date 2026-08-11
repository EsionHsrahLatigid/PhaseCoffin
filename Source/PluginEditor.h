#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <memory>

class PhaseCoffinAudioProcessor;

class PhaseCoffinAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit PhaseCoffinAudioProcessorEditor(PhaseCoffinAudioProcessor&);
    ~PhaseCoffinAudioProcessorEditor() override = default;

    void paint(juce::Graphics&) override;
    void resized() override;
    juce::String getTooltip() { return tooltipText; }

    static constexpr int defaultWidth = 960;
    static constexpr int defaultHeight = 544;
    static constexpr int minimumWidth = 720;
    static constexpr int minimumHeight = 432;

private:
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    static constexpr std::size_t controlCount = 11;

    PhaseCoffinAudioProcessor& ownerProcessor;
    juce::TooltipWindow tooltipWindow { this, 700 };
    juce::String tooltipText;
    std::array<juce::Slider, controlCount> sliders;
    std::array<juce::Label, controlCount> labels;
    std::array<std::unique_ptr<SliderAttachment>, controlCount> attachments;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseCoffinAudioProcessorEditor)
};
