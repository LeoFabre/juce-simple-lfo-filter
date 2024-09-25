#pragma once

#include "juce_audio_processors/juce_audio_processors.h"
#include "juce_dsp/juce_dsp.h"

class AudioPluginAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Accessor pour APVTS
    juce::AudioProcessorValueTreeState& getAPVTS() { return parameters; }

private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;

    // Paramètres
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameters();

    // Filtre IIR
    juce::dsp::IIR::Filter<float> filter;

    // Coefficients du filtre
    juce::dsp::IIR::Coefficients<float>::Ptr currentFilter;

    // LFO
    double lfoPhase = 0.0;
    double lfoIncrement = 0.0;

    // Sample rate
    double currentSampleRate = 44100.0;

    // Gain
    float filterGain = 1.0f;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)
};
