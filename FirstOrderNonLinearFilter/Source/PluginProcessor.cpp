/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
FirstOrderNonLinearFilterAudioProcessor::FirstOrderNonLinearFilterAudioProcessor()
     : AudioProcessor (BusesProperties()
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                       ),
    apvts(*this, &undoManager, "Parameters", createParameterLayout()),
    parameters(*this),
    processorFloat(*this),
    processorDouble(*this)
{
    bypassState = dynamic_cast <juce::AudioParameterBool*> (this->getAPVTS().getParameter("bypassID"));
}

FirstOrderNonLinearFilterAudioProcessor::~FirstOrderNonLinearFilterAudioProcessor()
{
}

//==============================================================================
juce::AudioProcessorParameter* FirstOrderNonLinearFilterAudioProcessor::getBypassParameter() const
{
    return bypassState;
}

bool FirstOrderNonLinearFilterAudioProcessor::isBypassed() const noexcept
{
    return bypassState->get() == true;
}

void FirstOrderNonLinearFilterAudioProcessor::setBypassParameter(juce::AudioParameterBool* newBypass) noexcept
{
    if (bypassState != newBypass)
    {
        bypassState = newBypass;
        releaseResources();
        reset();
    }
}

bool FirstOrderNonLinearFilterAudioProcessor::supportsDoublePrecisionProcessing() const
{
    return false;
}

juce::AudioProcessor::ProcessingPrecision FirstOrderNonLinearFilterAudioProcessor::getProcessingPrecision() const noexcept
{
    return processingPrecision;
}

bool FirstOrderNonLinearFilterAudioProcessor::isUsingDoublePrecision() const noexcept
{
    return processingPrecision == doublePrecision;
}

void FirstOrderNonLinearFilterAudioProcessor::setProcessingPrecision(ProcessingPrecision newPrecision) noexcept
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
const juce::String FirstOrderNonLinearFilterAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool FirstOrderNonLinearFilterAudioProcessor::acceptsMidi() const
{
    return false;
}

bool FirstOrderNonLinearFilterAudioProcessor::producesMidi() const
{
    return false;
}

bool FirstOrderNonLinearFilterAudioProcessor::isMidiEffect() const
{
    return false;
}

double FirstOrderNonLinearFilterAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int FirstOrderNonLinearFilterAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int FirstOrderNonLinearFilterAudioProcessor::getCurrentProgram()
{
    return 0;
}

void FirstOrderNonLinearFilterAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String FirstOrderNonLinearFilterAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void FirstOrderNonLinearFilterAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void FirstOrderNonLinearFilterAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    
    update();

    processorFloat.prepare(getSpec());
    processorDouble.prepare(getSpec());
}

void FirstOrderNonLinearFilterAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    processorFloat.reset();
    processorDouble.reset();
}

bool FirstOrderNonLinearFilterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void FirstOrderNonLinearFilterAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    update();

    if (isBypassed() == true)
    {
        processBlockBypassed(buffer, midiMessages);
    }

    else
    {
        juce::ScopedNoDenormals noDenormals;

        processorFloat.process(buffer, midiMessages);
    }
}

void FirstOrderNonLinearFilterAudioProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    update();

    if (isBypassed() == true)
    {
        processBlockBypassed(buffer, midiMessages);
    }

    else
    {
        juce::ScopedNoDenormals noDenormals;

        processorDouble.process(buffer, midiMessages);
    }
}

void FirstOrderNonLinearFilterAudioProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(buffer);
    juce::ignoreUnused(midiMessages);
}

void FirstOrderNonLinearFilterAudioProcessor::processBlockBypassed(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(buffer);
    juce::ignoreUnused(midiMessages);
}

//==============================================================================
bool FirstOrderNonLinearFilterAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* FirstOrderNonLinearFilterAudioProcessor::createEditor()
{
    return new FirstOrderNonLinearFilterAudioProcessorEditor (*this);
}

juce::AudioProcessorValueTreeState::ParameterLayout FirstOrderNonLinearFilterAudioProcessor::createParameterLayout()
{
    APVTS::ParameterLayout params;

    Parameters::setParameterLayout(params);

    return params;
}

//==============================================================================
void FirstOrderNonLinearFilterAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void FirstOrderNonLinearFilterAudioProcessor::getCurrentProgramStateInformation(juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}


void FirstOrderNonLinearFilterAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void FirstOrderNonLinearFilterAudioProcessor::setCurrentProgramStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(apvts.state.getType()))
            apvts.replaceState(juce::ValueTree::fromXml(*xmlState));
}

void FirstOrderNonLinearFilterAudioProcessor::update()
{
    spec.sampleRate = getSampleRate();
    spec.maximumBlockSize = getBlockSize();
    spec.numChannels = getTotalNumInputChannels();
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new FirstOrderNonLinearFilterAudioProcessor();
}
