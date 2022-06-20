/*
  ==============================================================================

    PluginWrapper.cpp
    Created: 8 May 2022 9:38:17pm
    Author:  StoneyDSP

  ==============================================================================
*/

#include "PluginWrapper.h"
#include "PluginProcessor.h"

template <typename SampleType>
ProcessWrapper<SampleType>::ProcessWrapper(SecondOrderNonLinearFilterAudioProcessor& p, APVTS& apvts, ProcessSpec& spec)
    :
    audioProcessor(p),
    state(apvts),
    setup(spec)
{
    setup.sampleRate = audioProcessor.getSampleRate();
    setup.maximumBlockSize = audioProcessor.getBlockSize();
    setup.numChannels = audioProcessor.getTotalNumInputChannels();

    frequencyPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("frequencyID"));
    gainPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("gainID"));
    typePtr = dynamic_cast <juce::AudioParameterChoice*> (state.getParameter("typeID"));
    outputPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("outputID"));
    mixPtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("mixID"));
    bypassPtr = dynamic_cast <juce::AudioParameterBool*> (state.getParameter("bypassID"));
    drivePtr = dynamic_cast <juce::AudioParameterFloat*> (state.getParameter("driveID"));

    jassert(frequencyPtr != nullptr);
    jassert(gainPtr != nullptr);
    jassert(typePtr != nullptr);
    jassert(outputPtr != nullptr);
    jassert(mixPtr != nullptr);
    jassert(bypassPtr != nullptr);
    jassert(drivePtr != nullptr);

    filter.setTransformType(TransformationType::directFormIItransposed);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::prepare(juce::dsp::ProcessSpec& spec)
{
    spec.sampleRate = audioProcessor.getSampleRate();
    spec.maximumBlockSize = audioProcessor.getBlockSize();
    spec.numChannels = audioProcessor.getTotalNumInputChannels();

    mixer.prepare(spec);
    driveUp.prepare(spec);
    filter.prepare(spec);
    driveDn.prepare(spec);
    output.prepare(spec);

    reset();
    update();
}

template <typename SampleType>
void ProcessWrapper<SampleType>::reset()
{
    mixer.reset();
    mixer.setWetLatency(audioProcessor.getLatencySamples());
    driveUp.reset();
    filter.reset(static_cast<SampleType>(0.0));
    driveDn.reset();
    output.reset();
}

//==============================================================================
template <typename SampleType>
void ProcessWrapper<SampleType>::process(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    update();

    auto block = juce::dsp::AudioBlock<SampleType>(buffer);

    mixer.pushDrySamples(block);

    auto context = juce::dsp::ProcessContextReplacing(block);

    context.isBypassed = bypassPtr->get();

    driveUp.process(context);

    filter.process(context);

    driveDn.process(context);

    output.process(context);

    mixer.mixWetSamples(block);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::update()
{
    setup.sampleRate = audioProcessor.getSampleRate();
    setup.maximumBlockSize = audioProcessor.getBlockSize();
    setup.numChannels = audioProcessor.getTotalNumInputChannels();

    audioProcessor.setBypassParameter(bypassPtr);

    mixer.setWetMixProportion(mixPtr->get() * 0.01f);
    driveUp.setGainLinear(juce::Decibels::decibelsToGain(drivePtr->get()));
    filter.setFrequency(frequencyPtr->get());
    filter.setGain(gainPtr->get());
    filter.setFilterType(static_cast<FilterType>(typePtr->getIndex()));
    driveDn.setGainLinear(juce::Decibels::decibelsToGain(drivePtr->get() * static_cast<SampleType>(-1.0)));
    output.setGainLinear(juce::Decibels::decibelsToGain(outputPtr->get()));
}

//==============================================================================
template class ProcessWrapper<float>;
template class ProcessWrapper<double>;