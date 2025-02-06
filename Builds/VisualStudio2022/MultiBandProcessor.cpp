#include "MultiBandProcessor.h"

MultiBandProcessor::MultiBandProcessor()
{
	// Initialize the crossover filters
	for (int i = 0; i < 10; i++)
	{
		crossoverFilters_[i].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
		crossoverFilters_[i].setCutoffFrequency(crossoverFrequencies[i]);
	}
}