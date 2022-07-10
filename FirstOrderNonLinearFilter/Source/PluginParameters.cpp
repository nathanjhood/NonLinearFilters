/*
  ==============================================================================

    PluginParameters.cpp
    Created: 29 May 2022 7:58:00pm
    Author:  Nathan J. Hood

  ==============================================================================
*/

#include "PluginParameters.h"
#include "PluginProcessor.h"

Parameters::Parameters(FirstOrderNonLinearFilterAudioProcessor& p) : audioProcessor(p)
{
}

void Parameters::setParameterLayout(Params& params)
{
    const auto dBMax = juce::Decibels::gainToDecibels(16.0f);
    const auto dBMin = juce::Decibels::gainToDecibels(0.0625f);
    const auto dBOut = juce::Decibels::gainToDecibels(0.5f, -120.0f) * 20.0f;

    const auto freqRange = juce::NormalisableRange<float>(20.00f, 20000.00f, 0.001f, 00.198894f);
    const auto resRange = juce::NormalisableRange<float>(00.00f, 1.00f, 0.01f, 1.00f);
    const auto gainRange = juce::NormalisableRange<float>(dBMin, dBMax, 0.01f, 1.00f);
    const auto mixRange = juce::NormalisableRange<float>(00.00f, 100.00f, 0.01f, 1.00f);
    const auto outputRange = juce::NormalisableRange<float>(dBOut, dBMax, 0.01f, 1.00f);

    const auto fString = juce::StringArray({ "LP", "HP", "LS", "HS" , "LS(c)", "HS(c)" });
    const auto tString = juce::StringArray({ "Linear", "NL1", "NL2", "NL3", "NL4" });
    const auto osString = juce::StringArray({ "--", "2x", "4x", "8x", "16x" });

    const auto decibels = juce::String{ ("dB") };
    const auto frequency = juce::String{ ("Hz") };
    const auto reso = juce::String{ ("q") };
    const auto percentage = juce::String{ ("%") };

    auto genParam = juce::AudioProcessorParameter::genericParameter;
    auto inMeter = juce::AudioProcessorParameter::inputMeter;
    auto outParam = juce::AudioProcessorParameter::outputGain;
    auto outMeter = juce::AudioProcessorParameter::outputMeter;

    juce::ignoreUnused(inMeter);
    juce::ignoreUnused(outMeter);

    //auto lambda = [](float value, int) {return static_cast<juce::String>(round(value * 100.f) / 100.f); };

    auto freqAttributes = juce::AudioParameterFloatAttributes()
        .withLabel(frequency)
        .withCategory(genParam);

    auto gainAttributes = juce::AudioParameterFloatAttributes()
        .withLabel(decibels)
        .withCategory(genParam);

    auto mixAttributes = juce::AudioParameterFloatAttributes()
        .withLabel(percentage)
        .withCategory(genParam);

    auto outputAttributes = juce::AudioParameterFloatAttributes()
        .withLabel(decibels)
        .withCategory(outParam);

    //auto linearityAttributes = juce::AudioParameterChoiceAttributes()
        //.withCategory(genParam);

    //const auto linearityParam = std::make_unique<juce::AudioParameterChoice>("linearityID", "Saturation", tString, 0, linearityAttributes);

    params.add
        //======================================================================
        (std::make_unique<juce::AudioProcessorParameterGroup>("BandOneID", "0", "seperatorA",
            //==================================================================
            std::make_unique<juce::AudioParameterFloat>("frequencyID", "Frequency", freqRange, 632.455f, freqAttributes),
            std::make_unique<juce::AudioParameterFloat>("gainID", "Shelf +/-", gainRange, 00.00f, gainAttributes),
            std::make_unique<juce::AudioParameterFloat>("driveID", "Drive", gainRange, 00.00f, gainAttributes),
            std::make_unique<juce::AudioParameterChoice>("typeID", "Type", fString, 0),
            std::make_unique<juce::AudioParameterChoice>("linearityID", "Saturation", tString, 0)
            //==================================================================
            ));

    params.add
        //======================================================================
        (std::make_unique<juce::AudioProcessorParameterGroup>("masterID", "1", "seperatorB",
            //==================================================================
            std::make_unique<juce::AudioParameterChoice>("osID", "Oversampling", osString, 0),
            std::make_unique<juce::AudioParameterFloat>("outputID", "Output", outputRange, 00.00f, outputAttributes),
            std::make_unique<juce::AudioParameterFloat>("mixID", "Mix", mixRange, 100.00f, mixAttributes)
            //==================================================================
            ));
}
