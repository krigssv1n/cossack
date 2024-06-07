/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "EllipticSlider.h"

//==============================================================================
/**
*/
class CossackAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    CossackAudioProcessorEditor (CossackAudioProcessor&);
    ~CossackAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    CossackAudioProcessor& audioProcessor_;

	juce::Image background_;

	//juce::Font avenirNextCyrRegularFont_;
	juce::Font collonseFont_;
	juce::Font collonseBoldFont_;
	juce::Font collonseHollowFont_;

	juce::TooltipWindow tooltipWindow_{ this };

	//
	// Low/high cut
	//

	juce::DropShadow lowCutShadow_{ juce::Colour(0x90000000), 8, juce::Point<int>(4, 4) };
	juce::TextButton lowCutButton_;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lowCutAttachment_;

	juce::TextButton highCutButton_;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> highCutAttachment_;

	//juce::Slider rolloffFactorSlider_;
	//std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rolloffFactorAttachment_;
	//juce::Label rolloffFactorLabel_;

	//
	// Mid/side
	//

	// { "midButton", juce::Colours::darkgrey, juce::Colours::dimgrey, juce::Colours::grey };
	juce::TextButton midSideButtons_[3];
	// TODO: make radio button group attachment class
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> midSideAttachments_[3];

	//
	// Equalizer
	//

	EllipticSlider equalizerSideSliders_[10];
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> equalizerSideAttachments_[10];

	EllipticSlider equalizerMidSliders_[10];
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> equalizerMidAttachments_[10];

	juce::Label equalizerLabels_[10];
	//juce::Label equalizerHzLabel_;

	//
	// Harmonics
	//

	juce::TextButton harmonicsMidButtons_[10];
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> harmonicsMidAttachments_[10];

	// We don't use frequencies 31 & 63 for adding side harmonics
	juce::TextButton harmonicsSideButtons_[8];
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> harmonicsSideAttachments_[8];

	juce::Label harmonicsLabels_[10];

	//
	// Compressors (LA-2A & SSL G-Master)
	//

	juce::Slider optoSlider_;
	juce::ImageButton optoLights_[10];
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> optoAttachment_;
	juce::Label optoLabel_;

	juce::Slider glueSlider_;
	juce::ImageButton glueLights_[10];
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glueAttachment_;
	juce::Label glueLabel_;

	//
	// Signal power indicator
	//

	//
	// Author & plugin info
	//

	juce::ImageComponent authorLogoLabel_;
	juce::Label authorNameLabel_;
	juce::Label pluginNameLabel_;

	//std::unique_ptr<juce::Drawable> svgTest_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossackAudioProcessorEditor)
};
