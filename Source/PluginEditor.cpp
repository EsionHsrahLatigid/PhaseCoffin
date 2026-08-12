#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ParameterIDs.h"

namespace
{
constexpr const char* ids[] {
    phasecoffin::parameters::rate,
    phasecoffin::parameters::depth,
    phasecoffin::parameters::center,
    phasecoffin::parameters::spread,
    phasecoffin::parameters::stages,
    phasecoffin::parameters::feedback,
    phasecoffin::parameters::barberDirection,
    phasecoffin::parameters::coffinSkew,
    phasecoffin::parameters::stereoPhase,
    phasecoffin::parameters::mix,
    phasecoffin::parameters::trim
};

constexpr const char* names[] {
    "RATE", "DEPTH", "CENTER", "SPREAD", "STAGES", "FEEDBACK", "BARBER", "SKEW", "STEREO", "MIX", "TRIM"
};

constexpr const char* tips[] {
    "LFO speed for the moving allpass coffin notches.",
    "Sweep width in octaves around the center frequency.",
    "Middle frequency of the allpass notch field.",
    "Octave spacing across the six allpass stages.",
    "Number of first-order allpass stages per bank.",
    "Sub-unity wet feedback around the allpass banks.",
    "Off, upward, or downward barberpole-style bank crossfade.",
    "Bends the sweep toward a lopsided coffin-like motion.",
    "Offsets right-channel LFO phase for stereo divergence.",
    "Wet/dry balance from dry input to notched phaser.",
    "Output trim after the wet/dry mix."
};

juce::Colour grey(int value)
{
    return juce::Colour(static_cast<juce::uint8>(value), static_cast<juce::uint8>(value), static_cast<juce::uint8>(value));
}
} // namespace

PhaseCoffinAudioProcessorEditor::PhaseCoffinAudioProcessorEditor(PhaseCoffinAudioProcessor& p)
    : AudioProcessorEditor(&p), ownerProcessor(p),
      tooltipText("PhaseCoffin: every host parameter is exposed as a labelled monochrome control with a stable component ID.")
{
    setResizeLimits(minimumWidth, minimumHeight, defaultWidth * 2, defaultHeight * 2);
    setResizable(true, true);
    setName("PhaseCoffin editor");
    setComponentID("phasecoffin-editor");
    setTitle("PhaseCoffin");
    setDescription("PhaseCoffin monochrome 8-bit custom editor");
    setWantsKeyboardFocus(true);

    for (std::size_t i = 0; i < controlCount; ++i)
    {
        auto& slider = sliders[i];
        slider.setSliderStyle(juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 88, 24);
        slider.setColour(juce::Slider::trackColourId, juce::Colour(0xfff2f2f0));
        slider.setColour(juce::Slider::backgroundColourId, juce::Colour(0xff2a2a2a));
        slider.setColour(juce::Slider::thumbColourId, juce::Colour(0xfff2f2f0));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colour(0xfff2f2f0));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, grey(5));
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff8a8a86));
        slider.setName(names[i]);
        slider.setComponentID(juce::String("phasecoffin-") + ids[i]);
        slider.setTitle(names[i]);
        slider.setDescription(tips[i]);
        slider.setTooltip(tips[i]);
        slider.setWantsKeyboardFocus(true);
        addAndMakeVisible(slider);
        attachments[i] = std::make_unique<SliderAttachment>(ownerProcessor.parameters, ids[i], slider);

        auto& label = labels[i];
        label.setText(names[i], juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centredLeft);
        label.setColour(juce::Label::textColourId, juce::Colour(0xfff2f2f0));
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);
    }

    setSize(defaultWidth, defaultHeight);
}

void PhaseCoffinAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xff050505));

    g.setColour(juce::Colour(0xfff2f2f0));
    g.setFont(juce::FontOptions(24.0f));
    g.drawText("PhaseCoffin", 32, 16, area.getWidth() - 64, 32, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff8a8a86));
    g.setFont(juce::FontOptions(12.0f));
    g.drawText("PHASER", 32, 48, area.getWidth() - 64, 16, juce::Justification::centredLeft);

    g.setColour(juce::Colour(0xff2a2a2a));
    g.drawHorizontalLine(72, 32.0f, static_cast<float>(area.getWidth() - 32));
}

void PhaseCoffinAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(32);
    area.removeFromTop(48);
    const int labelWidth = 86;
    const int gap = 8;
    const int columns = 2;
    const int rows = 6;
    const int usableWidth = area.getWidth() - gap * (columns - 1);
    const int columnWidth = usableWidth / columns;
    const int rowHeight = juce::jmax(28, juce::jmin(40, (area.getHeight() - gap * (rows - 1)) / rows));

    for (std::size_t i = 0; i < controlCount; ++i)
    {
        const int column = static_cast<int>(i / static_cast<std::size_t>(rows));
        const int row = static_cast<int>(i % static_cast<std::size_t>(rows));
        const int x = area.getX() + column * (columnWidth + gap);
        const int y = area.getY() + row * (rowHeight + gap);
        labels[i].setBounds(x, y, labelWidth, rowHeight);
        sliders[i].setBounds(x + labelWidth, y, columnWidth - labelWidth, rowHeight);
    }
}
