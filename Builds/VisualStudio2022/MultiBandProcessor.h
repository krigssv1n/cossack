#pragma once

#include <JuceHeader.h>

class MultiBandProcessor
{
public:
	float getBandSample(int band)
	{

	}

private:
	juce::dsp::LinkwitzRileyFilter<float> crossoverFilters_[10];
	float bandSamples_[11];
};

