#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//
// Splits the incoming signal into multiple bands
//
// TODO: Adjustable crossover frequencies
//

// Uncomment to do phase compensation right after the multi-band split.
// Much slower due to having triangular complexity of the filter number increase.
//
// Allows sending data of the separate bands outside.
//
// Keep commented to instead do compensation at the end, in reconstructSample(),
// reducing complexity to linear.
//#define PHASE_CORRECTION_IMMEDIATE

template<typename SampleType>
class MultiBandProcessor
{
public:
	MultiBandProcessor();
	
	void prepare(const juce::dsp::ProcessSpec &spec);

	// Split the signal into bands
	void processSample(int ch, SampleType input);
	
	// Join the bands back, applying phase compensation
	SampleType reconstructSample(int ch);
	
	void reset();

	// Get a singular band
	SampleType& getBand(int n);

	// Get a pointer to all the bands
	SampleType* getBandsPointer();

private:
	// Splitting filters
	juce::dsp::LinkwitzRileyFilter<SampleType> filtersLHP_[CossackConstants::crossoverCount];

	// Phase compensation filters
#ifdef PHASE_CORRECTION_IMMEDIATE
	// FIXME: Make number of filters correct
	juce::dsp::LinkwitzRileyFilter<SampleType> filtersAP_[CossackConstants::crossoverCount][CossackConstants::crossoverCount];
#else
	juce::dsp::LinkwitzRileyFilter<SampleType> filtersAP_[CossackConstants::crossoverCount - 1];
#endif

	SampleType bands_[CossackConstants::bandCount];
};

