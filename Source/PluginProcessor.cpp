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
	AudioProcessor(BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
		.withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
		.withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
	),
#else
#endif
	parameters_{ 0 },
	lowCutProcessor_{
		{ 30.f, 8 },
		{ 100.f, 2 }
	},
	highCutProcessor_{ 20000.f, 8, true },
	valueTreeState_(*this, nullptr, juce::Identifier("CossackParameters"), createParameterLayout())
{
	// Low/high cut
	parameters_.lowCut = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("lowCut"));
	parameters_.highCut = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("highCut"));

	// Mid/side
	parameters_.mid = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("mid"));
	parameters_.midSide = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("midSide"));
	parameters_.side = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("side"));

	for (int i = 0; i < 10; i++)
	{
		// Equalizer
		for (int j = 0; j < 2; j++)
		{
			const juce::String index = std::to_string(j) + "_" + std::to_string(i);
			parameters_.equalizers[j][i] = static_cast<juce::AudioParameterFloat*>(valueTreeState_.getParameter("equalizer" + index));
			valueTreeState_.addParameterListener(parameters_.equalizers[j][i]->getParameterID(), this);
		}

		// Harmonics
		parameters_.harmonicsMid[i] = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("harmonicsMid" + std::to_string(i)));

		if (i >= 2)
			parameters_.harmonicsSide[i - 2] = static_cast<juce::AudioParameterBool*>(valueTreeState_.getParameter("harmonicsSide" + std::to_string(i)));
	}

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
	// Save this for later
	sampleRate_ = sampleRate;

	// Use this method as the place to do any pre-playback
    // initialisation that you need..
	lowCutProcessor_[0].prepareToPlay(sampleRate_, samplesPerBlock);
	lowCutProcessor_[1].prepareToPlay(sampleRate_, samplesPerBlock);
	highCutProcessor_.prepareToPlay(sampleRate_, samplesPerBlock);

	// Mono spec for the mid equalizer or mono input
	//equalizerProcessors_[0].prepare({ sampleRate_, static_cast<juce::uint32> (samplesPerBlock), 1 });
	equalizerProcessors_[0].prepare({ sampleRate_, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (juce::jmin(getTotalNumInputChannels(), getTotalNumOutputChannels())) });

	// Stereo spec for the side equalizer
	equalizerProcessors_[1].prepare({ sampleRate_, static_cast<juce::uint32> (samplesPerBlock), static_cast<juce::uint32> (juce::jmin(getTotalNumInputChannels(), getTotalNumOutputChannels())) });
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

	// KRIGS: Update parameters & their dependencies
	updateParameters();

	if (totalNumInputChannels == 2)
	{
		// Stereo input.
		// Do the mid/side processing.
		// Use separate values for filtering where necessary.

		// TODO: Optimize the order of actions
		float* samples[] = { buffer.getWritePointer(0), buffer.getWritePointer(1) };

		// Construct the buffers
		//juce::AudioBuffer<float> midBuffer(1, buffer.getNumSamples());
		//float* midSamples = { midBuffer.getWritePointer(0) };
		juce::AudioBuffer<float> midBuffer(2, buffer.getNumSamples());
		float* midSamples[] = { midBuffer.getWritePointer(0), midBuffer.getWritePointer(1) };

		juce::AudioBuffer<float> sideBuffer(2, buffer.getNumSamples());
		float* sideSamples[] = { sideBuffer.getWritePointer(0), sideBuffer.getWritePointer(1) };

		//
		// Split into mid & side.
		// Expand the stereo base.
		//

		constexpr float stereoWidth = 1.2f;
		constexpr float scale = 1.f / juce::jmax(1.f + stereoWidth, 2.f);

		for (auto i = 0; i < buffer.getNumSamples(); i++)
		{
			//midSamples[i] = (samples[0][i] + samples[1][i]) * scale;
			midSamples[0][i] = midSamples[1][i] = (samples[0][i] + samples[1][i]) * scale;

			//sideSamples[0][i] = (samples[0][i] - samples[1][i]) * scale * stereoWidth;

			//sideSamples[0][i] = samples[0][i] * stereoWidth - midSamples[i];
			//sideSamples[1][i] = samples[1][i] * stereoWidth - midSamples[i];
			sideSamples[0][i] = (samples[0][i] - samples[1][i]) * scale * stereoWidth;
			sideSamples[1][i] = (samples[1][i] - samples[0][i]) * scale * stereoWidth;
		}

		// Bitfield to ease parameter usage
		const char midSide = parameters_.mid->get() ? 1 : (parameters_.side->get() ? 2 : 3);

		//
		// Low cut
		//

		if (parameters_.lowCut->get())
		{
			if (midSide & 1)
				lowCutProcessor_[0].processBlock(midBuffer, midiMessages);

			if (midSide & 2)
				lowCutProcessor_[1].processBlock(sideBuffer, midiMessages);
		}

		//
		// Equalizer & buffer restoration
		//
		
		if ((midSide & 3) == 3)
		{
			{
				// Mid equalization
				juce::dsp::AudioBlock<float> block(midBuffer);
				juce::dsp::ProcessContextReplacing<float> context(block);

				equalizerProcessors_[0].process(context);
			}

			{
				// Side equalization
				juce::dsp::AudioBlock<float> block(sideBuffer);
				juce::dsp::ProcessContextReplacing<float> context(block);

				equalizerProcessors_[1].process(context);
			}

			// Concatenate the mid & side back together
			for (auto i = 0; i < buffer.getNumSamples(); i++)
			{
				// old way, before the decision to leave the side part stereo-split was made
				//samples[0][i] = midSamples[i] + sideSamples[i];
				//samples[1][i] = midSamples[i] - sideSamples[i];

				// new way
				//samples[0][i] = midSamples[i] + sideSamples[0][i];
				//samples[1][i] = midSamples[i] + sideSamples[1][i];
				samples[0][i] = midSamples[0][i] + sideSamples[0][i];
				samples[1][i] = midSamples[1][i] + sideSamples[1][i];
			}
		}
		else
		{
			if (midSide & 1)
			{
				// Mid equalization
				juce::dsp::AudioBlock<float> block(midBuffer);
				juce::dsp::ProcessContextReplacing<float> context(block);

				equalizerProcessors_[0].process(context);

				// Leave only the mid buffer
				//buffer.copyFrom(0, 0, midBuffer, 0, 0, buffer.getNumSamples());
				//buffer.copyFrom(1, 0, midBuffer, 0, 0, buffer.getNumSamples());
				buffer.copyFrom(0, 0, midBuffer, 0, 0, buffer.getNumSamples());
				buffer.copyFrom(1, 0, midBuffer, 1, 0, buffer.getNumSamples());
			}
			else if (midSide & 2)
			{
				// Side equalization
				juce::dsp::AudioBlock<float> block(sideBuffer);
				juce::dsp::ProcessContextReplacing<float> context(block);

				equalizerProcessors_[1].process(context);

				// Leave only the side buffer
				buffer.copyFrom(0, 0, sideBuffer, 0, 0, buffer.getNumSamples());
				buffer.copyFrom(1, 0, sideBuffer, 1, 0, buffer.getNumSamples());
			}
		}
	}
	else
	{
		// Mono input.
		// Don't do mid/side processing.
		// Use mid values for filtering.

		//
		// Low cut
		//
		
		if (parameters_.lowCut->get())
			lowCutProcessor_[0].processBlock(buffer, midiMessages);

		//
		// Equalizer
		//
		
		juce::dsp::AudioBlock<float> block(buffer);
		juce::dsp::ProcessContextReplacing<float> context(block);

		equalizerProcessors_[0].process(context);
	}

	// High cut is the same for mid & side
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
// KRIGS: Parameter management.
//==============================================================================

juce::AudioProcessorValueTreeState& CossackAudioProcessor::getValueTreeState()
{
	return valueTreeState_;
}

juce::AudioProcessorValueTreeState::ParameterLayout CossackAudioProcessor::createParameterLayout()
{
	juce::AudioProcessorValueTreeState::ParameterLayout layout;

	// Low/high cut
	layout.add(std::make_unique<juce::AudioParameterBool>("lowCut", "Low Cut", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("highCut", "High Cut", false));
	//layout.add(std::make_unique<juce::AudioParameterFloat>("rolloffFactor", "Rolloff Factor", juce::NormalisableRange{ 5.f, 100.f, 0.1f, 0.5f }, 48.f));

	for (int i = 0; i < 10; i++)
	{
		// Equalizer
		for (int j = 0; j < 2; j++)
		{
			const juce::String index = std::to_string(j) + "_" + std::to_string(i);
			layout.add(std::make_unique<juce::AudioParameterFloat>("equalizer" + index, "Equalizer" + index, juce::NormalisableRange{ -12.f, 12.f, 0.1f, 1.f, false }, 0.f));
		}

		// Harmonics
		layout.add(std::make_unique<juce::AudioParameterBool>("harmonicsMid" + std::to_string(i), "Harmonics Mid" + std::to_string(i), false));

		if (i >= 2)
			layout.add(std::make_unique<juce::AudioParameterBool>("harmonicsSide" + std::to_string(i), "Harmonics Side" + std::to_string(i), false));
	}

	// Mid/side
	layout.add(std::make_unique<juce::AudioParameterBool>("mid", "Mid", false));
	layout.add(std::make_unique<juce::AudioParameterBool>("midSide", "Mid/Side", true));
	layout.add(std::make_unique<juce::AudioParameterBool>("side", "Side", false));
	// TODO: Make radio button group attachment class.
	//layout.add(std::make_unique<juce::AudioParameterInt>("midSide", "Mid/Side", 1));

	// Compressors
	layout.add(std::make_unique<juce::AudioParameterFloat>("opto", "Opto", juce::NormalisableRange{ 0.f, 1.f, 0.01f }, 0.f));
	layout.add(std::make_unique<juce::AudioParameterFloat>("glue", "Glue", juce::NormalisableRange{ 0.f, 1.f, 0.01f }, 0.f));

	return layout;
}

void CossackAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{

}

void CossackAudioProcessor::updateParameters()
{
	const float Q = 2.f;

	for (int i = 0; i < 2; i++)
	{
		*equalizerProcessors_[i].get<0>().state = *Coefficients::makeLowShelf(sampleRate_, frequencyBands[0], inverseRootTwo, juce::Decibels::decibelsToGain(parameters_.equalizers[i][0]->get()));
		*equalizerProcessors_[i].get<1>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[1], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][1]->get()));
		*equalizerProcessors_[i].get<2>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[2], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][2]->get()));
		*equalizerProcessors_[i].get<3>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[3], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][3]->get()));
		*equalizerProcessors_[i].get<4>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[4], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][4]->get()));
		*equalizerProcessors_[i].get<5>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[5], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][5]->get()));
		*equalizerProcessors_[i].get<6>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[6], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][6]->get()));
		*equalizerProcessors_[i].get<7>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[7], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][7]->get()));
		*equalizerProcessors_[i].get<8>().state = *Coefficients::makePeakFilter(sampleRate_, frequencyBands[8], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][8]->get()));
		*equalizerProcessors_[i].get<9>().state = *Coefficients::makeHighShelf(sampleRate_, frequencyBands[9], inverseRootTwo, juce::Decibels::decibelsToGain(parameters_.equalizers[i][9]->get()));
	}
}
