/*
  ==============================================================================

    Oversampler.h
    Created: 19 Jun 2022 9:57:18pm
    Author:  Nathan J. Hood

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#ifndef OVERSAMPLER_H_INCLUDED
#define OVERSAMPLER_H_INCLUDED

template <typename SampleType>
class Oversampler
{
public:
    //==============================================================================
    /** Constructor. */
    Oversampler(juce::dsp::ProcessSpec& spec);

    //==========================================================================
    /** Initialises the processor. */
    void prepare(juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset();

    //==========================================================================
    void processSamplesUp(juce::dsp::AudioBlock<SampleType>& block);

    void processSamplesDown(juce::dsp::AudioBlock<SampleType>& block);

    //==========================================================================
    /** Sets the oversampling factor. */
    void setOversampling(int newOS);

    SampleType getLatencySamples() const noexcept;

    int getOversamplingFactor() const noexcept { return oversamplingFactor; };

private:
    //==========================================================================
    // This reference is provided as a quick way for the wrapper to
    // access the processor object that created it.
    juce::dsp::ProcessSpec& setup;

    //==========================================================================
    /** Instantiate objects. */
    std::unique_ptr<juce::dsp::Oversampling<SampleType>> oversampler[5];

    //==========================================================================
    /** Parameter pointers. */
    juce::AudioParameterChoice* osPtr{ nullptr };

    //==========================================================================
    /** Init variables. */
    int curOS = 0, prevOS = 0, oversamplingFactor = 1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oversampler)
};

#endif //OVERSAMPLER_H_INCLUDED