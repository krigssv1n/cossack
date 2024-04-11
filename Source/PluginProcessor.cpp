/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CossackAudioProcessor::CossackAudioProcessor()
	:
#ifndef JucePlugin_PreferredChannelConfigurations
     AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
#else
#endif
	valueTreeState_(*this, nullptr, juce::Identifier("NewProjectParameters"), createParameterLayout())
{
	// Low/high cut
	parameters_.lowCut = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("lowCut"));
	parameters_.highCut = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("highCut"));

	// Equalizer
	for (int i = 0; i < 10; i++)
		parameters_.equalizer[i] = static_cast<juce::AudioParameterFloat*>(valueTreeState_.getParameter("equalizer" + std::to_string(i)));

	// Mid/side
	parameters_.mid = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("mid"));
	parameters_.midSide = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("midSide"));
	parameters_.side = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("side"));

	// Compressors
	parameters_.opto = static_cast<juce::AudioParameterFloat*>(valueTreeState_.getParameter("opto"));
	parameters_.glue = static_cast<juce::AudioParameterFloat*>(valueTreeState_.getParameter("glue"));
}

CossackAudioProcessor::~CossackAudioProcessor()
{
}

//==============================================================================
const juce::String CossackAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool CossackAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool CossackAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool CossackAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double CossackAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int CossackAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int CossackAudioProcessor::getCurrentProgram()
{
    return 0;
}

void CossackAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String CossackAudioProcessor::getProgramName (int index)
{
    return {};
}

void CossackAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void CossackAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	midLowCutProcessor_.prepareToPlay(sampleRate, samplesPerBlock);
	sideLowCutProcessor_.prepareToPlay(sampleRate, samplesPerBlock);
	highCutProcessor_.prepareToPlay(sampleRate, samplesPerBlock);
}

void CossackAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool CossackAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void CossackAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
	if (totalNumInputChannels == 2)
	{
		// TODO: optimize the order
		
		// stereo input, perform the mid/side processing
		const float* samples[] = { buffer.getReadPointer(0), buffer.getReadPointer(1) };

		// construct the buffers
		juce::AudioBuffer<float> midBuffer(buffer.getNumChannels(), buffer.getNumSamples());
		float* midSamples[] = { midBuffer.getWritePointer(0), midBuffer.getWritePointer(1) };

		juce::AudioBuffer<float> sideBuffer(buffer.getNumChannels(), buffer.getNumSamples());
		float* sideSamples[] = { sideBuffer.getWritePointer(0), sideBuffer.getWritePointer(1) };

		for (auto i = 0; i < buffer.getNumSamples(); i++)
		{
			float mid = (samples[0][i] + samples[1][i]) * 0.5f;

			midSamples[0][i] = midSamples[1][i] = mid;

			sideSamples[0][i] = samples[0][i] - mid;
			sideSamples[1][i] = samples[1][i] - mid;

			/*
			 s0 = 2 * s0 - s0 - s1 = s0 - s1 
			 s1 = 2 * s1 - s0 - s1 = s1 - s0
			*/
		}

		// pass more low frequencies in the mid, cut more in the side
		if (parameters_.mid->get())
		{
			// leave only mid
			if (parameters_.lowCut->get())
				midLowCutProcessor_.processBlock(midBuffer, midiMessages);

			buffer = midBuffer;
		}
		else if (parameters_.side->get())
		{
			// leave only side
			if (parameters_.lowCut->get())
				sideLowCutProcessor_.processBlock(sideBuffer, midiMessages);

			buffer = sideBuffer;
		}
		else
		{
			// filter if needed...
			if (parameters_.lowCut->get())
			{
				midLowCutProcessor_.processBlock(midBuffer, midiMessages);
				sideLowCutProcessor_.processBlock(sideBuffer, midiMessages);

				// ...add the mid & side buffers back together...
				buffer = midBuffer;
				buffer.addFrom(0, 0, sideBuffer, 0, 0, buffer.getNumSamples());
				buffer.addFrom(1, 0, sideBuffer, 1, 0, buffer.getNumSamples());
			}

			// ...or do nothing
		}
	}
	else
	{
		// non-stereo input

		// cut low frequencies evenly
		if (parameters_.lowCut->get())
			midLowCutProcessor_.processBlock(buffer, midiMessages);
	}

	// high cut is the same for mid & side
	if (parameters_.highCut->get())
		highCutProcessor_.processBlock(buffer, midiMessages);
}

//==============================================================================
bool CossackAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* CossackAudioProcessor::createEditor()
{
    return new CossackAudioProcessorEditor (*this);
}

//==============================================================================
void CossackAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void CossackAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new CossackAudioProcessor();
}

//==============================================================================
// KRIGS: Creates parameter list.
juce::AudioProcessorValueTreeState::ParameterLayout CossackAudioProcessor::createParameterLayout()
{
	// Clear the parameter pointers
	parameters_ = {};

	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	// Low/high cut
	layout.add(std::make_unique<juce::AudioParameterBool>("lowCut", "Low Cut", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("highCut", "High Cut", false));
	//layout.add(std::make_unique<juce::AudioParameterFloat>("rolloffFactor", "Rolloff Factor", juce::NormalisableRange{ 5.f, 100.f, 0.1f, 0.5f }, 48.f));

	// Equalizer
	for (int i = 0; i < 10; i++)
		layout.add(std::make_unique<juce::AudioParameterFloat>("equalizer" + std::to_string(i), "Equalizer" + std::to_string(i), juce::NormalisableRange{-12.f, 12.f, 0.1f, 1.f, false}, 0.f));

	// Mid/side
	layout.add(std::make_unique<juce::AudioParameterBool>("mid", "Mid", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("midSide", "Mid/Side", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("side", "Side", false));
	// TODO: make radio button group attachment class
	//layout.add(std::make_unique<juce::AudioParameterInt>("midSide", "Mid/Side", 1));

	// Compressors
	layout.add(std::make_unique<juce::AudioParameterFloat>("opto", "Opto", juce::NormalisableRange{ 0.f, 1.f, 0.01f }, 0.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("glue", "Glue", juce::NormalisableRange{ 0.f, 1.f, 0.01f }, 0.f));

	return layout;
}

juce::AudioProcessorValueTreeState& CossackAudioProcessor::getValueTreeState()
{
	return valueTreeState_;
}

