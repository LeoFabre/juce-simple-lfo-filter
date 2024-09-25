#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       parameters(*this, nullptr, juce::Identifier("PARAMETERS"), createParameters())
#endif
{
    // Initialisation du filtre avec les paramètres par défaut
    currentFilter = juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, 1000.0f, 1.0f, filterGain);
    filter.coefficients = currentFilter; // Assigner directement le pointeur intelligent
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
}

// Création des paramètres
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameters()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Fréquence du filtre
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_FREQ", "Filter Frequency",
        juce::NormalisableRange<float>(20.0f, 20000.0f, 1.0f, 0.5f), 1000.0f));

    // Résonance du filtre (Q)
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_RESONANCE", "Filter Resonance",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f, 1.0f), 1.0f));

    // Profondeur du LFO
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LFO_DEPTH", "LFO Depth",
        juce::NormalisableRange<float>(0.0f, 1000.0f, 1.0f, 1.0f), 100.0f));

    // Taux du LFO
    params.push_back(std::make_unique<juce::AudioParameterFloat>("LFO_RATE", "LFO Rate",
        juce::NormalisableRange<float>(0.1f, 20.0f, 0.1f, 0.5f), 5.0f));

    // Gain du filtre
    params.push_back(std::make_unique<juce::AudioParameterFloat>("FILTER_GAIN", "Filter Gain",
        juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f, 1.0f), 1.0f));

    return { params.begin(), params.end() };
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

// Préparation du plugin
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    // Préparer le filtre avec les spécifications DSP
    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();

    filter.prepare(spec);

    // Récupération des paramètres
    float freq = *parameters.getRawParameterValue("FILTER_FREQ");
    float q = *parameters.getRawParameterValue("FILTER_RESONANCE");
    filterGain = *parameters.getRawParameterValue("FILTER_GAIN");

    // Mise à jour des coefficients du filtre avec la fréquence, Q et gain
    currentFilter = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freq, q, filterGain);
    filter.coefficients = currentFilter; // Assigner directement le pointeur intelligent

    // Calcul de l'incrément du LFO
    float lfoRate = *parameters.getRawParameterValue("LFO_RATE");
    lfoIncrement = (2.0 * juce::MathConstants<double>::pi * lfoRate) / sampleRate;
}

void AudioPluginAudioProcessor::releaseResources()
{
    // Libération des ressources si nécessaire
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // Supporte mono et stéréo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    #if ! JucePlugin_IsSynth
    // Vérifie que l'entrée et la sortie ont la même configuration
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
    #endif

    return true;
  #endif
}
#endif

// Traitement audio optimisé
void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear les canaux de sortie non utilisés
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // Récupération des paramètres
    float freq = *parameters.getRawParameterValue("FILTER_FREQ");
    float q = *parameters.getRawParameterValue("FILTER_RESONANCE");
    float lfoDepth = *parameters.getRawParameterValue("LFO_DEPTH");
    float lfoRate = *parameters.getRawParameterValue("LFO_RATE");
    float gain = *parameters.getRawParameterValue("FILTER_GAIN");

    // Mise à jour des coefficients du filtre si la fréquence, Q ou gain a changé
    // (Optionnel : Vous pouvez ajouter une logique pour ne mettre à jour que si les paramètres ont changé)
    currentFilter = juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, freq, q, gain);
    filter.coefficients = currentFilter; // Assigner directement le pointeur intelligent

    // Mise à jour de l'incrément du LFO en cas de changement de taux
    lfoIncrement = (2.0 * juce::MathConstants<double>::pi * lfoRate) / currentSampleRate;

    // Calcul du LFO pour le bloc
    float lfoValue = std::sin(lfoPhase) * lfoDepth;

    // Modulation de la fréquence du filtre
    float modulatedFreq = freq + lfoValue;
    modulatedFreq = juce::jlimit(20.0f, 20000.0f, modulatedFreq); // Limitation de la fréquence

    // Mise à jour des coefficients du filtre avec la fréquence modulée et le gain
    currentFilter = juce::dsp::IIR::Coefficients<float>::makePeakFilter(currentSampleRate, modulatedFreq, q, gain);
    filter.coefficients = currentFilter; // Assigner directement le pointeur intelligent

    // Incrément de la phase du LFO basé sur le nombre d'échantillons dans le bloc
    lfoPhase += lfoIncrement * buffer.getNumSamples();
    lfoPhase = std::fmod(lfoPhase, 2.0 * juce::MathConstants<double>::pi);

    // Appliquer le filtre à tout le bloc
    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    filter.process(context);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

// Gestion de l'état du plugin
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // Sauvegarde de l'état des paramètres
    juce::ValueTree state = parameters.copyState();
    if (state.isValid())
    {
        std::unique_ptr<juce::XmlElement> xml (state.createXml());
        if (xml != nullptr)
        {
            copyXmlToBinary (*xml, destData);
        }
    }
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // Restauration de l'état des paramètres
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml && xml->hasTagName (parameters.state.getType()))
    {
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
