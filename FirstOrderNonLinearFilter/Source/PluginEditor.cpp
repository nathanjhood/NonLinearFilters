/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FirstOrderNonLinearFilterAudioProcessorEditor::FirstOrderNonLinearFilterAudioProcessorEditor (FirstOrderNonLinearFilterAudioProcessor& p, APVTS& apvts, juce::UndoManager& um)
    :
    juce::AudioProcessorEditor(&p),
    audioProcessor(p),
    state(apvts),
    undoManager(um),
    subComponents(p, apvts)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    addAndMakeVisible(subComponents);
    addAndMakeVisible(undoButton);
    addAndMakeVisible(redoButton);
    undoButton.onClick = [this] { undoManager.undo(); };
    redoButton.onClick = [this] { undoManager.redo(); };
    setResizable(true, true);
    setSize(650, 300);

    startTimerHz(60);
}

FirstOrderNonLinearFilterAudioProcessorEditor::~FirstOrderNonLinearFilterAudioProcessorEditor()
{
}

void FirstOrderNonLinearFilterAudioProcessorEditor::timerCallback()
{
}

//==============================================================================
void FirstOrderNonLinearFilterAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::darkslateblue);

    //// draw an outline around the component
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds(), 2);

    // Add project info text to background here
    g.setColour(juce::Colours::antiquewhite);
    g.setFont(15.0f);
    g.drawFittedText(ProjectInfo::companyName, getLocalBounds(), juce::Justification::topLeft, 1);
    g.drawFittedText(ProjectInfo::projectName, getLocalBounds(), juce::Justification::topRight, 1);
    g.drawFittedText(ProjectInfo::versionString, getLocalBounds(), juce::Justification::bottomLeft, 1);
}

void FirstOrderNonLinearFilterAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    subComponents.setBounds(0, 0, getWidth(), getHeight());
    undoButton.setBounds((getWidth() / 2) - 10, getHeight() - 20, 20, 20);
    redoButton.setBounds((getWidth() / 2) + 10, getHeight() - 20, 20, 20);
    subComponents.resized();
    undoButton.resized();
    redoButton.resized();
}
