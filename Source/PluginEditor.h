#pragma once

#include <ehl/juce_design/EhlDesign.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <memory>

class PhaseCoffinAudioProcessor;

class PhaseCoffinAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                              private juce::Timer
{
public:
    explicit PhaseCoffinAudioProcessorEditor(PhaseCoffinAudioProcessor&);
    ~PhaseCoffinAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    juce::String getTooltip() { return tooltipText; }

    static constexpr int defaultWidth = ehl::juce_design::Metrics::defaultWidth;
    static constexpr int defaultHeight = ehl::juce_design::Metrics::defaultHeight;
    static constexpr int minimumWidth = ehl::juce_design::Metrics::minimumWidth;
    static constexpr int minimumHeight = ehl::juce_design::Metrics::minimumHeight;

private:
    void timerCallback() override;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    static constexpr std::size_t controlCount = 11;

    PhaseCoffinAudioProcessor& ownerProcessor;
    ehl::juce_design::LookAndFeel lookAndFeel;
    juce::TooltipWindow tooltipWindow { this, 700 };
    juce::String tooltipText;
    ehl::juce_design::ParameterDisplay parameterDisplay { ehl::juce_design::DisplayKind::phaser };
    std::array<juce::Slider, controlCount> sliders;
    std::array<juce::Label, controlCount> labels;
    std::array<std::unique_ptr<SliderAttachment>, controlCount> attachments;

    float normalizeControlValue(const char* parameterID, double value) const;
    float normalizeSlider(std::size_t index) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PhaseCoffinAudioProcessorEditor)
};
