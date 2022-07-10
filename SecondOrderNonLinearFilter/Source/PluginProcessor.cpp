/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SecondOrderNonLinearFilterAudioProcessor::SecondOrderNonLinearFilterAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
    apvts(*this, &undoManager, "Parameters", createParameterLayout()),
    spec(),
    parameters(*this),
    processorFloat(*this),
    processorDouble(*this),
    bypassState(dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter("bypassID"))),
    processingPrecision(singlePrecision)
{
}

SecondOrderNonLinearFilterAudioProcessor::~SecondOrderNonLinearFilterAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorParameter* SecondOrderNonLinearFilterAudioProcessor::getBypassParameter() const
{
    return bypassState;
}

bool SecondOrderNonLinearFilterAudioProcessor::isBypassed() const noexcept
{
    return bypassState->get() == true;
}

void SecondOrderNonLinearFilterAudioProcessor::setBypassParameter(juce::AudioParameterBool* newBypass) noexcept
{
    if (bypassState != newBypass)
    {
        bypassState = newBypass;
        releaseResources();
        reset();
    }

}

bool SecondOrderNonLinearFilterAudioProcessor::supportsDoublePrecisionProcessing() const
{
    return false;
}

juce::AudioProcessor::ProcessingPrecision SecondOrderNonLinearFilterAudioProcessor::getProcessingPrecision() const noexcept
{
    return processingPrecision;
}

bool SecondOrderNonLinearFilterAudioProcessor::isUsingDoublePrecision() const noexcept
{
    return processingPrecision == doublePrecision;
}

void SecondOrderNonLinearFilterAudioProcessor::setProcessingPrecision(ProcessingPrecision newPrecision) noexcept
{
    // If you hit this assertion then you're trying to use double precision
    // processing on a processor which does not support it!
    jassert(newPrecision != doublePrecision || supportsDoublePrecisionProcessing());

    if (processingPrecision != newPrecision)
    {
        processingPrecision = newPrecision;
        releaseResources();
        reset();
    }
}

//==============================================================================
const juce::String SecondOrderNonLinearFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SecondOrderNonLinearFilterAudioProcessor::acceptsMidi() const
{
    return false;
}

bool SecondOrderNonLinearFilterAudioProcessor::producesMidi() const
{
    return false;
}

bool SecondOrderNonLinearFilterAudioProcessor::isMidiEffect() const
{
    return false;
}

double SecondOrderNonLinearFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SecondOrderNonLinearFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SecondOrderNonLinearFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SecondOrderNonLinearFilterAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String SecondOrderNonLinearFilterAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void SecondOrderNonLinearFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void SecondOrderNonLinearFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    juce::ignoreUnused(sampleRate, samplesPerBlock);

    processingPrecision = getProcessingPrecision();

    spec.sampleRate = getSampleRate();
    spec.maximumBlockSize = getBlockSize();
    spec.numChannels = getTotalNumInputChannels();

    processorFloat.prepare(getSpec());
    processorDouble.prepare(getSpec());
}

void SecondOrderNonLinearFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    processorFloat.reset();
    processorDouble.reset();
}

bool SecondOrderNonLinearFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;

    return true;
}

void SecondOrderNonLinearFilterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    if (bypassState->get())
    {
        processBlockBypassed(buffer, midiMessages);
    }

    else
    {
        juce::ScopedNoDenormals noDenormals;

        processorFloat.process(buffer, midiMessages);
    }
}

void SecondOrderNonLinearFilterAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    if (bypassState->get())
    {
        processBlockBypassed(buffer, midiMessages);
    }

    else
    {
        juce::ScopedNoDenormals noDenormals;

        processorDouble.process(buffer, midiMessages);
    }
}

void SecondOrderNonLinearFilterAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    juce::dsp::AudioBlock<float> block(buffer);
    juce::dsp::ProcessContextReplacing context(block);

    const auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    outputBlock.copyFrom(inputBlock);
}

void SecondOrderNonLinearFilterAudioProcessor::processBlockBypassed(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    midiMessages.clear();

    juce::dsp::AudioBlock<double> block(buffer);
    juce::dsp::ProcessContextReplacing context(block);

    const auto& inputBlock = context.getInputBlock();
    auto& outputBlock = context.getOutputBlock();

    outputBlock.copyFrom(inputBlock);
}

//==============================================================================
bool SecondOrderNonLinearFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SecondOrderNonLinearFilterAudioProcessor::createEditor()
{
    return new SecondOrderNonLinearFilterAudioProcessorEditor(*this);
}

juce::AudioProcessorValueTreeState::ParameterLayout SecondOrderNonLinearFilterAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout params;

    params.add(std::make_unique<juce::AudioParameterBool>("bypassID", "Bypass", false));

    Parameters::setParameterLayout(params);

    return params;
}

//==============================================================================
void SecondOrderNonLinearFilterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void SecondOrderNonLinearFilterAudioProcessor::getCurrentProgramStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}


void SecondOrderNonLinearFilterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void SecondOrderNonLinearFilterAudioProcessor::setCurrentProgramStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SecondOrderNonLinearFilterAudioProcessor();
}
