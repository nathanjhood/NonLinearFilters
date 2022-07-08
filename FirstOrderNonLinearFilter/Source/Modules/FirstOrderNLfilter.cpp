/*
  ==============================================================================

    FirstOrderNLfilter.cpp
    Created: 19 Jun 2022 12:56:42am
    Author:  Nathan J. Hood

  ==============================================================================
*/

#include "FirstOrderNLfilter.h"

template <typename SampleType>
FirstOrderNLfilter<SampleType>::FirstOrderNLfilter()
{
    reset();
}

//==============================================================================
template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setFrequency(SampleType newFreq)
{
    jassert(minFreq <= newFreq && newFreq <= maxFreq);

    if (hz != newFreq)
    {
        hz = juce::jlimit(minFreq, maxFreq, newFreq);

        omega = (hz * ((pi * two) / static_cast <SampleType>(sampleRate)));
        cos = (std::cos(omega));
        sin = (std::sin(omega));

        coefficients();
    }
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setGain(SampleType newGain)
{
    if (g != newGain)
    {
        g = newGain;
        
        a = juce::Decibels::decibelsToGain(g * 0.5);

        coefficients();
    }
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setFilterType(filterType newFiltType)
{
    if (filtType != newFiltType)
    {
        filtType = newFiltType;
        reset();
        coefficients();
    }
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setSaturationType(satType newTransformType)
{
    if (saturationType != newTransformType)
    {
        saturationType = newTransformType;
        reset();
        coefficients();
    }
}

//==============================================================================
template <typename SampleType>
void FirstOrderNLfilter<SampleType>::prepare(juce::dsp::ProcessSpec& spec)
{
    jassert(spec.sampleRate > 0);
    jassert(spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    Wn_1.resize(spec.numChannels);
    Xn_1.resize(spec.numChannels);
    Yn_1.resize(spec.numChannels);

    reset();

    minFreq = static_cast <SampleType> (sampleRate / 24576.0);
    maxFreq = static_cast <SampleType> (sampleRate / 2.125);

    jassert(static_cast <SampleType> (20.0) >= minFreq && minFreq <= static_cast <SampleType> (20000.0));
    jassert(static_cast <SampleType> (20.0) <= maxFreq && maxFreq >= static_cast <SampleType> (20000.0));

    setFrequency(hz);
    setGain(g);

    coefficients();
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::reset(SampleType initialValue)
{
    for (auto v : { &Wn_1, &Xn_1, &Yn_1, })
        std::fill(v->begin(), v->end(), initialValue);
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::processSample(int channel, SampleType inputValue)
{
    jassert(juce::isPositiveAndBelow(channel, Wn_1.size()));
    jassert(juce::isPositiveAndBelow(channel, Xn_1.size()));
    jassert(juce::isPositiveAndBelow(channel, Yn_1.size()));

    switch (saturationType)
    {
    case SaturationType::linear:
        inputValue = linear(channel, inputValue);
        break;
    case SaturationType::nonlinear1:
        inputValue = nonlinear1(channel, inputValue);
        break;
    case SaturationType::nonlinear2:
        inputValue = nonlinear2(channel, inputValue);
        break;
    case SaturationType::nonlinear3:
        inputValue = nonlinear3(channel, inputValue);
        break;
    case SaturationType::nonlinear4:
        inputValue = nonlinear4(channel, inputValue);
        break;
    default:
        inputValue = linear(channel, inputValue);
    }

    return inputValue;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::linear(int channel, SampleType inputSample)
{
    auto& Xn1 = Xn_1[(size_t)channel];

    auto& Xn = inputSample;
    auto& Yn = outputSample;

    Yn = ((Xn * b0) + Xn1);

    Xn1 = ((Xn * b1) + (Yn * a1));

    return Yn;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::nonlinear1(int channel, SampleType inputSample)
{
    auto& Xn1 = Xn_1[(size_t)channel];

    auto& Xn = inputSample;
    auto& Yn = outputSample;

    Yn = ((Xn * b0) + Xn1);

    Xn1 = std::tanh((Xn * b1) + (Yn * a1));

    return Yn;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::nonlinear2(int channel, SampleType inputSample)
{
    auto& Xn1 = Xn_1[(size_t)channel];

    auto& Xn = inputSample;
    auto& Yn = outputSample;

    Yn = std::tanh((Xn * b0) + Xn1);

    Xn1 = ((Xn * b1) + ((Yn) * a1));

    return Yn;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::nonlinear3(int channel, SampleType inputSample)
{
    auto& Xn1 = Xn_1[(size_t)channel];

    auto& Xn = inputSample;
    auto& Yn = outputSample;

    Yn = std::tanh((Xn * b0) + Xn1);

    Xn1 = std::tanh((Xn * b1) + (Yn * a1));

    return Yn;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::nonlinear4(int channel, SampleType inputSample)
{
    auto& Xn1 = Xn_1[(size_t)channel];

    auto& Xn = std::sin(inputSample);
    auto& Yn = outputSample;

    Yn = ((Xn * b0) + Xn1);

    Xn1 = ((Xn * b1) + (Yn * a1));

    return std::asin(Yn);
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::coefficients()
{
    switch (filtType)
    {

    case filterType::lowPass:

        b_0 = omega / (one + omega);
        b_1 = omega / (one + omega);
        a_0 = one;
        a_1 = minusOne * ((one - omega) / (one + omega));

        break;


    case filterType::highPass:

        b_0 = one / (one + omega);
        b_1 = (one / (one + omega)) * minusOne;
        a_0 = one;
        a_1 = ((one - omega) / (one + omega)) * minusOne;

        break;


    case filterType::lowShelf:

        b_0 = one + ((omega / (one + omega)) * (minusOne + (a * a)));
        b_1 = (((omega / (one + omega)) * (minusOne + (a * a))) - ((one - omega) / (one + omega)));
        a_0 = one;
        a_1 = minusOne * ((one - omega) / (one + omega));

        break;


    case filterType::lowShelfC:

        b_0 = one + ((omega / a) / (one + (omega / a)) * (minusOne + (a * a)));
        b_1 = ((((omega / a) / (one + (omega / a))) * (minusOne + (a * a))) - ((one - (omega / a)) / (one + (omega / a))));
        a_0 = one;
        a_1 = minusOne * ((one - (omega / a)) / (one + (omega / a)));

        break;


    case filterType::highShelf:

        b_0 = one + ((minusOne + (a * a)) / (one + omega));
        b_1 = minusOne * (((one - omega) / (one + omega)) + ((minusOne + (a * a)) / (one + omega)));
        a_0 = one;
        a_1 = minusOne * ((one - omega) / (one + omega));

        break;


    case filterType::highShelfC:

        b_0 = one + ((minusOne + (a * a)) / (one + (omega * a)));
        b_1 = minusOne * (((one - (omega * a)) / (one + (omega * a))) + ((minusOne + (a * a)) / (one + (omega * a))));
        a_0 = one;
        a_1 = minusOne * ((one - (omega * a)) / (one + (omega * a)));

        break;


    default:

        b_0 = one;
        b_1 = zero;
        a_0 = one;
        a_1 = zero;

        break;
    }

    a0 = (one / a_0);
    a1 = ((-a_1) * a0);
    b0 = (b_0 * a0);
    b1 = (b_1 * a0);
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::snapToZero() noexcept
{
    for (auto v : { &Wn_1, &Xn_1, &Yn_1 })
        for (auto& element : *v)
            juce::dsp::util::snapToZero(element);
}

template class FirstOrderNLfilter<float>;
template class FirstOrderNLfilter<double>;