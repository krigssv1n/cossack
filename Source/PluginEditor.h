/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

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

	juce::TooltipWindow tooltipWindow_ { this };

	// Low/high cut
	juce::DropShadow lowCutShadow_{ juce::Colour(0x90000000), 8, juce::Point<int>(4, 4) };
	juce::TextButton lowCutButton_;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> lowCutAttachment_;

	juce::TextButton highCutButton_;
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> highCutAttachment_;

	//juce::Slider rolloffFactorSlider_;
	//std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> rolloffFactorAttachment_;
	//juce::Label rolloffFactorLabel_;

	// Equalizer
	juce::Slider equalizerSliders_[10];
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> equalizerAttachments_[10];
	juce::Label equalizerLabels_[10];
	juce::Label equalizerHzLabel_;

	// Mid/side
	// { "midButton", juce::Colours::darkgrey, juce::Colours::dimgrey, juce::Colours::grey };
	juce::TextButton midSideButtons_[3];
	// TODO: make radio button group attachment class
	std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> midSideAttachments_[3];

	// Compressors (LA-2A & SSL G-Master)
	juce::Slider optoSlider_;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> optoAttachment_;
	juce::Label optoLabel_;

	juce::Slider glueSlider_;
	std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> glueAttachment_;
	juce::Label glueLabel_;

	// Signal power indicator
	
	// Author's logo
	juce::ImageComponent authorLogoLabel_;
	juce::Label authorNameLabel_;

	//std::unique_ptr<juce::Drawable> svgTest_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossackAudioProcessorEditor)
};
