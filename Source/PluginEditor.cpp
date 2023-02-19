/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
ParallelCompressionAudioProcessorEditor::ParallelCompressionAudioProcessorEditor (ParallelCompressionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), waveZoom(), channelToggle(), ingainSlider(), outgainSlider(),
    compThreshold(), compRatio(), compAttack(), compRelease(), mixSlider(),
    ingainSliderAttachment(audioProcessor.treestate, "input gain", ingainSlider),
    outgainSliderAttachment(audioProcessor.treestate, "output gain", outgainSlider),
    compThresholdAttachment(audioProcessor.treestate, "threshold", compThreshold),
    compRatioAttachment(audioProcessor.treestate, "ratio", compRatio),
    compAttackAttachment(audioProcessor.treestate, "attack", compAttack),
    compReleaseAttachment(audioProcessor.treestate, "release", compRelease),
    compMixAttachment(audioProcessor.treestate, "mixer", mixSlider)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.

    // waveform viewer
    addAndMakeVisible(audioProcessor.waveViewer);
    audioProcessor.waveViewer.setColours(juce::Colours::black, juce::Colours::whitesmoke.withAlpha(0.5f));

    // waveform zoom
    addAndMakeVisible(waveZoom);
    waveZoom.setSliderStyle(juce::Slider::SliderStyle::LinearBarVertical);
    waveZoom.setRange(32.0, 1024.0, 16.0);
    waveZoom.setTextValueSuffix(" samples");
    // connection to audioprocessor
    waveZoom.onValueChange = [this]()
    {
        audioProcessor.waveViewer.setBufferSize(waveZoom.getValue());
    };
    // toggle to make the waveform viewer stereo instead of mono
    addAndMakeVisible(channelToggle);
    channelToggle.setButtonText("Stereo");
    channelToggle.onClick = [this]()
    {
        channelToggle.getToggleState() ? audioProcessor.waveViewer.setNumChannels(2) : audioProcessor.waveViewer.setNumChannels(1);
    };

    // input gain slider
    addAndMakeVisible(ingainSlider);
    ingainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    ingainSlider.setRange(-24.0, 24.0, 1.0);
    ingainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(ingainLabel);
    ingainLabel.setText("Input Gain", juce::dontSendNotification);
    ingainLabel.attachToComponent(&ingainSlider, true);

    // output gain slider
    addAndMakeVisible(outgainSlider);
    outgainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
    outgainSlider.setRange(-24.0, 24.0, 1.0);
    outgainSlider.setTextValueSuffix(" dB");
    addAndMakeVisible(outgainLabel);
    outgainLabel.setText("Output Gain", juce::dontSendNotification);
    outgainLabel.attachToComponent(&outgainSlider, true);

    // threshold knob
    addAndMakeVisible(compThreshold);
    compThreshold.setRange(-36.0, 0.0, 1.0);
    compThreshold.setTextValueSuffix(" dB");
    addAndMakeVisible(thresholdLabel);
    thresholdLabel.setText("Threshold", juce::dontSendNotification);
    thresholdLabel.attachToComponent(&compThreshold, true);

    // ratio knob
    addAndMakeVisible(compRatio);
    compRatio.setRange(1.0, 12.0, 1.0);
    compRatio.setTextValueSuffix(":1");
    addAndMakeVisible(ratioLabel);
    ratioLabel.setText("Ratio", juce::dontSendNotification);
    ratioLabel.attachToComponent(&compRatio, true);

    // attack knob (1-10 no unit knob. range is 3-33ms)
    addAndMakeVisible(compAttack);
    compAttack.setRange(0.0, 10.0, 0.1);
    addAndMakeVisible(attackLabel);
    attackLabel.setText("Attack", juce::dontSendNotification);
    attackLabel.attachToComponent(&compAttack, true);

    // release knob (1-10 no unit knob. range is 50-300ms)
    addAndMakeVisible(compRelease);
    compRelease.setRange(0.0, 10.0, 0.1);
    addAndMakeVisible(releaseLabel);
    releaseLabel.setText("Release", juce::dontSendNotification);
    releaseLabel.attachToComponent(&compRelease, true);

    // mix knob (parallel compression)
    addAndMakeVisible(mixSlider);
    mixSlider.setTextValueSuffix(" %");
    mixSlider.setRange(0.0, 100.0, 1.0);
    addAndMakeVisible(mixLabel);
    mixLabel.setText("Mix", juce::dontSendNotification);
    mixLabel.attachToComponent(&mixSlider, true);

    setSize (800, 400);
}

ParallelCompressionAudioProcessorEditor::~ParallelCompressionAudioProcessorEditor()
{
}

//==============================================================================
void ParallelCompressionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    //g.setColour (juce::Colours::white);
    //g.setFont (30.0f);
    //g.drawFittedText ("Waveform", getLocalBounds(), juce::Justification::centredTop, 1);
}

void ParallelCompressionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    auto waveViewerArea = bounds.removeFromTop(bounds.getHeight() * 0.5);

    audioProcessor.waveViewer.setBounds(waveViewerArea.getCentreX() - 100.0, waveViewerArea.getCentreY() - 100.0, 200.0, 200.0);
    waveZoom.setBounds((audioProcessor.waveViewer.getX() + audioProcessor.waveViewer.getWidth() + 5), audioProcessor.waveViewer.getY(), 128, audioProcessor.waveViewer.getHeight());
    channelToggle.setBounds((waveZoom.getX() + waveZoom.getWidth() + 45), waveViewerArea.getCentreY() - 18, 64, 32);

    outgainSlider.setBounds(audioProcessor.waveViewer.getX() * 0.5, waveViewerArea.getY()+25, 128, waveViewerArea.getHeight()-25);
    outgainLabel.setBounds(outgainSlider.getX() + (outgainSlider.getWidth() * 0.5), waveViewerArea.getY(), outgainSlider.getWidth(), 25);
    
    ingainSlider.setBounds(waveViewerArea.getX() , waveViewerArea.getY() + 25, 128, waveViewerArea.getHeight()-25);
    ingainLabel.setBounds(ingainSlider.getX() + (ingainSlider.getWidth() * 0.5), waveViewerArea.getY(), ingainSlider.getWidth(), 25);

    compThreshold.setBounds(waveViewerArea.getX(), bounds.getY()+ 25, bounds.getWidth() * 0.2, bounds.getHeight()-25);
    thresholdLabel.setBounds(compThreshold.getX() + 45, bounds.getY(), compThreshold.getWidth(), 25);
    
    compRatio.setBounds(waveViewerArea.getX() + compThreshold.getWidth(), bounds.getY()+25, bounds.getWidth() * 0.20, bounds.getHeight()-25);
    ratioLabel.setBounds(compRatio.getX() + 58, bounds.getY(), compRatio.getWidth(), 25);

    compAttack.setBounds(compRatio.getX() + compRatio.getWidth(), bounds.getY()+25, bounds.getWidth() * 0.20, bounds.getHeight()-25);
    attackLabel.setBounds(compAttack.getX() + 58, bounds.getY(), compAttack.getWidth(), 25);

    compRelease.setBounds(compAttack.getX() + compAttack.getWidth(), bounds.getY()+25, bounds.getWidth() * 0.20, bounds.getHeight()-25);
    releaseLabel.setBounds(compRelease.getX() + 54, bounds.getY(), compRelease.getWidth(), 25);

    mixSlider.setBounds(compRelease.getX() + compRelease.getWidth(), bounds.getY() + 25, bounds.getWidth() * 0.20, bounds.getHeight() - 25);
    mixLabel.setBounds(mixSlider.getX() + 64, bounds.getY(), mixSlider.getWidth(), 25);
}

