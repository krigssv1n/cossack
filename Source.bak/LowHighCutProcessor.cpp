/*
  ==============================================================================

    LowHighCutProcessor.cpp
    Created: 23 Feb 2024 7:30:14pm
    Author:  KOT

  ==============================================================================
*/

#include "LowHighCutProcessor.h"

LowHighCutProcessor::LowHighCutProcessor(float cutoffFrequency, int order, bool isHighCut) :
	cutoffFrequency_(cutoffFrequency),
	order_(order),
	isHighCut_(isHighCut),
	hasChanged_(true)
{
	jassert(cutoffFrequency_ >= 0);
	jassert(order >= 0);
}

LowHighCutProcessor::~LowHighCutProcessor()
{
}

void LowHighCutProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
	if (hasChanged_)
	{
		// Rebuild processor list
		processors_.clear();

		int nl = juce::jmin(getTotalNumInputChannels(), getTotalNumOutputChannels());

		// FIXME: Figure out if we need a setting for mono buffers to save performance
		juce::dsp::ProcessSpec spec
		{
			sampleRate,
			static_cast<juce::uint32> (samplesPerBlock),
			static_cast<juce::uint32> (juce::jmin(getTotalNumInputChannels(), getTotalNumOutputChannels()))
		};

		//pd->state = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, cutoffFrequency_);
		//pd->state = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, cutoffFrequency_);

		// KRIGS: (6 * order) dB/oct rolloff slope
		auto filters = isHighCut_ ?
			juce::dsp::FilterDesign<float>::designIIRLowpassHighOrderButterworthMethod(cutoffFrequency_, sampleRate, order_) :
			juce::dsp::FilterDesign<float>::designIIRHighpassHighOrderButterworthMethod(cutoffFrequency_, sampleRate, order_);

		for (auto& filter : filters)
		{
			auto pd = std::make_shared<Duplicator>(filter);
			pd->prepare(spec);

			processors_.push_back(pd);
		}

		hasChanged_ = false;
	}
}

void LowHighCutProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiBuffer)
{
	if (processors_.empty())
		return;

	juce::dsp::AudioBlock<float> block(buffer);
	juce::dsp::ProcessContextReplacing<float> context(block);

	for (auto& pd : processors_)
		pd->process(context);

	/*
	// KRIGS: simple low/high pass filter for test, 6 & 12 dB/oct
	oneSampleBuffer_.resize(buffer.getNumChannels(), 0.f);

	const auto tan = tanf(juce::MathConstants<float>::pi * cutoffFrequency_ / sampleRate_);
	const auto a1 = (tan - 1.f) / (tan + 1.f);
	const auto sign = isHighCut_ ? 1.f : -1.f;

	for (auto channel = 0; channel < buffer.getNumChannels(); channel++)
	{
		auto channelSamples = buffer.getWritePointer(channel);

		for (auto i = 0; i < buffer.getNumSamples(); i++)
		{
			const auto inputSample = channelSamples[i];

			const auto allPassFilteredSample = a1 * inputSample + oneSampleBuffer_[channel];
			oneSampleBuffer_[channel] = inputSample - a1 * allPassFilteredSample;

			channelSamples[i] = 0.5f * (inputSample + sign * allPassFilteredSample);
		}
	}
	*/
}

void LowHighCutProcessor::reset()
{
	for (auto& pd : processors_)
		pd->reset();
}

const juce::String LowHighCutProcessor::getName() const
{
	return "Low/High Cut";
}

void LowHighCutProcessor::setCutoffFrequency(float f)
{
	cutoffFrequency_ = f;
	hasChanged_ = true;
}

void LowHighCutProcessor::setIsHighCut(bool b)
{
	isHighCut_ = b;
	hasChanged_ = true;
}
