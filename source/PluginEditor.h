#pragma once

#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    AudioPluginAudioProcessor& processorRef;
    juce::AudioProcessorValueTreeState& apvts;

    // Sliders
    juce::Slider filterFreqSlider;
    juce::Slider filterResonanceSlider;
    juce::Slider lfoDepthSlider;
    juce::Slider lfoRateSlider;
    juce::Slider filterGainSlider;

    // Attachments
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterFreqAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterResonanceAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoDepthAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> lfoRateAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> filterGainAttachment;

    // Labels
    juce::Label filterFreqLabel;
    juce::Label filterResonanceLabel;
    juce::Label lfoDepthLabel;
    juce::Label lfoRateLabel;
    juce::Label filterGainLabel;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name);

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};