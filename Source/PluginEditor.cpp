#include "PluginEditor.h"
#include "PluginProcessor.h"
#include "ParameterIDs.h"

namespace
{
namespace design = ehl::juce_design;

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

} // namespace

PhaseCoffinAudioProcessorEditor::PhaseCoffinAudioProcessorEditor(PhaseCoffinAudioProcessor& p)
    : AudioProcessorEditor(&p), ownerProcessor(p),
      tooltipText("PhaseCoffin: every host parameter is exposed as a labelled monochrome control with a stable component ID.")
{
    setLookAndFeel(&lookAndFeel);
    setResizeLimits(minimumWidth, minimumHeight, design::Metrics::maximumWidth, design::Metrics::maximumHeight);
    setResizable(true, true);
    setName("PhaseCoffin editor");
    setComponentID("phasecoffin-editor");
    setTitle("PhaseCoffin");
    setDescription("PhaseCoffin monochrome 8-bit custom editor");
    setWantsKeyboardFocus(true);

    for (std::size_t i = 0; i < controlCount; ++i)
    {
        auto& slider = sliders[i];
        design::styleSlider(slider);
        slider.setName(names[i]);
        slider.setComponentID(juce::String("phasecoffin-") + ids[i]);
        slider.setTitle(names[i]);
        slider.setDescription(tips[i]);
        slider.setTooltip(tips[i]);
        addAndMakeVisible(slider);
        attachments[i] = std::make_unique<SliderAttachment>(ownerProcessor.parameters, ids[i], slider);

        auto& label = labels[i];
        design::styleLabel(label);
        label.setText(names[i], juce::dontSendNotification);
        label.setName(names[i]);
        label.setInterceptsMouseClicks(false, false);
        addAndMakeVisible(label);
    }

    setSize(defaultWidth, defaultHeight);
}

PhaseCoffinAudioProcessorEditor::~PhaseCoffinAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

void PhaseCoffinAudioProcessorEditor::paint(juce::Graphics& g)
{
    design::paintEditorChrome(g, getLocalBounds(), "PhaseCoffin", "PHASER");
}

void PhaseCoffinAudioProcessorEditor::resized()
{
    for (std::size_t i = 0; i < controlCount; ++i)
        design::layoutLabelledControl(labels[i], sliders[i], design::controlCell(getLocalBounds(), i));
}
