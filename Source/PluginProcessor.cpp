#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace
{
constexpr float pi = juce::MathConstants<float>::pi;
float dbToGain (float db) { return juce::Decibels::decibelsToGain (db); }
}

UberknallInspiredAudioProcessor::UberknallInspiredAudioProcessor()
    : AudioProcessor (BusesProperties().withInput ("Input", juce::AudioChannelSet::stereo(), true)
                                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{}

UberknallInspiredAudioProcessor::APVTS::ParameterLayout UberknallInspiredAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout layout;
    auto addFloat = [&layout] (const char* id, const char* name, float min, float max, float def, float interval = 0.01f)
    {
        layout.add (std::make_unique<juce::AudioParameterFloat> (juce::ParameterID { id, 1 }, name,
                    juce::NormalisableRange<float> (min, max, interval), def));
    };
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "driveBypass", 1 }, "Drive Bypass", false));
    addFloat ("gain", "Gain", 0.0f, 10.0f, 5.0f);
    layout.add (std::make_unique<juce::AudioParameterBool> (juce::ParameterID { "eqBypass", 1 }, "Tone Stack Bypass", false));
    addFloat ("bass", "Bass", -12.0f, 12.0f, 0.0f);
    addFloat ("mid", "Mid", -12.0f, 12.0f, 0.0f);
    addFloat ("treble", "Treble", -12.0f, 12.0f, 0.0f);
    addFloat ("presence", "Presence", -6.0f, 9.0f, 0.0f);
    addFloat ("master", "Master", -24.0f, 12.0f, 0.0f);
    return layout;
}

std::atomic<float>* UberknallInspiredAudioProcessor::getParam (const char* id) const
{
    return parameters.getRawParameterValue (id);
}

void UberknallInspiredAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels = (juce::uint32) getTotalNumOutputChannels();
    channelStates.clear();
    channelStates.resize ((size_t) getTotalNumOutputChannels());
    updateFilters();
    for (auto& s : channelStates)
    {
        s.driveHighPass.reset(); s.bassShelf.reset(); s.midPeak.reset();
        s.trebleShelf.reset(); s.presencePeak.reset(); s.previousDrive = 0.0f;
    }
}

void UberknallInspiredAudioProcessor::releaseResources() {}

bool UberknallInspiredAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void UberknallInspiredAudioProcessor::updateFilters()
{
    const float bass = getParam("bass")->load();
    const float mid = getParam("mid")->load();
    const float treble = getParam("treble")->load();
    const float presence = getParam("presence")->load();
    for (auto& s : channelStates)
    {
        *s.driveHighPass.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighPass (spec.sampleRate, 75.0f, 0.707f);
        *s.bassShelf.coefficients = *juce::dsp::IIR::Coefficients<float>::makeLowShelf (spec.sampleRate, 115.0f, 0.707f, dbToGain(bass));
        *s.midPeak.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (spec.sampleRate, 390.0f, 0.85f, dbToGain(mid));
        *s.trebleShelf.coefficients = *juce::dsp::IIR::Coefficients<float>::makeHighShelf (spec.sampleRate, 2200.0f, 0.707f, dbToGain(treble));
        *s.presencePeak.coefficients = *juce::dsp::IIR::Coefficients<float>::makePeakFilter (spec.sampleRate, 3600.0f, 0.9f, dbToGain(presence));
    }
}

void UberknallInspiredAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int channels = juce::jmin (buffer.getNumChannels(), (int) channelStates.size());
    const bool driveOn = getParam("driveBypass")->load() < 0.5f;
    const bool eqOn = getParam("eqBypass")->load() < 0.5f;
    const float gain = getParam("gain")->load();
    const float driveAmount = 1.0f + gain * 1.15f;
    const float master = dbToGain (getParam("master")->load());

    if (eqOn) updateFilters();

    for (int ch = 0; ch < channels; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        auto& s = channelStates[(size_t) ch];
        for (int n = 0; n < buffer.getNumSamples(); ++n)
        {
            float x = data[n];
            if (driveOn)
            {
                x = s.driveHighPass.processSample (x);
                // Cascaded, mildly asymmetric soft-clipping stages: a starting DSP voicing,
                // not a claim of transistor-level or schematic-exact emulation.
                const float stage1 = std::tanh (x * driveAmount * 1.35f + 0.035f);
                const float stage2 = std::tanh (stage1 * (1.15f + gain * 0.11f) - 0.018f);
                x = stage2 * (0.82f + 0.018f * gain);
            }
            if (eqOn)
            {
                x = s.bassShelf.processSample (x);
                x = s.midPeak.processSample (x);
                x = s.trebleShelf.processSample (x);
                x = s.presencePeak.processSample (x);
            }
            data[n] = x * master;
        }
    }
}

juce::AudioProcessorEditor* UberknallInspiredAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor (*this);
}

void UberknallInspiredAudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, dest);
}

void UberknallInspiredAudioProcessor::setStateInformation (const void* data, int size)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, size));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new UberknallInspiredAudioProcessor();
}
