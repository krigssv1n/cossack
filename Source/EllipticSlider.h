/*
  ==============================================================================

    EllipticSlider.h
    Created: 22 Apr 2024 5:50:48am
    Author:  KOT

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

class EllipticSlider : public juce::Slider
{
public:
	EllipticSlider();

	bool hitTest(int x, int y) override;

private:
	//==============================================================================
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EllipticSlider)
};
