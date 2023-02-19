/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

struct CustomRotarySlider : juce::Slider
{
    CustomRotarySlider() : juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::TextBoxBelow)
    {

    }
};

//==============================================================================
/**
*/
class ParallelCompressionAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    ParallelCompressionAudioProcessorEditor (ParallelCompressionAudioProcessor&);
    ~ParallelCompressionAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

 

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    ParallelCompressionAudioProcessor& audioProcessor;

    juce::Label ingainLabel, outgainLabel, thresholdLabel, ratioLabel, attackLabel, releaseLabel, mixLabel;
 
    juce::Slider waveZoom, ingainSlider, outgainSlider;
    
    juce::ToggleButton channelToggle;
    
    CustomRotarySlider compThreshold, compRatio, compAttack, compRelease, mixSlider;
    
    using APVTS = juce::AudioProcessorValueTreeState;
    using Attachment = APVTS::SliderAttachment;

    Attachment ingainSliderAttachment,
        outgainSliderAttachment,
        compThresholdAttachment,
        compRatioAttachment,
        compAttackAttachment,
        compReleaseAttachment,
        compMixAttachment;

    std::vector<juce::Component*> getComps();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ParallelCompressionAudioProcessorEditor)
};
