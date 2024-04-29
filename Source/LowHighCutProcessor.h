/*
  ==============================================================================

    LowHighCutProcessor.h
    Created: 23 Feb 2024 7:30:14pm
    Author:  KOT

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <vector>
#include "ProcessorBase.h"

class LowHighCutProcessor : public ProcessorBase
{
public:
	LowHighCutProcessor(float cutoffFrequency = 2000.f, int order = 8, bool isHighCut = false);
	~LowHighCutProcessor() override;

	void prepareToPlay(double sampleRate, int samplesPerBlock) override;

	void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

	void reset() override;

	const juce::String getName() const override;

	void setCutoffFrequency(float);
	void setIsHighCut(bool);

private:
	using ProcessorType = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;

	std::vector<std::shared_ptr<ProcessorType>> processors_;

	float cutoffFrequency_;
	int order_;
	bool isHighCut_;
	bool hasChanged_;

	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LowHighCutProcessor)
};
