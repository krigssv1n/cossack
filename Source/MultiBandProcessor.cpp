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
	for (int i = 0; i < CossackConstants::crossoverCount; i++)
	{
		filtersLHP_[i].setCutoffFrequency(static_cast<SampleType>(CossackConstants::crossoverFrequencies[i]));
		filtersLHP_[i].prepare(spec);

#ifdef PHASE_CORRECTION_IMMEDIATE
		// FIXME: Make number of filters correct
		for (int j = 0; j < CossackConstants::crossoverCount; j++)
		{
			filtersAP_[i][j].setType(juce::dsp::LinkwitzRileyFilterType::allpass);
			filtersAP_[i][j].setCutoffFrequency(static_cast<SampleType>(CossackConstants::crossoverFrequencies[i]));
			filtersAP_[i][j].prepare(spec);
		}
#else
		if (i > 0)
		{
			filtersAP_[i - 1].setType(juce::dsp::LinkwitzRileyFilterType::allpass);
			filtersAP_[i - 1].setCutoffFrequency(static_cast<SampleType>(CossackConstants::crossoverFrequencies[i]));
			filtersAP_[i - 1].prepare(spec);
		}
#endif
	}
}

template<typename SampleType>
void MultiBandProcessor<SampleType>::processSample(int ch, SampleType input)
{
	//SampleType lp[CossackConstants::crossoverCount];
	//SampleType hp[CossackConstants::crossoverCount];

	bands_[0] = input;

	// Go over the crossover frequencies.
	// TODO: Each time, split off the high end of the unsplit signal.
	// This way the lowest band gets the highest order filtering, which plays well with our hearing.
	for (int i = 0; i < CossackConstants::crossoverCount; i++)
	{
		// Split the current band
		filtersLHP_[i].processSample(ch, bands_[i], bands_[i], bands_[i + 1]);
		//filtersLHP_[i].processSample(ch, input, lp[i], hp[i]);
	}

	//bands_[0] = lp[0] + hp[0];

#ifdef PHASE_CORRECTION_IMMEDIATE
	for (int i = 0; i < CossackConstants::crossoverCount - 1; i++)
	{
		for (int j = i + 1; j < CossackConstants::crossoverCount; j++)
			bands_[i] = filtersAP_[j][i].processSample(ch, bands_[i]);
	}
#endif

/*
	return;

	//bands_[0] = filtersLP_[0].processSample(ch, bands_[0]);
	//bands_[0] = filtersAP_[0].processSample(ch, bands_[0]);

	SampleType lp, hp;

#if 1
	//
	// crossover 0
	//

	// split
	filtersLP_[0].processSample(ch, bands_[0], bands_[0], bands_[1]);

	//
	// crossover 1
	//

	// split
	filtersLP_[1].processSample(ch, bands_[1], bands_[1], bands_[2]);

	// compensate the low band's phase shift
	bands_[0] = filtersAP_[1].processSample(ch, bands_[0]);
#else
	filtersLP_[0].processSample(ch, input, bands_[0], hp);

	filtersLP_[1].processSample(ch, input, lp, bands_[2]);

	bands_[1] = input - bands_[0] - bands_[2];
#endif
*/
}

template<typename SampleType>
SampleType MultiBandProcessor<SampleType>::reconstructSample(int ch)
{
	int i;
	SampleType sum;

#ifdef PHASE_CORRECTION_IMMEDIATE
	sum = 0.0;

	for (i = 0; i < CossackConstants::bandCount; i++)
		sum += bands_[i];

#else
	// Formula is as follows:
	// sum = b9 + b8 + ap8(b7 + ap7(b6 + ap6(b5 + ap5(b4 + ap4(b3 + ap3(b2 + ap2(b1 + ap1(b0))))))))
	sum = bands_[0];

	for (i = 1; i < CossackConstants::crossoverCount; i++)
		sum = bands_[i] + filtersAP_[i - 1].processSample(ch, sum);

	// Add the highest band (no compensation required)
	sum += bands_[i];
#endif

	return sum;
}

template<typename SampleType>
void MultiBandProcessor<SampleType>::reset()
{
	for (int i = 0; i < CossackConstants::crossoverCount; i++) {
		filtersLHP_[i].reset();

#ifdef PHASE_CORRECTION_IMMEDIATE
		// FIXME: Make number of filters correct
		for (int j = 0; j < CossackConstants::crossoverCount; j++)
			filtersAP_[i][j].reset();
#else
		if (i > 0)
			filtersAP_[i - 1].reset();
#endif
	}
}

template<typename SampleType>
SampleType& MultiBandProcessor<SampleType>::getBand(int n)
{
	return bands_[n];
}

template<typename SampleType>
SampleType* MultiBandProcessor<SampleType>::getBandsPointer()
{
	return &bands_[0];
}
