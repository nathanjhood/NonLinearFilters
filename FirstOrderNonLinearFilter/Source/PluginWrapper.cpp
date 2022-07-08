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
ProcessWrapper<SampleType>::ProcessWrapper(FirstOrderNonLinearFilterAudioProcessor& p) 
    : 
    audioProcessor(p),
    state(p.getAPVTS()),
    setup(p.getSpec()),
    mixer(),
    filter(),
    driveUp(),
    driveDn(),
    output(),
    frequencyPtr (dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("frequencyID"))),
    gainPtr (dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("gainID"))),
    typePtr (dynamic_cast <juce::AudioParameterChoice*> (p.getAPVTS().getParameter("typeID"))),
    linearityPtr (dynamic_cast <juce::AudioParameterChoice*> (p.getAPVTS().getParameter("linearityID"))),
    outputPtr (dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("outputID"))),
    mixPtr (dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("mixID"))),
    drivePtr (dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("driveID")))
{
    jassert(frequencyPtr != nullptr);
    jassert(gainPtr != nullptr);
    jassert(typePtr != nullptr);
    jassert(outputPtr != nullptr);
    jassert(mixPtr != nullptr);
    jassert(drivePtr != nullptr);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::prepare(juce::dsp::ProcessSpec& spec)
{
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
    //mixer.setWetLatency(static_cast<SampleType>(audioProcessor.getLatencySamples()));
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

    juce::dsp::AudioBlock<SampleType> block(buffer);

    mixer.pushDrySamples(block);

    juce::dsp::ProcessContextReplacing context(block);

    driveUp.process(context);

    filter.process(context);

    driveDn.process(context);

    output.process(context);

    mixer.mixWetSamples(block);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::update()
{
    mixer.setWetMixProportion(mixPtr->get() * 0.01f);
    driveUp.setGainDecibels(drivePtr->get() * static_cast<SampleType>(1.0));
    filter.setFrequency(frequencyPtr->get());
    filter.setGain(gainPtr->get());
    filter.setFilterType(static_cast<FilterType>(typePtr->getIndex()));
    filter.setSaturationType(static_cast<SaturationType>(linearityPtr->getIndex()));
    driveDn.setGainDecibels(drivePtr->get() * static_cast<SampleType>(-1.0));
    output.setGainDecibels(outputPtr->get());
}

//==============================================================================
template class ProcessWrapper<float>;
template class ProcessWrapper<double>;