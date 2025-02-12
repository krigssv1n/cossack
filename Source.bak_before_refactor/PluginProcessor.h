/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
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

	//static constexpr double inverseSqrt2 = static_cast<double> (0.70710678118654752440L);
	static constexpr double inverseSqrt2 = 1.0 / juce::MathConstants<double>::sqrt2;

	// Same as the number of bands
	static constexpr int centralFrequencies[]{ 31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };

	// One less than the number of bands
	static constexpr float crossoverFrequencies[]{ 46.875f, 93.75f, 187.5f, 375.f, 750.f, 1500.f, 3000.f, 6000.f, 12000.f };

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
		juce::AudioParameterFloat* equalizers[2][10];

		// Harmonics
		juce::AudioParameterBool* harmonicsMid[10];
		juce::AudioParameterBool* harmonicsSide[8];

		// Compressors
		juce::AudioParameterFloat* opto;
		juce::AudioParameterFloat* glue;
	} parameters_;

	LowHighCutProcessor lowCutProcessor_[2];
	LowHighCutProcessor highCutProcessor_;

	// FIXME: temporary
	using Filter = juce::dsp::IIR::Filter<float>;
	using Coefficients = juce::dsp::IIR::Coefficients<float>;
	using Duplicator = juce::dsp::ProcessorDuplicator<Filter, Coefficients>;

	juce::dsp::ProcessorChain<Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator, Duplicator> equalizerProcessors_[2];

	juce::dsp::Convolution convolution_;

	MultiBandProcessor<float> multiBandProcessor_;

	//==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CossackAudioProcessor)
};
