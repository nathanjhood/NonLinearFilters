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
    reset(static_cast<SampleType>(0.0));
}

//==============================================================================
template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setFrequency(SampleType newFreq)
{
    jassert(static_cast<SampleType>(20.0) <= newFreq && newFreq <= static_cast<SampleType>(20000.0));

    hz = static_cast<SampleType>(juce::jlimit(minFreq, maxFreq, newFreq));
    frq.setTargetValue(hz);
    coefficients();
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setGain(SampleType newGain)
{
    g = static_cast<SampleType>(newGain);
    lev.setTargetValue(g);
    coefficients();
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setFilterType(filterType newFiltType)
{
    if (filtType != newFiltType)
    {
        filtType = newFiltType;
        reset(static_cast<SampleType>(0.0));
        coefficients();
    }
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setTransformType(transformationType newTransformType)
{
    if (transformType != newTransformType)
    {
        transformType = newTransformType;
        reset(static_cast<SampleType>(0.0));
        coefficients();
    }
}

//==============================================================================
template <typename SampleType>
void FirstOrderNLfilter<SampleType>::setRampDurationSeconds(double newDurationSeconds) noexcept
{
    if (rampDurationSeconds != newDurationSeconds)
    {
        rampDurationSeconds = newDurationSeconds;
        reset(static_cast<SampleType>(0.0));
    }
}

template <typename SampleType>
double FirstOrderNLfilter<SampleType>::getRampDurationSeconds() const noexcept
{
    return rampDurationSeconds;
}

template <typename SampleType>
bool FirstOrderNLfilter<SampleType>::isSmoothing() const noexcept
{
    bool compSmoothing = frq.isSmoothing() || lev.isSmoothing();

    return compSmoothing;
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

    reset(static_cast<SampleType>(0.0));

    minFreq = static_cast <SampleType>(sampleRate) / static_cast <SampleType>(24576.0);
    maxFreq = static_cast <SampleType>(sampleRate) / static_cast <SampleType>(2.125);

    jassert(static_cast <SampleType>(20.0) >= minFreq && minFreq <= static_cast <SampleType>(20000.0));
    jassert(static_cast <SampleType>(20.0) <= maxFreq && maxFreq >= static_cast <SampleType>(20000.0));

    setFrequency(hz);
    setGain(g);

    coefficients();
}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::reset(SampleType initialValue)
{
    for (auto v : { &Wn_1, &Xn_1, &Yn_1, })
        std::fill(v->begin(), v->end(), initialValue);

    frq.reset(sampleRate, rampDurationSeconds);
    lev.reset(sampleRate, rampDurationSeconds);

    coefficients();
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::processSample(int channel, SampleType inputValue)
{
    jassert(juce::isPositiveAndBelow(channel, Wn_1.size()));
    jassert(juce::isPositiveAndBelow(channel, Xn_1.size()));
    jassert(juce::isPositiveAndBelow(channel, Yn_1.size()));

    switch (transformType)
    {
    case TransformationType::directFormI:
        inputValue = directFormI(channel, inputValue);
        break;
    case TransformationType::directFormII:
        inputValue = directFormII(channel, inputValue);
        break;
    case TransformationType::directFormItransposed:
        inputValue = directFormITransposed(channel, inputValue);
        break;
    case TransformationType::directFormIItransposed:
        inputValue = directFormIITransposed(channel, inputValue);
        break;
    default:
        inputValue = directFormIITransposed(channel, inputValue);
    }

    return inputValue;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::directFormI(int channel, SampleType inputValue)
{
    auto& Xn1 = Xn_1[(size_t)channel];
    auto& Yn1 = Yn_1[(size_t)channel];

    SampleType Xn = inputValue;

    SampleType Yn = ((Xn * b0) + (Xn1 * b1) + (Yn1 * a1));

    Xn1 = Xn, Yn1 = Yn;

    return Yn;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::directFormII(int channel, SampleType inputValue)
{
    auto& Wn1 = Wn_1[(size_t)channel];

    SampleType Xn = inputValue;

    SampleType Wn = (Xn + ((Wn1 * a1)));
    SampleType Yn = ((Wn * b0) + (Wn1 * b1));

    Wn1 = Wn;

    return Yn;
}

//template <typename SampleType>
//SampleType FirstOrderNLfilter<SampleType>::directFormITransposedDecramped???(int channel, SampleType inputValue)
//{
//    auto& Wn1 = Wn_1[(size_t)channel];
//
//    SampleType Xn = inputValue;
//
//    SampleType Wn = (Xn + Wn1);
//    SampleType Yn = ((Wn * b0) + (Wn * b1));
//
//    Wn1 = (Wn * a1);
//
//    return Yn;
//}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::directFormITransposed(int channel, SampleType inputValue)
{
    auto& Wn1 = Wn_1[(size_t)channel];
    auto& Yn1 = Yn_1[(size_t)channel];

    SampleType Xn = inputValue;

    SampleType Wn = (Xn + Wn1);
    SampleType Yn = ((Wn * b0) + Yn1);

    Wn1 = (Wn * a1), Yn1 = ((Wn * b1));

    return Yn;
}

template <typename SampleType>
SampleType FirstOrderNLfilter<SampleType>::directFormIITransposed(int channel, SampleType inputValue)
{
    auto& Xn1 = Xn_1[(size_t)channel];

    auto Xn = inputValue;

    auto Yn = ((Xn * b0) + Xn1);

    Xn1 = std::tanh((Xn * b1) + (Yn * a1));

    return Yn;
}

//template <typename SampleType>
//SampleType FirstOrderNLfilter<SampleType>::directFormIITransposedNLFeedback(int channel, SampleType inputValue)
//{
//    auto& Xn1 = Xn_1[(size_t)channel];
//
//    auto Xn = inputValue;
//
//    auto Yn = ((Xn * b0) + Xn1);
//
//    Xn1 = ((Xn * b1) + (std::tanh(Yn) * a1));
//
//    return Yn;
//}

template <typename SampleType>
void FirstOrderNLfilter<SampleType>::coefficients()
{
    SampleType omega = static_cast <SampleType>(frq.getNextValue() * ((pi * two) / sampleRate));
    SampleType cos = static_cast <SampleType>(std::cos(omega));
    SampleType sin = static_cast <SampleType>(std::sin(omega));
    SampleType tan = static_cast <SampleType>(sin / cos);
    SampleType a = static_cast <SampleType>(juce::Decibels::decibelsToGain(static_cast<SampleType>(lev.getNextValue() * static_cast <SampleType>(0.5))));

    juce::ignoreUnused(tan);

    SampleType b_0 = one;
    SampleType b_1 = zero;
    SampleType a_0 = one;
    SampleType a_1 = zero;

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

    a0 = static_cast <SampleType>(one / a_0);
    a1 = static_cast <SampleType>((a_1 * a0) * minusOne);
    b0 = static_cast <SampleType>(b_0 * a0);
    b1 = static_cast <SampleType>(b_1 * a0);
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