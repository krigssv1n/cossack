/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Common.h"
#include "LowHighCutProcessor.h"
#include "MultiBandProcessor.h"

//==============================================================================
/**
*/
class CossackAudioProcessor :
	public juce::AudioProcessor,
	private juce::AudioProcessorValueTreeState::Listener
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
	// Creates parameter list for the APVTS
	static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

	void parameterChanged(const juce::String& parameterID, float newValue) override;
	void updateParameters();

	float testHarmonics(float sample);

	juce::AudioProcessorValueTreeState valueTreeState_;

	// Set in prepareToPlay()
	double sampleRate_;

	struct
	{
		// Low/high cut
		juce::AudioParameterBool* lowCut;
		juce::AudioParameterBool* highCut;

		// Mid/side
		juce::AudioParameterBool* mid;
		juce::AudioParameterBool* midSide;
		juce::AudioParameterBool* side;
		// TODO: Make radio button group attachment class.
		//juce::AudioParameterInt* midSide;

		// Equalizer
		juce::AudioParameterFloat* equalizers[2][CossackConstants::bandCount];

		// Harmonics
		juce::AudioParameterBool* harmonicsMid[10];
		juce::AudioParameterBool* harmonicsSide[8];

		// Compressors
		juce::AudioParameterFloat* opto;
		juce::AudioParameterFloat* glue;
	} parameters_;

	LowHighCutProcessor lowCutProcessor_[2];
	LowHighCutProcessor highCutProcessor_;

	MultiBandProcessor<float> multiBandProcessor_;

	juce::dsp::Gain<float> equalizerGains_[2][CossackConstants::bandCount];

	// FIXME: temporary
	using Filter = juce::dsp::IIR::Filter<float>;
	using Coefficients = juce::dsp::IIR::Coefficients<float>;
	using Duplicator = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

	juce::dsp::ProcessorChain<Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator> equalizerProcessors_[2];

	juce::dsp::Convolution convolution_;

	//==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossackAudioProcessor)
};
