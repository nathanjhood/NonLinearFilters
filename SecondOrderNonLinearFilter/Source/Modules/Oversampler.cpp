/*
  ==============================================================================

    Oversampler.cpp
    Created: 19 Jun 2022 9:57:18pm
    Author:  Nathan J. Hood

  ==============================================================================
*/

#include "Oversampler.h"

template <typename SampleType>
Oversampler<SampleType>::Oversampler(juce::dsp::ProcessSpec& spec) : setup(spec)
{
    auto osFilter = juce::dsp::Oversampling<SampleType>::filterHalfBandPolyphaseIIR;

    for (int i = 0; i < 5; ++i)
        oversampler[i] = std::make_unique<juce::dsp::Oversampling<SampleType>>
        (setup.numChannels, i, osFilter, true, false);
}

template <typename SampleType>
void Oversampler<SampleType>::prepare(juce::dsp::ProcessSpec& spec)
{
    oversamplingFactor = 1 << curOS;
    prevOS = curOS;

    for (int i = 0; i < 5; ++i)
        oversampler[i]->initProcessing(spec.maximumBlockSize);

    for (int i = 0; i < 5; ++i)
        oversampler[i]->numChannels = (size_t)spec.numChannels;
}

template <typename SampleType>
void Oversampler<SampleType>::reset()
{
    for (int i = 0; i < 5; ++i)
        oversampler[i]->reset();
}

template <typename SampleType>
void Oversampler<SampleType>::processSamplesUp(juce::dsp::AudioBlock<SampleType>& block)
{
    oversampler[curOS]->processSamplesUp(block);
}

template <typename SampleType>
void Oversampler<SampleType>::processSamplesDown(juce::dsp::AudioBlock<SampleType>& block)
{
    oversampler[curOS]->processSamplesDown(block);
}

template <typename SampleType>
void Oversampler<SampleType>::setOversampling(int newOS)
{
    curOS = newOS;
    if (curOS != prevOS)
    {
        oversamplingFactor = 1 << curOS;
        prevOS = curOS;
    }
}

template <typename SampleType>
SampleType Oversampler<SampleType>::getLatencySamples() const noexcept
{
    // latency of oversampling
    return oversampler[curOS]->getLatencyInSamples();
}

template class Oversampler<float>;
template class Oversampler<double>;