/*
  ==============================================================================

    SecondOrderNLfilter.h
    Created: 19 Jun 2022 12:56:42am
    Author:  Nathan J. Hood

  ==============================================================================
*/

#pragma once

#ifndef SECONDORDERNLFILTER_H_INCLUDED
#define SECONDORDERNLFILTER_H_INCLUDED

#include <JuceHeader.h>

#include "Coefficient.h"

enum struct FilterType
{
    lowPass2 = 0,
    lowPass1 = 1,
    highPass2 = 2,
    highPass1 = 3,
    bandPass = 4,
    bandPassQ = 5,
    lowShelf2 = 6,
    lowShelf1 = 7,
    lowShelf1C = 8,
    highShelf2 = 9,
    highShelf1 = 10,
    highShelf1C = 11,
    peak = 12,
    notch = 13,
    allPass = 14
};

enum class SaturationType
{
    linear = 0,
    nonlinear1 = 1,
    nonlinear2 = 2,
    nonlinear3 = 3,
    nonlinear4 = 4
};


template <typename SampleType>
class SecondOrderNLfilter
{
public:
    using filterType = FilterType;
    using satType = SaturationType;
    //==============================================================================
    /** Constructor. */
    SecondOrderNLfilter();

    //==============================================================================
    /** Sets the centre Frequency of the filter. Range = 20..20000 */
    void setFrequency(SampleType newFreq);

    /** Sets the resonance of the filter. Range = 0..1 */
    void setResonance(SampleType newRes);

    /** Sets the centre Frequency gain of the filter. Peak and shelf modes only. */
    void setGain(SampleType newGain);

    /** Sets the type of the filter. See enum for available types. */
    void setFilterType(filterType newFiltType);

    /** Sets the saturation position for the filter to use. See enum for available types. */
    void setSaturationType(satType newTransformType);

    //==============================================================================
    /** Initialises the processor. */
    void prepare(juce::dsp::ProcessSpec& spec);

    /** Resets the internal state variables of the processor. */
    void reset(SampleType initialValue = {0.0});

    /** Ensure that the state variables are rounded to zero if the state
    variables are denormals. This is only needed if you are doing sample
    by sample processing.*/
    void snapToZero() noexcept;

    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    template <typename ProcessContext>
    void process(const ProcessContext& context) noexcept
    {
        const auto& inputBlock = context.getInputBlock();
        auto& outputBlock = context.getOutputBlock();
        const auto numChannels = outputBlock.getNumChannels();
        const auto numSamples = outputBlock.getNumSamples();

        jassert(inputBlock.getNumChannels() == numChannels);
        jassert(inputBlock.getNumSamples() == numSamples);

        if (context.isBypassed)
        {

            outputBlock.copyFrom(inputBlock);
            return;
        }

        for (size_t channel = 0; channel < numChannels; ++channel)
        {
            auto* inputSamples = inputBlock.getChannelPointer(channel);
            auto* outputSamples = outputBlock.getChannelPointer(channel);

            for (size_t i = 0; i < numSamples; ++i)
                outputSamples[i] = processSample((int)channel, inputSamples[i]);
        }

#if JUCE_DSP_ENABLE_SNAP_TO_ZERO
        snapToZero();
#endif
    }

    //==============================================================================
    /** Processes one sample at a time on a given channel. */
    SampleType processSample(int channel, SampleType inputValue);

private:
    //==============================================================================
    /*SampleType& getb0() { return b0; }
    SampleType& getb1() { return b1; }
    SampleType& getb2() { return b2; }
    SampleType& geta0() { return a0; }
    SampleType& geta1() { return a1; }
    SampleType& geta2() { return a2; }*/

    //==============================================================================
    void coefficients();

    //==============================================================================
    SampleType linear(int channel, SampleType inputValue);
    SampleType nonlinear1(int channel, SampleType inputValue);
    SampleType nonlinear2(int channel, SampleType inputValue);
    SampleType nonlinear3(int channel, SampleType inputValue);
    SampleType nonlinear4(int channel, SampleType inputValue);

    //==============================================================================
    /** Unit-delay objects. */
    std::vector<SampleType> Wn_1, Wn_2, Xn_1, Xn_2, Yn_1, Yn_2;

    //==========================================================================
    /** Coefficient gain */
    Coefficient<SampleType> b0, b1, b2, a0, a1, a2;

    /** Coefficient calculation */
    Coefficient<SampleType> b_0, b_1, b_2, a_0, a_1, a_2;

    //==========================================================================
    /** Initialised parameter */
    SampleType loop = 0.0, outputSample = 0.0;
    SampleType minFreq = 20.0, maxFreq = 20000.0, hz = 1000.0, q = 0.5, g = 0.0;
    filterType filtType = filterType::lowPass2;
    satType saturationType = satType::linear;

    SampleType omega, cos, sin, tan, alpha, a, sqrtA, omegaDivA, omegaMulA{ 0.0 };

    //==============================================================================
    /** Initialise constants. */
    const SampleType zero = (0.0), one = (+1.0), two = (+2.0), minusOne = (-1.0), minusTwo = (-2.0);
    const SampleType pi = (juce::MathConstants<SampleType>::pi);
    double sampleRate = 48000.0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SecondOrderNLfilter)
};

#endif //SECONDORDERNLFILTER_H_INCLUDED
