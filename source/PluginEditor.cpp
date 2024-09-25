#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), apvts(processorRef.getAPVTS())
{
    // Configuration des sliders et labels
    setupSlider(filterFreqSlider, filterFreqLabel, "Filter Frequency");
    setupSlider(filterResonanceSlider, filterResonanceLabel, "Filter Resonance");
    setupSlider(lfoDepthSlider, lfoDepthLabel, "LFO Depth");
    setupSlider(lfoRateSlider, lfoRateLabel, "LFO Rate");
    setupSlider(filterGainSlider, filterGainLabel, "Filter Gain"); // Configuration du slider de gain

    // Attachments
    filterFreqAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "FILTER_FREQ", filterFreqSlider);
    filterResonanceAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "FILTER_RESONANCE", filterResonanceSlider);
    lfoDepthAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "LFO_DEPTH", lfoDepthSlider);
    lfoRateAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "LFO_RATE", lfoRateSlider);
    filterGainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(apvts, "FILTER_GAIN", filterGainSlider); // Attachment pour le gain

    // Ajout des composants à l'éditeur
    addAndMakeVisible(filterFreqSlider);
    addAndMakeVisible(filterFreqLabel);
    addAndMakeVisible(filterResonanceSlider);
    addAndMakeVisible(filterResonanceLabel);
    addAndMakeVisible(lfoDepthSlider);
    addAndMakeVisible(lfoDepthLabel);
    addAndMakeVisible(lfoRateSlider);
    addAndMakeVisible(lfoRateLabel);
    addAndMakeVisible(filterGainSlider); // Ajout du slider de gain
    addAndMakeVisible(filterGainLabel); // Ajout du label de gain

    // Taille de l'éditeur
    setSize (500, 350); // Ajusté pour accueillir le nouveau slider
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
}

// Méthode pour configurer un slider et son label
void AudioPluginAudioProcessorEditor::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    // Configuration du slider
    slider.setSliderStyle(juce::Slider::LinearVertical);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 60, 20);
    slider.setColour(juce::Slider::thumbColourId, juce::Colours::darkorange);

    // Configuration du label
    label.setText(name, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.attachToComponent(&slider, false);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Remplissage de l'arrière-plan
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // Texte central
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Peak Filter with LFO Modulation and Gain Control", getLocalBounds().removeFromTop(30), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // Définition des zones pour les sliders
    auto area = getLocalBounds().reduced(40);
    int sliderWidth = 60;
    int sliderHeight = 200;
    int spacing = 20;

    // Positionnement des sliders horizontalement
    filterFreqSlider.setBounds(area.removeFromLeft(sliderWidth).withHeight(sliderHeight));
    area.removeFromLeft(spacing);
    filterResonanceSlider.setBounds(area.removeFromLeft(sliderWidth).withHeight(sliderHeight));
    area.removeFromLeft(spacing);
    lfoDepthSlider.setBounds(area.removeFromLeft(sliderWidth).withHeight(sliderHeight));
    area.removeFromLeft(spacing);
    lfoRateSlider.setBounds(area.removeFromLeft(sliderWidth).withHeight(sliderHeight));
    area.removeFromLeft(spacing);
    filterGainSlider.setBounds(area.removeFromLeft(sliderWidth).withHeight(sliderHeight)); // Positionnement du slider de gain
}