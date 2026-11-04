#pragma once
#include <JuceHeader.h>

class UberknallInspiredAudioProcessor : public juce::AudioProcessor
{
public:
    UberknallInspiredAudioProcessor();
    ~UberknallInspiredAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Uberknall Inspired"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double get tailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    using APVTS = juce::AudioProcessorValueTreeState;
    APVTS parameters;
    static APVTS::ParameterLayout createParameterLayout();

private:
    struct ChannelState
    {
        juce::dsp::IIR::Filter<float> driveHighPass;
        juce::dsp::IIR::Filter<float> bassShelf;
        juce::dsp::IIR::Filter<float> midPeak;
        juce::dsp::IIR::Filter<float> trebleShelf;
        juce::dsp::IIR::Filter<float> presencePeak;
        float previousDrive = 0.0f;
    };

    std::vector<ChannelState> channelStates;
    juce::dsp::ProcessSpec spec {};
    std::atomic<float>* getParam (const char* id) const;
    void updateFilters();
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UberknallInspiredAudioProcessor)
};
