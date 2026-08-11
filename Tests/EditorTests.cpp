#include "TestSupport.h"
#include "ParameterIDs.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <juce_events/juce_events.h>
#include <string>

int main()
{
    juce::ScopedJuceInitialiser_GUI juce;
    return test_support::run("phasecoffin_editor_tests", [] {
        PhaseCoffinAudioProcessor processor;
        std::unique_ptr<juce::AudioProcessorEditor> editor(processor.createEditor());
        auto* custom = dynamic_cast<PhaseCoffinAudioProcessorEditor*>(editor.get());
        test_support::check(custom != nullptr, "custom editor type, not GenericAudioProcessorEditor");
        test_support::check(dynamic_cast<juce::GenericAudioProcessorEditor*>(editor.get()) == nullptr, "not GenericAudioProcessorEditor");
        test_support::check(editor->getWidth() == PhaseCoffinAudioProcessorEditor::defaultWidth, "default width");
        test_support::check(editor->getHeight() == PhaseCoffinAudioProcessorEditor::defaultHeight, "default height");
        test_support::check(editor->getComponentID() == "phasecoffin-editor", "component id");
        test_support::check(editor->getName().isNotEmpty(), "accessible name");
        test_support::check(custom->getTooltip().isNotEmpty(), "tooltip");
        test_support::check(editor->getWantsKeyboardFocus(), "keyboard focus");
        const char* ids[] {
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
        for (const auto* id : ids)
        {
            auto* component = editor->findChildWithID(juce::String("phasecoffin-") + id);
            test_support::check(component != nullptr, std::string("control exists: ") + id);
            auto* slider = dynamic_cast<juce::Slider*>(component);
            test_support::check(slider != nullptr, std::string("control is slider: ") + id);
            test_support::check(component->getName().isNotEmpty(), std::string("control accessible name: ") + id);
            test_support::check(slider->getTooltip().isNotEmpty(), std::string("control tooltip: ") + id);
            test_support::check(component->getWantsKeyboardFocus(), std::string("control keyboard focus: ") + id);
        }

        juce::Image image(juce::Image::RGB, 320, 200, true);
        juce::Graphics g(image);
        editor->setBounds(0, 0, image.getWidth(), image.getHeight());
        editor->paint(g);
        const auto first = image.getPixelAt(0, 0);
        bool varied = false;
        for (int y = 0; y < image.getHeight(); y += 16)
            for (int x = 0; x < image.getWidth(); x += 16)
                varied = varied || image.getPixelAt(x, y) != first;
        test_support::check(varied, "software paint uses monochrome palette and procedural motif");
    });
}
