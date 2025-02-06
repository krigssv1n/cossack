#pragma once

#include <JuceHeader.h>

//
// Splits the incoming signal into multiple bands
//
// TODO: Adjustable crossover frequencies
//
template<typename SampleType>
class MultiBandProcessor
{
public:
	MultiBandProcessor();
	
	void prepare(const juce::dsp::ProcessSpec &spec);

	// Split the signal into bands
	void processSample(int ch, SampleType input);
	
	void reset();

	// Get a singular band, resulting from processing
	SampleType getBand(int band);

private:
	juce::dsp::LinkwitzRileyFilter<SampleType> filters_[10];
	SampleType bands_[11];
};

