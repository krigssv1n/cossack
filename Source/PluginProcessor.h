/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "LowHighCutProcessor.h"

//==============================================================================
/**
*/
class CossackAudioProcessor : public juce::AudioProcessor
{
public:
    //==============================================================================
    CossackAudioProcessor();
    ~CossackAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	juce::AudioProcessorValueTreeState& getValueTreeState();

private:
	juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	juce::AudioProcessorValueTreeState valueTreeState_;

	struct
	{
		// Low/high cut
		juce::AudioParameterBool* lowCut = nullptr;
		juce::AudioParameterBool* highCut = nullptr;

		// Equalizer
		juce::AudioParameterFloat* equalizer[10] = { nullptr };

		// Mid/side
		juce::AudioParameterBool* mid = nullptr;
		juce::AudioParameterBool* midSide = nullptr;
		juce::AudioParameterBool* side = nullptr;
		// TODO: make radio button group attachment class
		//juce::AudioParameterInt* midSide = nullptr;

		// Compressors
		juce::AudioParameterFloat* opto = { nullptr };
		juce::AudioParameterFloat* glue = { nullptr };
	} parameters_;

	LowHighCutProcessor midLowCutProcessor_{ 30.f, 8 };
	LowHighCutProcessor sideLowCutProcessor_{ 100.f, 2 };
	LowHighCutProcessor highCutProcessor_{ 20000.f, 8, true };

	//==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossackAudioProcessor)
};
