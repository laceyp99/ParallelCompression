/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class ParallelCompressionAudioProcessor  : public juce::AudioProcessor, juce::AudioProcessorValueTreeState::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    ParallelCompressionAudioProcessor();
    ~ParallelCompressionAudioProcessor() override;

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

    // functions to calc attack and release times 
    float calcAttack(float value);
    float calcRelease(float value);

    void updateParameters();

    // waveform visual - called in plugineditor
    juce::AudioVisualiserComponent waveViewer;

    //==============================================================================
    // Value Trees
    juce::AudioProcessorValueTreeState treestate;

private:

    // effect objects
    juce::dsp::Gain<float> ingain;
    juce::dsp::Gain<float> outgain;
    juce::dsp::Compressor<float> comp;
    juce::dsp::DryWetMixer<float> mix;

    // parameters
    float inputGain = 1.0;
    float threshold = 0.0;
    float ratio = 3.0;
    float attack = 3.0;
    float release = 3.0;
    float outputGain = 1.0;
    float mixer = 100.0;

    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParallelCompressionAudioProcessor)
};
