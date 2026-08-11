#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ParameterIDs.h"

#include <cmath>

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
    setSize(defaultWidth, defaultHeight);
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
        slider.setColour(juce::Slider::trackColourId, grey(222));
        slider.setColour(juce::Slider::backgroundColourId, grey(28));
        slider.setColour(juce::Slider::thumbColourId, grey(245));
        slider.setColour(juce::Slider::textBoxTextColourId, grey(238));
        slider.setColour(juce::Slider::textBoxBackgroundColourId, grey(5));
        slider.setColour(juce::Slider::textBoxOutlineColourId, grey(96));
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
        label.setColour(juce::Label::textColourId, grey(230));
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);
    }
}

void PhaseCoffinAudioProcessorEditor::paint(juce::Graphics& g)
{
    const auto area = getLocalBounds();
    g.fillAll(juce::Colour(0xff050505));

    const auto grid = 8;
    g.setColour(juce::Colour(0xff202020));
    for (int x = 0; x < area.getWidth(); x += grid)
        g.drawVerticalLine(x, 0.0f, static_cast<float>(area.getHeight()));
    for (int y = 0; y < area.getHeight(); y += grid)
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(area.getWidth()));

    const auto rate = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::rate)->load();
    const auto depth = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::depth)->load();
    const auto center = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::center)->load();
    const auto spread = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::spread)->load();
    const auto feedback = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::feedback)->load();
    const auto skew = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::coffinSkew)->load();
    const auto stereo = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::stereoPhase)->load();
    const auto mix = ownerProcessor.parameters.getRawParameterValue(phasecoffin::parameters::mix)->load();

    g.setColour(juce::Colour(0xffe8e8e8));
    g.setFont(juce::FontOptions(32.0f, juce::Font::bold));
    g.drawText("PhaseCoffin", 32, 24, area.getWidth() - 64, 48, juce::Justification::centredLeft);
    g.setFont(juce::FontOptions(16.0f));
    g.drawText("jp.ehl.phasecoffin / PhCf", 34, 74, area.getWidth() - 68, 24, juce::Justification::centredLeft);

    const int motifLeft = 32;
    const int motifTop = 112;
    const int motifWidth = area.getWidth() - 64;
    g.setColour(juce::Colour(0xff505050));
    g.drawRect(motifLeft, motifTop, motifWidth, 82, 2);
    for (int stage = 0; stage < 6; ++stage)
    {
        const float xNorm = static_cast<float>(stage) / 5.0f;
        const int x = motifLeft + 24 + static_cast<int>(xNorm * static_cast<float>(motifWidth - 48));
        const float sweep = std::sin((xNorm + rate * 0.07f + skew * 0.31f) * juce::MathConstants<float>::twoPi);
        const int notch = static_cast<int>((0.5f + 0.5f * sweep) * (50.0f + depth * 18.0f));
        g.setColour(stage % 2 == 0 ? juce::Colour(0xfff0f0f0) : juce::Colour(0xff9d9d9d));
        g.fillRect(x, motifTop + 8 + notch / 3, 12, 58 - notch / 2);
        g.setColour(juce::Colour(0xff111111));
        g.fillRect(x + 4, motifTop + 18 + notch / 2, 4, 28);
    }

    g.setColour(juce::Colour(0xfff2f2f2));
    const int bottom = area.getHeight() - 40;
    for (int x = 32; x < area.getWidth() - 32; x += 24)
    {
        const float position = static_cast<float>(x - 32) / static_cast<float>(juce::jmax(1, area.getWidth() - 64));
        const int h = 12 + static_cast<int>((0.35f + mix * 0.4f + spread * 0.25f) * static_cast<float>(((x / 24) % 9) * 7));
        const int offset = static_cast<int>((0.5f + 0.5f * std::sin((position + stereo) * juce::MathConstants<float>::twoPi)) * 22.0f);
        g.fillRect(x, bottom - h - offset, 8, h);
    }

    g.setColour(juce::Colour(0xff808080));
    const int centerX = 32 + static_cast<int>((std::log(juce::jlimit(45.0f, 12000.0f, center)) - std::log(45.0f)) / (std::log(12000.0f) - std::log(45.0f)) * static_cast<float>(area.getWidth() - 64));
    const int feedbackHeight = static_cast<int>((feedback + 0.88f) / 1.76f * 48.0f);
    g.drawVerticalLine(centerX, 106.0f, 202.0f);
    g.fillRect(area.getWidth() - 54, 154 - feedbackHeight, 14, feedbackHeight);
}

void PhaseCoffinAudioProcessorEditor::resized()
{
    const auto area = getLocalBounds().reduced(32);
    const int labelWidth = 86;
    const int rowHeight = 28;
    const int gap = 8;
    const int columns = 2;
    const int usableWidth = area.getWidth() - gap * (columns - 1);
    const int columnWidth = usableWidth / columns;
    const int startY = getHeight() < 500 ? 208 : 224;

    for (std::size_t i = 0; i < controlCount; ++i)
    {
        const int column = static_cast<int>(i % static_cast<std::size_t>(columns));
        const int row = static_cast<int>(i / static_cast<std::size_t>(columns));
        const int x = area.getX() + column * (columnWidth + gap);
        const int y = startY + row * (rowHeight + gap);
        labels[i].setBounds(x, y, labelWidth, rowHeight);
        sliders[i].setBounds(x + labelWidth, y, columnWidth - labelWidth, rowHeight);
    }
}
