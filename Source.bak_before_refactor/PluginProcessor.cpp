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
	//convolution_{ juce::dsp::Convolution::NonUniform{ 1024 } },
	valueTreeState_(*this, nullptr, juce::Identifier("CossackParameters"), createParameterLayout())
{
	//
	// Get the parameters
	//

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

	//
	// Load other data
	//

	//auto dir = juce::File::getCurrentWorkingDirectory();

	//int numTries = 0;

	//while (!dir.getChildFile("Resources").exists() && numTries++ < 15)
	//	dir = dir.getParentDirectory();

	//convolution_.loadImpulseResponse(dir.getChildFile("Resources").getChildFile("guitar_amp.wav"),
	convolution_.loadImpulseResponse(juce::File("F:/Media/Audio/Samples/Impulse Responses/guitar_amp.wav"),
	//convolution_.loadImpulseResponse(juce::File("F:/Media/Audio/Samples/Impulse Responses/cassette_recorder.wav"),
		juce::dsp::Convolution::Stereo::yes,
		juce::dsp::Convolution::Trim::no,
		1024);
		//juce::dsp::Convolution::Normalise::no);
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

	// KRIGS: From JUCE docs.
	// This method will return the total number of input channels by accumulating the number of channels on each input bus.
	// The number of channels of the buffer passed to your processBlock callback will be equivalent
	// to either getTotalNumInputChannels or getTotalNumOutputChannels - which ever is greater.
	auto channelCount = static_cast<juce::uint32> (juce::jmax(getTotalNumInputChannels(), getTotalNumOutputChannels()));
	juce::dsp::ProcessSpec spec{ sampleRate_, static_cast<juce::uint32> (samplesPerBlock), channelCount };

	// Mono spec for the mid equalizer or mono input
	equalizerProcessors_[0].prepare(spec);

	// Stereo spec for the side equalizer
	equalizerProcessors_[1].prepare(spec);

	// IR convolution for saturation/distortion
	convolution_.prepare(spec);
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

// TEST: Harmonic enhancement
static float invSqrtFast(float x)
{
	const float xHalf = 0.5f * x;
	int i = *(int*)&x;
	i = 0x5f3759df - (i >> 1);
	x = *(float*)&i;
	x = x * (1.5f - xHalf * x * x);

	return x;
}

static float atanFast(float x)
{
	// TODO: Look for other approximations
	return x / (1.f + 0.28f * x * x);
}

template <typename T> int sgn(T val) {
	return (T(0) < val) - (val < T(0));
}

float CossackAudioProcessor::testHarmonics(float x)
{
	// Saturation coefficient
	const float k = 1.f + 7.f * parameters_.glue->get();

	// Waveshaper type
	const int type = 0;
	
	// Sigmoids only for now
	switch (type)
	{
	case 0:
	{
		// logistic function
		// vacuum tubes emulation

		//return (2.f / (1.f + expf(-k * x)) - 1.f) / (2.f / (1.f + expf(-k)) - 1.f);
		//return (2.f / (1.f + juce::dsp::FastMathApproximations::exp(-k * x)) - 1.f) / (2.f / (1.f + juce::dsp::FastMathApproximations::exp(-k)) - 1.f);

		const auto f = [](float f) { return 2.f / (1.f + expf(-f)) - 1.f; };
		//const auto f = [](float f) { return 2.f / (1.f + juce::dsp::FastMathApproximations::exp(-f)) - 1.f; };

		return f(k * x) / f(k);
	}
	case 1:
	{
		// atan
		// vacuum tubes emulation
		return atanf(k * x) / atanf(k);
		//return atanFast(k * x) / atanFast(k);
	}
	case 2:
	{
		// tanh
		// differential transistor pairs emulation
		return tanhf(k * x) / tanh(k);
		//return juce::dsp::FastMathApproximations::tanh(k * x) / juce::dsp::FastMathApproximations::tanh(k);
	}
	default:
		break;
	}

/*
	// sin
	const float piOverInvK = juce::MathConstants<float>::pi / (9.f - k);
	return sinf(piOverInvK * x) / sinf(piOverInvK);
	//return juce::dsp::FastMathApproximations::sin(piOverInvK * x) / juce::dsp::FastMathApproximations::sin(piOverInvK);
*/

/*
	{
		// !
		// https://www.musicdsp.org/en/latest/Effects/89-2-wave-shaping-things.html
		// TODO: autogain, 1 more parameter slider

		float y;
		float A = 1.f;
		float B = k;

		if (x < 0)
			y = -A * tanhf(B * x) / B;
		else
			y = x;

		if (y >= 0)
			y = -A * tanh(B * y) / B;

		return y;
	}
*/

	// !
	// error function
	// soft clipping, simulation of an oscilloscope
	// TODO: autogain
	//return erff(k * x) / k;

	// ?
	// https://www.musicdsp.org/en/latest/Effects/43-waveshaper.html
	//const float a = 0.01f + k / 4.f;

	//float z = juce::MathConstants<float>::pi * a;
	//float s = 1.f / sinf(z);
	//float b = 1.f / a;

	//if (x > b)
	//	return 1.f;
	//else
	//	return sinf(z * x) * s;

	// ?
	//// https://www.musicdsp.org/en/latest/Effects/42-soft-saturation.html
	//// f(x)'=f(x)*(1/((a+1)/2))
	//const float a = parameters_.glue->get();
	//float r = 0.f;

	//if (x < a)
	//	r = x;
	//else if (x > a) {
	//	const float b = (x - a) / (1 - a);
	//	r = a + (x - a) / (1.f + b * b);
	//}
	//else if (x > 1.f)
	//	r = (a + 1.f) / 2.f;

	//return r;// / (a + 1.f) * 2.f;

	// ?
	// https://www.musicdsp.org/en/latest/Effects/41-waveshaper.html	
	//return x * (fabsf(x) + k) / (x * x + (k - 1.f) * fabsf(x) + 1.f);

	// ?
	// https://www.musicdsp.org/en/latest/Effects/46-waveshaper.html
	// a should be in [-1; 1)
	//const float a = -1.f + 1.99f * (k - 1.f) / 19.f;
	//const float b = 2.f * a / (1.f - a);
	//return (1.f + b) * x / (1.f + b * fabsf(x));

	// ?
	// https://www.musicdsp.org/en/latest/Effects/104-variable-hardness-clipping-function.html
	//return sgn(x) * powf(atanf(powf(fabsf(x), k)), (1 / k));
	//return atanf(k * x) / atan(k);// *(2.f / juce::MathConstants<float>::pi);

	// ?
	// simple polynomial
	//return 1.5f * x - 0.5f * x * x * x;

	// ?
	// soft clipping
	// fairly similar to tanh, but not quite as sharp, thus producing slightly more distortion at low-to-moderate input levels
	//return x / sqrtf(1.f + x * x);
	//return x * invSqrtFast(1.f + x * x);

	// ?
	// soft clipping
	//return sample / (1.f + fabs(sample));

	return 0.f;
}

void CossackAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

	// KRIGS: Same for everything
	const int sampleCount = buffer.getNumSamples();

	// In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, sampleCount);

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.

	// KRIGS: Update parameters & their dependencies
	updateParameters();

	//
	// Perform the processing.
	//

	const float harmonicsDrive = parameters_.opto->get();

	// Plugin settings should disallow number of input channels higher than 2.
	if (totalNumInputChannels == 2)
	{
		// Stereo input.
		// Do the mid/side split.
		// Use values from separate mid/side controls for filtering.

		// TODO: Optimize the order of actions
		float* samples[] = { buffer.getWritePointer(0), buffer.getWritePointer(1) };

		// Construct the buffers
		//juce::AudioBuffer<float> midBuffer(1, sampleCount);
		//float* midSamples = { midBuffer.getWritePointer(0) };
		// Make it double channel for convenience
		juce::AudioBuffer<float> midBuffer(2, sampleCount);
		float* midSamples[] = { midBuffer.getWritePointer(0), midBuffer.getWritePointer(1) };

		juce::AudioBuffer<float> sideBuffer(2, sampleCount);
		float* sideSamples[] = { sideBuffer.getWritePointer(0), sideBuffer.getWritePointer(1) };

		//
		// Mid/side split & stereo base expansion.
		//

		constexpr float stereoWidth = 1.2f;
		constexpr float scale = 1.f / juce::jmax(1.f + stereoWidth, 2.f);

		for (auto i = 0; i < sampleCount; i++)
		{
			//midSamples[i] = (samples[0][i] + samples[1][i]) * scale;
			midSamples[0][i] = midSamples[1][i] = (samples[0][i] + samples[1][i]) * scale;

			//sideSamples[0][i] = (samples[0][i] - samples[1][i]) * scale * stereoWidth;

			//sideSamples[0][i] = samples[0][i] * stereoWidth - midSamples[i];
			//sideSamples[1][i] = samples[1][i] * stereoWidth - midSamples[i];
			sideSamples[0][i] = (samples[0][i] - samples[1][i]) * scale * stereoWidth;
			sideSamples[1][i] = (samples[1][i] - samples[0][i]) * scale * stereoWidth;
		}

		// Bitfield for ease of usage
		const char midSide = parameters_.mid->get() ? 1 : (parameters_.side->get() ? 2 : 3);

		//
		// Low cut, always comes first
		//

		if (parameters_.lowCut->get())
		{
			if (midSide & 1)
				lowCutProcessor_[0].processBlock(midBuffer, midiMessages);

			if (midSide & 2)
				lowCutProcessor_[1].processBlock(sideBuffer, midiMessages);
		}

		//
		// Equalization, harmonics & buffer restoration
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
				// Mid harmonics
				if (parameters_.harmonicsMid[0]->get()) {
					//juce::AudioBuffer<float> harmonicsBuffer(1, sampleCount);
					//harmonicsBuffer.copyFrom(0, 0, midBuffer, 0, 0, sampleCount);

					//juce::dsp::AudioBlock<float> block(harmonicsBuffer);
					//juce::dsp::ProcessContextReplacing<float> context(block);
					//convolution_.process(context);

					//// Mix according to the drive value
					//const float* harmonicsSamples{ harmonicsBuffer.getReadPointer(0) };

					for (int i = 0; i < sampleCount; i++) {
						//midSamples[0][i] = midSamples[1][i] = juce::jmap(harmonicsDrive, midSamples[0][i], harmonicsSamples[i]);
						midSamples[0][i] = midSamples[1][i] = juce::jmap(harmonicsDrive, midSamples[0][i], testHarmonics(midSamples[0][i]));
					}
				}
			}

			{
				// Side equalization
				juce::dsp::AudioBlock<float> block(sideBuffer);
				juce::dsp::ProcessContextReplacing<float> context(block);
				equalizerProcessors_[1].process(context);
			}

			{
				// Side harmonics
				if (parameters_.harmonicsMid[0]->get()) {
					//juce::AudioBuffer<float> harmonicsBuffer(2, sampleCount);
					//harmonicsBuffer.copyFrom(0, 0, sideBuffer, 0, 0, sampleCount);
					//harmonicsBuffer.copyFrom(1, 0, sideBuffer, 1, 0, sampleCount);

					//juce::dsp::AudioBlock<float> block(harmonicsBuffer);
					//juce::dsp::ProcessContextReplacing<float> context(block);
					//convolution_.process(context);

					//// Mix according to the drive value
					//const float* harmonicsSamples[] { harmonicsBuffer.getReadPointer(0), harmonicsBuffer.getReadPointer(1) };

					for (int i = 0; i < sampleCount; i++) {
						for (int j = 0; j < 2; j++) {
							//sideSamples[j][i] = juce::jmap(harmonicsDrive, sideSamples[j][i], harmonicsSamples[j][i]);
							sideSamples[j][i] = juce::jmap(harmonicsDrive, sideSamples[j][i], testHarmonics(sideSamples[j][i]));
						}
					}
				}
			}

			// Concatenate the mid & side buffers back together
			for (auto i = 0; i < sampleCount; i++)
			{
				// old way, before the decision was made to leave the side part stereo-split
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
				{
					// Mid equalization
					juce::dsp::AudioBlock<float> block(midBuffer);
					juce::dsp::ProcessContextReplacing<float> context(block);
					equalizerProcessors_[0].process(context);
				}

				{
					// Mid harmonics
					if (parameters_.harmonicsMid[0]->get()) {
						//juce::AudioBuffer<float> harmonicsBuffer(1, sampleCount);
						//harmonicsBuffer.copyFrom(0, 0, midBuffer, 0, 0, sampleCount);

						//juce::dsp::AudioBlock<float> block(harmonicsBuffer);
						//juce::dsp::ProcessContextReplacing<float> context(block);
						//convolution_.process(context);

						//// Mix according to the drive value
						//const float* harmonicsSamples{ harmonicsBuffer.getReadPointer(0) };

						for (int i = 0; i < sampleCount; i++) {
							//midSamples[0][i] = midSamples[1][i] = juce::jmap(harmonicsDrive, midSamples[0][i], harmonicsSamples[i]);
							midSamples[0][i] = midSamples[1][i] = juce::jmap(harmonicsDrive, midSamples[0][i], testHarmonics(midSamples[0][i]));
						}
					}
				}

				// Leave only the mid buffer, ditching the side
				//buffer.copyFrom(0, 0, midBuffer, 0, 0, sampleCount);
				//buffer.copyFrom(1, 0, midBuffer, 0, 0, sampleCount);
				buffer.copyFrom(0, 0, midBuffer, 0, 0, sampleCount);
				buffer.copyFrom(1, 0, midBuffer, 1, 0, sampleCount);
			}
			else if (midSide & 2)
			{
				{
					// Side equalization
					juce::dsp::AudioBlock<float> block(sideBuffer);
					juce::dsp::ProcessContextReplacing<float> context(block);
					equalizerProcessors_[1].process(context);
				}

				{
					// Side harmonics
					if (parameters_.harmonicsMid[0]->get()) {
						//juce::AudioBuffer<float> harmonicsBuffer(2, sampleCount);
						//harmonicsBuffer.copyFrom(0, 0, sideBuffer, 0, 0, sampleCount);
						//harmonicsBuffer.copyFrom(1, 0, sideBuffer, 1, 0, sampleCount);

						//juce::dsp::AudioBlock<float> block(harmonicsBuffer);
						//juce::dsp::ProcessContextReplacing<float> context(block);
						//convolution_.process(context);

						//// Mix according to the drive value
						//const float* harmonicsSamples[] { harmonicsBuffer.getReadPointer(0), harmonicsBuffer.getReadPointer(1) };

						for (int i = 0; i < sampleCount; i++) {
							for (int j = 0; j < 2; j++) {
								//sideSamples[j][i] = juce::jmap(harmonicsDrive, sideSamples[j][i], harmonicsSamples[j][i]);
								sideSamples[j][i] = juce::jmap(harmonicsDrive, sideSamples[j][i], testHarmonics(sideSamples[j][i]));
							}
						}
					}
				}

				// Leave only the side buffer, ditching the mid
				buffer.copyFrom(0, 0, sideBuffer, 0, 0, sampleCount);
				buffer.copyFrom(1, 0, sideBuffer, 1, 0, sampleCount);
			}
		}
	}
	else
	{
		// Mono input.
		// Don't do mid/side split.
		// Use values from mid-related controls for filtering.
		float* samples = buffer.getWritePointer(0);

		//
		// Low cut, always comes first
		//
		
		if (parameters_.lowCut->get())
			lowCutProcessor_[0].processBlock(buffer, midiMessages);


		//
		// Equalization
		//
		
		{
			juce::dsp::AudioBlock<float> block(buffer);
			juce::dsp::ProcessContextReplacing<float> context(block);
			equalizerProcessors_[0].process(context);
		}

		//
		// Harmonics
		//

		if (parameters_.harmonicsMid[0]->get()) {
			//juce::AudioBuffer<float> harmonicsBuffer(1, sampleCount);
			//harmonicsBuffer.copyFrom(0, 0, buffer, 0, 0, sampleCount);

			//juce::dsp::AudioBlock<float> block(harmonicsBuffer);
			//juce::dsp::ProcessContextReplacing<float> context(block);
			//convolution_.process(context);

			//// Mix according to the drive value
			//const float* harmonicsSamples = harmonicsBuffer.getReadPointer(0);

			for (int i = 0; i < sampleCount; i++) {
				//samples[i] = juce::jmap(harmonicsDrive, samples[i], harmonicsSamples[i]);
				samples[i] = juce::jmap(harmonicsDrive, samples[i], testHarmonics(samples[i]));
			}
		}
	}

	// High cut, always finishes the chain and is the same for mid & side.
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
		*equalizerProcessors_[i].get<0>().state = *Coefficients::makeLowShelf(sampleRate_, centralFrequencies[0], inverseSqrt2, juce::Decibels::decibelsToGain(parameters_.equalizers[i][0]->get()));
		*equalizerProcessors_[i].get<1>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[1], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][1]->get()));
		*equalizerProcessors_[i].get<2>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[2], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][2]->get()));
		*equalizerProcessors_[i].get<3>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[3], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][3]->get()));
		*equalizerProcessors_[i].get<4>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[4], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][4]->get()));
		*equalizerProcessors_[i].get<5>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[5], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][5]->get()));
		*equalizerProcessors_[i].get<6>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[6], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][6]->get()));
		*equalizerProcessors_[i].get<7>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[7], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][7]->get()));
		*equalizerProcessors_[i].get<8>().state = *Coefficients::makePeakFilter(sampleRate_, centralFrequencies[8], Q, juce::Decibels::decibelsToGain(parameters_.equalizers[i][8]->get()));
		*equalizerProcessors_[i].get<9>().state = *Coefficients::makeHighShelf(sampleRate_, centralFrequencies[9], inverseSqrt2, juce::Decibels::decibelsToGain(parameters_.equalizers[i][9]->get()));
	}
}
