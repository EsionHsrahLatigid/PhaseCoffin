#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "ParameterIDs.h"

#include <cmath>

PhaseCoffinAudioProcessor::PhaseCoffinAudioProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout PhaseCoffinAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::rate, "Rate", juce::NormalisableRange<float>(0.02f, 20.0f, 0.001f, 0.35f), 0.38f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::depth, "Depth", 0.0f, 1.0f, 0.74f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::center, "Center", juce::NormalisableRange<float>(45.0f, 12000.0f, 0.1f, 0.42f), 620.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::spread, "Spread", 0.0f, 1.0f, 0.62f));
    params.push_back(std::make_unique<juce::AudioParameterInt>(phasecoffin::parameters::stages, "Stages", 1, 6, 6));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::feedback, "Feedback", -0.88f, 0.88f, 0.42f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>(phasecoffin::parameters::barberDirection, "Barber Direction", juce::StringArray { "Off", "Up", "Down" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::coffinSkew, "Coffin Skew", 0.0f, 1.0f, 0.58f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::stereoPhase, "Stereo Phase", 0.0f, 1.0f, 0.35f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::mix, "Mix", 0.0f, 1.0f, 0.75f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(phasecoffin::parameters::trim, "Trim", -24.0f, 12.0f, -3.0f));
    return { params.begin(), params.end() };
}

void PhaseCoffinAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    dsp.prepare(sampleRate, samplesPerBlock, getTotalNumOutputChannels());
}

bool PhaseCoffinAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    const auto mainIn = layouts.getMainInputChannelSet();
    const auto mainOut = layouts.getMainOutputChannelSet();
    if (mainOut != juce::AudioChannelSet::mono() && mainOut != juce::AudioChannelSet::stereo())
        return false;
    return mainIn == mainOut;
}

void PhaseCoffinAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    juce::ScopedNoDenormals noDenormals;

    phasecoffin::dsp::PhaseCoffinDSP::Parameters dspParameters;
    dspParameters.rateHz = parameters.getRawParameterValue(phasecoffin::parameters::rate)->load();
    dspParameters.depth = parameters.getRawParameterValue(phasecoffin::parameters::depth)->load();
    dspParameters.centerHz = parameters.getRawParameterValue(phasecoffin::parameters::center)->load();
    dspParameters.spread = parameters.getRawParameterValue(phasecoffin::parameters::spread)->load();
    dspParameters.stages = static_cast<int>(parameters.getRawParameterValue(phasecoffin::parameters::stages)->load() + 0.5f);
    dspParameters.feedback = parameters.getRawParameterValue(phasecoffin::parameters::feedback)->load();
    const int barberChoice = static_cast<int>(parameters.getRawParameterValue(phasecoffin::parameters::barberDirection)->load() + 0.5f);
    dspParameters.barberDirection = barberChoice == 1 ? 1 : (barberChoice == 2 ? -1 : 0);
    dspParameters.coffinSkew = parameters.getRawParameterValue(phasecoffin::parameters::coffinSkew)->load();
    dspParameters.stereoPhase = parameters.getRawParameterValue(phasecoffin::parameters::stereoPhase)->load();
    dspParameters.mix = parameters.getRawParameterValue(phasecoffin::parameters::mix)->load();
    dspParameters.trimDb = parameters.getRawParameterValue(phasecoffin::parameters::trim)->load();
    dsp.setTargets(dspParameters);

    const int totalIn = getTotalNumInputChannels();
    const int totalOut = getTotalNumOutputChannels();
    for (int channel = totalIn; channel < totalOut; ++channel)
        buffer.clear(channel, 0, buffer.getNumSamples());

    for (int channel = 0; channel < totalOut; ++channel)
    {
        auto* samples = buffer.getWritePointer(channel);
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
            samples[sample] = dsp.processSample(samples[sample], channel);
    }
}

juce::AudioProcessorEditor* PhaseCoffinAudioProcessor::createEditor()
{
    return new PhaseCoffinAudioProcessorEditor(*this);
}

void PhaseCoffinAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    if (auto state = parameters.copyState().createXml())
        copyXmlToBinary(*state, destData);
}

void PhaseCoffinAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary(data, sizeInBytes))
        if (xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PhaseCoffinAudioProcessor();
}
