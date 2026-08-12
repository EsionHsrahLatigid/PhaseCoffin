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
            const auto bounds = component->getBounds();
            test_support::check(! bounds.isEmpty(), std::string("control is laid out immediately after editor construction: ") + id);
            test_support::check(bounds.getY() >= 80, std::string("control starts below the header: ") + id);
            test_support::check(bounds.getRight() <= editor->getWidth(), std::string("control stays inside editor width: ") + id);
            test_support::check(bounds.getBottom() <= editor->getHeight(), std::string("control stays inside editor height: ") + id);
        }

        juce::Image image(juce::Image::RGB, 320, 200, true);
        juce::Graphics g(image);
        editor->setBounds(0, 0, image.getWidth(), image.getHeight());
        editor->paint(g);
        const auto background = juce::Colour(0xff050505);
        const auto divider = juce::Colour(0xff2a2a2a);
        bool headerTextHasInk = false;
        bool separatorBandIsSimple = true;
        bool bodyIsPlain = true;
        int maxChannelSpread = 0;
        for (int y = 0; y < image.getHeight(); ++y)
        {
            for (int x = 0; x < image.getWidth(); ++x)
            {
                const auto pixel = image.getPixelAt(x, y);
                headerTextHasInk = headerTextHasInk || (y < 64 && pixel != background);
                if (y >= 64 && y < 80)
                {
                    const bool onDivider = y == 72 && x >= 32 && x < image.getWidth() - 32;
                    separatorBandIsSimple = separatorBandIsSimple && pixel == (onDivider ? divider : background);
                }
                bodyIsPlain = bodyIsPlain && (y < 80 || pixel == background);
                const int red = pixel.getRed();
                const int green = pixel.getGreen();
                const int blue = pixel.getBlue();
                const int high = juce::jmax(juce::jmax(red, green), blue);
                const int low = juce::jmin(juce::jmin(red, green), blue);
                maxChannelSpread = juce::jmax(maxChannelSpread, high - low);
            }
        }
        test_support::check(headerTextHasInk, "simple header paints product text above divider");
        test_support::check(separatorBandIsSimple, "paint keeps only one divider in the 64px to 79px separator band");
        test_support::check(bodyIsPlain, "paint has no grid, motif, frame, or parameter-driven decoration below y=80");
        // The DHN9 paper/mid colours intentionally use a slight warm monochrome offset.
        test_support::check(maxChannelSpread <= 4,
                            "paint stays monochrome within DHN9 palette tolerance, max spread "
                                + std::to_string(maxChannelSpread));
    });
}
