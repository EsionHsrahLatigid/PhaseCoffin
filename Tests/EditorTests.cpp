#include "TestSupport.h"
#include "ParameterIDs.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <juce_events/juce_events.h>
#include <string>

namespace
{
void checkPaintContract(juce::AudioProcessorEditor& editor, int width, int height)
{
    juce::Image image(juce::Image::RGB, width, height, true);
    juce::Graphics g(image);
    editor.setBounds(0, 0, width, height);
    editor.paint(g);

    const auto background = ehl::juce_design::Palette::ink();
    const auto divider = ehl::juce_design::Palette::low();
    bool headerTextHasInk = false;
    bool bodyIsPlain = true;
    bool dividerIsPresent = false;
    int maxChannelSpread = 0;

    for (int y = 0; y < image.getHeight(); ++y)
    {
        for (int x = 0; x < image.getWidth(); ++x)
        {
            const auto pixel = image.getPixelAt(x, y);
            headerTextHasInk = headerTextHasInk || (y < ehl::juce_design::Metrics::headerHeight && pixel != background);
            bodyIsPlain = bodyIsPlain && (y < ehl::juce_design::Metrics::headerHeight || pixel == background);
            dividerIsPresent = dividerIsPresent
                || (y == ehl::juce_design::Metrics::dividerY
                    && x >= ehl::juce_design::Metrics::margin
                    && x < image.getWidth() - ehl::juce_design::Metrics::margin
                    && pixel == divider);

            const int red = pixel.getRed();
            const int green = pixel.getGreen();
            const int blue = pixel.getBlue();
            const int high = juce::jmax(juce::jmax(red, green), blue);
            const int low = juce::jmin(juce::jmin(red, green), blue);
            maxChannelSpread = juce::jmax(maxChannelSpread, high - low);
        }
    }

    test_support::check(headerTextHasInk, "shared header paints product text at " + std::to_string(width) + "x" + std::to_string(height));
    test_support::check(dividerIsPresent, "shared divider is present at " + std::to_string(width) + "x" + std::to_string(height));
    test_support::check(bodyIsPlain, "paint leaves common body plain at " + std::to_string(width) + "x" + std::to_string(height));
    test_support::check(maxChannelSpread <= 4,
                        "paint stays inside EHL monochrome palette tolerance at "
                            + std::to_string(width) + "x" + std::to_string(height));
}
} // namespace

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
            test_support::check(bounds.getY() >= ehl::juce_design::Metrics::headerHeight,
                                std::string("control starts below the shared header: ") + id);
            test_support::check(bounds.getRight() <= editor->getWidth(), std::string("control stays inside editor width: ") + id);
            test_support::check(bounds.getBottom() <= editor->getHeight(), std::string("control stays inside editor height: ") + id);
        }

        checkPaintContract(*editor, PhaseCoffinAudioProcessorEditor::defaultWidth, PhaseCoffinAudioProcessorEditor::defaultHeight);
        checkPaintContract(*editor, PhaseCoffinAudioProcessorEditor::minimumWidth, PhaseCoffinAudioProcessorEditor::minimumHeight);
    });
}
