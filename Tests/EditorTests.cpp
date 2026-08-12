#include "TestSupport.h"
#include "ParameterIDs.h"
#include "PluginEditor.h"
#include "PluginProcessor.h"

#include <juce_events/juce_events.h>
#include <cmath>
#include <string>

struct EditorTestAccess
{
    static void refresh(PhaseCoffinAudioProcessorEditor& editor) { editor.timerCallback(); }
};

namespace
{
void checkPaintContract(juce::AudioProcessorEditor& editor, int width, int height)
{
    juce::Image image(juce::Image::RGB, width, height, true);
    editor.setBounds(0, 0, width, height);
    {
        juce::Graphics g(image);
        editor.paint(g);
    }

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

bool nearlyEqual(float lhs, float rhs)
{
    return std::abs(lhs - rhs) < 0.0001f;
}

float parameterNormalizedValue(PhaseCoffinAudioProcessor& processor, const char* parameterID, float plainValue)
{
    auto* parameter = processor.parameters.getParameter(parameterID);
    test_support::check(parameter != nullptr, std::string("parameter exists: ") + parameterID);
    return parameter->convertTo0to1(plainValue);
}

juce::Slider& requireSlider(juce::AudioProcessorEditor& editor, const char* parameterID)
{
    auto* slider = dynamic_cast<juce::Slider*>(editor.findChildWithID(juce::String("phasecoffin-") + parameterID));
    test_support::check(slider != nullptr, std::string("control is slider: ") + parameterID);
    test_support::check(slider->getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag,
                        std::string("control uses shared rotary style: ") + parameterID);
    test_support::check(slider->getTextBoxPosition() == juce::Slider::TextBoxBelow,
                        std::string("control text box is below: ") + parameterID);
    return *slider;
}

void setSliderToNormalized(PhaseCoffinAudioProcessor& processor, juce::Slider& slider, const char* parameterID, float normalized)
{
    auto* parameter = processor.parameters.getParameter(parameterID);
    test_support::check(parameter != nullptr, std::string("parameter exists: ") + parameterID);
    slider.setValue(parameter->convertFrom0to1(normalized), juce::sendNotificationSync);
}

void dispatchEditorTimer(PhaseCoffinAudioProcessorEditor& editor)
{
    EditorTestAccess::refresh(editor);
}

void checkMaximumLayout(juce::AudioProcessorEditor& editor)
{
    editor.setBounds(0, 0, ehl::juce_design::Metrics::maximumWidth,
                     ehl::juce_design::Metrics::maximumHeight);
    editor.resized();

    auto* display = editor.findChildWithID("phasecoffin-parameter-display");
    test_support::check(display != nullptr, "maximum layout keeps parameter display");
    test_support::check(display->getBounds() == ehl::juce_design::parameterDisplayArea(editor.getLocalBounds()),
                        "maximum layout uses shared parameter display bounds");

    for (int i = 0; i < editor.getNumChildComponents(); ++i)
    {
        auto* child = editor.getChildComponent(i);
        if (! child->getComponentID().startsWith("phasecoffin-") || child == display)
            continue;
        test_support::check(! child->getBounds().isEmpty(), "maximum layout keeps product control visible");
        test_support::check(editor.getLocalBounds().contains(child->getBounds()),
                            "maximum layout keeps product control inside editor");
    }
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
        auto* display = dynamic_cast<ehl::juce_design::ParameterDisplay*>(editor->findChildWithID("phasecoffin-parameter-display"));
        test_support::check(display != nullptr, "parameter display exists");
        test_support::check(display->getKind() == ehl::juce_design::DisplayKind::phaser, "parameter display kind");
        test_support::check(display->getBounds() == ehl::juce_design::parameterDisplayArea(editor->getLocalBounds()), "parameter display bounds");
        test_support::check(! display->getWantsKeyboardFocus(), "parameter display is noninteractive");

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
            test_support::check(slider->getSliderStyle() == juce::Slider::RotaryHorizontalVerticalDrag,
                                std::string("control uses shared rotary style: ") + id);
            test_support::check(slider->getTextBoxPosition() == juce::Slider::TextBoxBelow,
                                std::string("control text box is below: ") + id);
            test_support::check(component->getName().isNotEmpty(), std::string("control accessible name: ") + id);
            test_support::check(slider->getTooltip().isNotEmpty(), std::string("control tooltip: ") + id);
            test_support::check(component->getWantsKeyboardFocus(), std::string("control keyboard focus: ") + id);
            const auto bounds = component->getBounds();
            test_support::check(! bounds.isEmpty(), std::string("control is laid out immediately after editor construction: ") + id);
            test_support::check(bounds.getY() >= ehl::juce_design::Metrics::controlsTop,
                                std::string("control starts below the parameter display: ") + id);
            test_support::check(bounds.getRight() <= editor->getWidth(), std::string("control stays inside editor width: ") + id);
            test_support::check(bounds.getBottom() <= editor->getHeight(), std::string("control stays inside editor height: ") + id);
        }

        auto& stages = requireSlider(*editor, phasecoffin::parameters::stages);
        auto& center = requireSlider(*editor, phasecoffin::parameters::center);
        auto& depth = requireSlider(*editor, phasecoffin::parameters::depth);
        auto& feedback = requireSlider(*editor, phasecoffin::parameters::feedback);

        dispatchEditorTimer(*custom);
        auto values = display->getValues();
        test_support::check(nearlyEqual(values[0], parameterNormalizedValue(processor, phasecoffin::parameters::stages, 6.0f)), "display reads default stages");
        test_support::check(nearlyEqual(values[1], parameterNormalizedValue(processor, phasecoffin::parameters::center, 620.0f)), "display reads default center");
        test_support::check(nearlyEqual(values[2], parameterNormalizedValue(processor, phasecoffin::parameters::depth, 0.74f)), "display reads default depth");
        test_support::check(nearlyEqual(values[3], parameterNormalizedValue(processor, phasecoffin::parameters::feedback, 0.42f)), "display reads default feedback");

        setSliderToNormalized(processor, stages, phasecoffin::parameters::stages, 0.0f);
        setSliderToNormalized(processor, center, phasecoffin::parameters::center, 0.5f);
        setSliderToNormalized(processor, depth, phasecoffin::parameters::depth, 1.0f);
        setSliderToNormalized(processor, feedback, phasecoffin::parameters::feedback, 0.25f);
        dispatchEditorTimer(*custom);
        values = display->getValues();
        test_support::check(nearlyEqual(values[0], 0.0f) && nearlyEqual(values[1], 0.5f)
                                && nearlyEqual(values[2], 1.0f) && nearlyEqual(values[3], 0.25f),
                            "parameter-to-display changes follow slider min/default/max positions");

        checkPaintContract(*editor, PhaseCoffinAudioProcessorEditor::defaultWidth, PhaseCoffinAudioProcessorEditor::defaultHeight);
        checkPaintContract(*editor, PhaseCoffinAudioProcessorEditor::minimumWidth, PhaseCoffinAudioProcessorEditor::minimumHeight);
        checkMaximumLayout(*editor);
        checkPaintContract(*editor, ehl::juce_design::Metrics::maximumWidth,
                           ehl::juce_design::Metrics::maximumHeight);
    });
}
