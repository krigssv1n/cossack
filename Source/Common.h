/*
  ==============================================================================

    Common.h
    Created: 23 Feb 2024 7:57:02pm
    Author:  KOT

  ==============================================================================
*/

#pragma once

#include <vector>

class CossackConstants
{
public:
	// Band central frequencies, used for editor display
	static constexpr int bandFrequencies[]{ 31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };
	static constexpr int bandCount = std::size(bandFrequencies);
	static_assert(bandCount >= 1);

	// Crossover frequencies, always one less than the number of bands
	static constexpr float crossoverFrequencies[]{ 46.875f, 93.75f, 187.5f, 375.f, 750.f, 1500.f, 3000.f, 6000.f, 12000.f };
	static constexpr int crossoverCount = std::size(crossoverFrequencies);
	static_assert(crossoverCount == (bandCount - 1));
};