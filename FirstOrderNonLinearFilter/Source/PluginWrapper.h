/*
  ==============================================================================

    PluginWrapper.h
    Created: 8 May 2022 9:38:17pm
    Author:  StoneyDSP

  ==============================================================================
*/

#pragma once

#ifndef PLUGINWRAPPER_H_INCLUDED
#define PLUGINWRAPPER_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "Modules/FirstOrderNLFilter.h"

class FirstOrderNonLinearFilterAudioProcessor;

template <typename SampleType>
class ProcessWrapper
{
public:
    using APVTS = juce::AudioProcessorValueTreeState;
    using ProcessSpec = juce::dsp::ProcessSpec;
    //==========================================================================
    /** Constructor. */
    ProcessWrapper(FirstOrderNonLinearFilterAudioProcessor& p, APVTS& apvts, ProcessSpec& spec);

    //==========================================================================
    /** Initialises the processor. */
    void prepare(juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    //==========================================================================
    void process(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages);

    //==========================================================================
    /** Updates the internal state variables of the processor. */
    void update();

private:
    //==========================================================================
    // This reference is provided as a quick way for the wrapper to
    // access the processor object that created it.
    FirstOrderNonLinearFilterAudioProcessor& audioProcessor;
    APVTS& state;
    ProcessSpec& setup;

    //==========================================================================
    /** Instantiate objects. */
    juce::dsp::DryWetMixer<SampleType> mixer;
    FirstOrderNLfilter<SampleType> filter;
    juce::dsp::Gain<SampleType> output;

    //==========================================================================
    /** Parameter pointers. */
    juce::AudioParameterFloat* frequencyPtr{ nullptr };
    juce::AudioParameterFloat* resonancePtr{ nullptr };
    juce::AudioParameterFloat* gainPtr{ nullptr };
    juce::AudioParameterChoice* typePtr{ nullptr };
    juce::AudioParameterChoice* transformPtr{ nullptr };
    juce::AudioParameterFloat* outputPtr{ nullptr };
    juce::AudioParameterFloat* mixPtr{ nullptr };
    juce::AudioParameterBool* bypassPtr{ nullptr };

    //==========================================================================
    /** Init variables. */

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ProcessWrapper)
};

#endif //  PLUGINWRAPPER_H_INCLUDED