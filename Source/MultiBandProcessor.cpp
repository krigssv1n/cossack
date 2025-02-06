#include "PluginProcessor.h"

template class MultiBandProcessor<float>;
template class MultiBandProcessor<double>;

template<typename SampleType>
MultiBandProcessor<SampleType>::MultiBandProcessor()
{
}

template<typename SampleType>
void MultiBandProcessor<SampleType>::prepare(const juce::dsp::ProcessSpec& spec)
{
	// Prepare the crossover filters
	for (int i = 0; i < 10; i++)
	{
		//filters_[i].setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
		filters_[i].setCutoffFrequency(static_cast<SampleType>(CossackAudioProcessor::crossoverFrequencies[i]));
		filters_[i].prepare(spec);
	}
}

template<typename SampleType>
void MultiBandProcessor<SampleType>::processSample(int ch, SampleType input)
{
	bands_[0] = input;

	for (int i = 0; i < 10; i++)
	{
		filters_[i].processSample(ch, bands_[i], bands_[i], bands_[i + 1]);
	}
}

template<typename SampleType>
void MultiBandProcessor<SampleType>::reset()
{
	for (int i = 0; i < 10; i++)
		filters_->reset();
}

template<typename SampleType>
SampleType MultiBandProcessor<SampleType>::getBand(int band)
{
	return bands_[band];
}
