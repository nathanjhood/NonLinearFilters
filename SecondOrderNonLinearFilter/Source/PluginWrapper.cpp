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
ProcessWrapper<SampleType>::ProcessWrapper(SecondOrderNonLinearFilterAudioProcessor& p)
    :
    audioProcessor (p),
    state (p.getAPVTS()),
    setup (p.getSpec()),
    frequencyPtr ( dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("frequencyID"))),
    resonancePtr ( dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("resonanceID"))),
    gainPtr ( dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("gainID"))),
    typePtr ( dynamic_cast <juce::AudioParameterChoice*> (p.getAPVTS().getParameter("typeID"))),
    linearityPtr ( dynamic_cast <juce::AudioParameterChoice*> (p.getAPVTS().getParameter("linearityID"))),
    osPtr ( dynamic_cast <juce::AudioParameterChoice*> (p.getAPVTS().getParameter("osID"))),
    outputPtr ( dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("outputID"))),
    mixPtr ( dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("mixID"))),
    bypassPtr ( dynamic_cast <juce::AudioParameterBool*> (p.getAPVTS().getParameter("bypassID"))),
    drivePtr ( dynamic_cast <juce::AudioParameterFloat*> (p.getAPVTS().getParameter("driveID")))
{
    jassert(frequencyPtr != nullptr);
    jassert(resonancePtr != nullptr);
    jassert(gainPtr != nullptr);
    jassert(typePtr != nullptr);
    jassert(linearityPtr != nullptr);
    jassert(osPtr != nullptr);
    jassert(outputPtr != nullptr);
    jassert(mixPtr != nullptr);
    jassert(bypassPtr != nullptr);
    jassert(drivePtr != nullptr);

    auto osFilter = juce::dsp::Oversampling<SampleType>::filterHalfBandFIREquiripple;

    for (int i = 0; i < 5; ++i)
        oversampler[i] = std::make_unique<juce::dsp::Oversampling<SampleType>>
        (audioProcessor.getTotalNumInputChannels(), i, osFilter, true, false);

    reset();
}

template <typename SampleType>
void ProcessWrapper<SampleType>::prepare(juce::dsp::ProcessSpec& spec)
{
    oversamplingFactor = 1 << curOS;
    prevOS = curOS;

    for (int i = 0; i < 5; ++i)
        oversampler[i]->initProcessing(spec.maximumBlockSize);

    for (int i = 0; i < 5; ++i)
        oversampler[i]->numChannels = (size_t)spec.numChannels;

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
    filter.reset();
    driveDn.reset();
    output.reset();

    for (int i = 0; i < 5; ++i)
        oversampler[i]->reset();
}

//==============================================================================
template <typename SampleType>
void ProcessWrapper<SampleType>::process(juce::AudioBuffer<SampleType>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    update();
    setOversampling();

    juce::dsp::AudioBlock<SampleType> block(buffer);
    juce::dsp::AudioBlock<SampleType> osBlock(buffer);

    mixer.pushDrySamples(block);

    osBlock = oversampler[curOS]->processSamplesUp(block);

    juce::dsp::ProcessContextReplacing context(osBlock);

    driveUp.process(context);

    filter.process(context);

    driveDn.process(context);

    output.process(context);

    oversampler[curOS]->processSamplesDown(block);

    mixer.mixWetSamples(block);
}

template <typename SampleType>
void ProcessWrapper<SampleType>::update()
{
    mixer.setWetMixProportion(mixPtr->get() * 0.01f);
    driveUp.setGainDecibels(drivePtr->get());
    filter.setFrequency(frequencyPtr->get() / oversamplingFactor);
    filter.setResonance(resonancePtr->get());
    filter.setGain(gainPtr->get());
    filter.setFilterType(static_cast<FilterType>(typePtr->getIndex()));
    filter.setSaturationType(static_cast<SaturationType>(linearityPtr->getIndex()));
    driveDn.setGainDecibels(drivePtr->get() * static_cast<SampleType>(-1.0));
    output.setGainDecibels(outputPtr->get());
}

template <typename SampleType>
void ProcessWrapper<SampleType>::setOversampling()
{
    curOS = (int)osPtr->getIndex();
    if (curOS != prevOS)
    {
        oversamplingFactor = 1 << curOS;
        prevOS = curOS;
        mixer.reset();
        filter.reset();
        output.reset();
    }
}

//==============================================================================
template class ProcessWrapper<float>;
template class ProcessWrapper<double>;