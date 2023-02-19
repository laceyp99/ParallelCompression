/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParallelCompressionAudioProcessor::ParallelCompressionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), treestate(*this, nullptr, "PARMETERS", createParameterLayout()), ingain(), outgain(), comp(), waveViewer(1)
#endif
{
    //initialize waveform viewer
    waveViewer.setRepaintRate(39);
    waveViewer.setBufferSize(256);

    treestate.addParameterListener("input gain", this);
    treestate.addParameterListener("threshold", this);
    treestate.addParameterListener("ratio", this);
    treestate.addParameterListener("attack", this);
    treestate.addParameterListener("release", this);
    treestate.addParameterListener("output gain", this);
    treestate.addParameterListener("mixer", this);
}

ParallelCompressionAudioProcessor::~ParallelCompressionAudioProcessor()
{
    treestate.removeParameterListener("input gain", this);
    treestate.removeParameterListener("threshold", this);
    treestate.removeParameterListener("ratio", this);
    treestate.removeParameterListener("attack", this);
    treestate.removeParameterListener("release", this);
    treestate.removeParameterListener("output gain", this);
    treestate.removeParameterListener("mixer", this);
}

float ParallelCompressionAudioProcessor::calcAttack(float value)
{
    float returnvalue = 3;
    returnvalue += value * 3;
    return returnvalue;
    // returns 3-33ms 
}

float ParallelCompressionAudioProcessor::calcRelease(float value)
{
    float rvalue = 0;
    rvalue = 50 + (value * 25);
    return rvalue;
    // returns 50-300ms
}

juce::AudioProcessorValueTreeState::ParameterLayout ParallelCompressionAudioProcessor::createParameterLayout()
{
    std::vector < std::unique_ptr<juce::RangedAudioParameter >> params;

    auto pInputGain = std::make_unique<juce::AudioParameterFloat>("input gain", "Input Gain", -24.0, 24.0, 0.0);
    auto pThreshold = std::make_unique<juce::AudioParameterFloat>("threshold", "Threshold", -36.0, 0.0, 0.0);
    auto pRatio = std::make_unique<juce::AudioParameterFloat>("ratio", "Ratio", 1.0, 10.0, 3.0);
    auto pAttack = std::make_unique<juce::AudioParameterFloat>("attack", "Attack", 0.0, 10.0, 3.0);
    auto pRelease = std::make_unique<juce::AudioParameterFloat>("release", "Release", 0.0, 10.0, 3.0);
    auto pOutputGain = std::make_unique<juce::AudioParameterFloat>("output gain", "Output Gain", -24.0, 24.0, 0.0);
    auto pMixer = std::make_unique<juce::AudioParameterFloat>("mixer", "Mixer", 0.0, 100.0, 100.0);

    params.push_back(std::move(pInputGain));
    params.push_back(std::move(pThreshold));
    params.push_back(std::move(pRatio));
    params.push_back(std::move(pAttack));
    params.push_back(std::move(pRelease));
    params.push_back(std::move(pOutputGain));

    params.push_back(std::move(pMixer));
    return { params.begin(), params.end() };

}

void ParallelCompressionAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "input gain")
    {
        inputGain = newValue;
    }
    if (parameterID == "threshold")
    {
        threshold = juce::Decibels::decibelsToGain(newValue);
    }
    if (parameterID == "ratio")
    {
        ratio = newValue;
    }
    if (parameterID == "attack")
    {
        attack = calcAttack(newValue);
    }
    if (parameterID == "release")
    {
        release = calcRelease(newValue);
    }
    if (parameterID == "output gain")
    {
        outputGain = newValue;
    }
    if (parameterID == "mixer")
    {
        mixer = newValue;
    }
}


//==============================================================================
const juce::String ParallelCompressionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool ParallelCompressionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool ParallelCompressionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool ParallelCompressionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double ParallelCompressionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int ParallelCompressionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int ParallelCompressionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void ParallelCompressionAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String ParallelCompressionAudioProcessor::getProgramName (int index)
{
    return {};
}

void ParallelCompressionAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void ParallelCompressionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    waveViewer.clear();

    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    ingain.prepare(spec);
    outgain.prepare(spec);
    comp.prepare(spec);
    mix.prepare(spec);

    ingain.reset();
    outgain.reset();
    comp.reset();
    mix.reset();

    updateParameters();
}

void ParallelCompressionAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    waveViewer.clear();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool ParallelCompressionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void ParallelCompressionAudioProcessor::updateParameters()
{
    // connected input gain 
   ingain.setGainDecibels(*treestate.getRawParameterValue("input gain"));
   ingain.setRampDurationSeconds(0.25);

   // connected compressor parameters
   comp.setThreshold(*treestate.getRawParameterValue("threshold"));
   comp.setRatio(*treestate.getRawParameterValue("ratio"));
   comp.setAttack(calcAttack(*treestate.getRawParameterValue("attack")));
   comp.setRelease(calcRelease(*treestate.getRawParameterValue("release")));

   // connected output gain
   outgain.setGainDecibels(*treestate.getRawParameterValue("output gain"));
   outgain.setRampDurationSeconds(0.25);

   // connected mix parameters
   mix.setWetMixProportion(*treestate.getRawParameterValue("mixer") / 100);
}

void ParallelCompressionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateParameters();

    // audio block and context
    juce::dsp::AudioBlock<float> block(buffer);
    auto context = juce::dsp::ProcessContextReplacing(block);

    //instance of block before any processing to push dry signal into mixer
    juce::dsp::AudioBlock<float>dryblock(buffer);  
    dryblock = block;
    mix.pushDrySamples(dryblock);
    
    // processing effects
    ingain.process(context);
    comp.process(context);
    outgain.process(context);
    
    // now mixer has both wet and dry blocks of signal
    mix.mixWetSamples(block);

    // waveform viewer captures final result of signal
    waveViewer.pushBuffer(buffer);
}

//==============================================================================
bool ParallelCompressionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* ParallelCompressionAudioProcessor::createEditor()
{
    return new ParallelCompressionAudioProcessorEditor(*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void ParallelCompressionAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream stream(destData, false);
    treestate.state.writeToStream(stream);
}

void ParallelCompressionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, size_t(sizeInBytes));

    if (tree.isValid())
    {
        treestate.state = tree;
        threshold = juce::Decibels::decibelsToGain(static_cast<float>(*treestate.getRawParameterValue("threshold")));
        ratio = *treestate.getRawParameterValue("ratio");
        attack = calcAttack(*treestate.getRawParameterValue("attack"));
        release = calcRelease(*treestate.getRawParameterValue("release"));
        
        mixer = *treestate.getRawParameterValue("mixer");
        inputGain = *treestate.getRawParameterValue("input gain");
        outputGain = *treestate.getRawParameterValue("output gain");
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new ParallelCompressionAudioProcessor();
}
