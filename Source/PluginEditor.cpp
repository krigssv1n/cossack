/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
CossackAudioProcessorEditor::CossackAudioProcessorEditor (CossackAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor_ (p)
{
    auto& apvst = audioProcessor_.getValueTreeState();

	background_ = juce::ImageCache::getFromMemory(BinaryData::background_jpg, BinaryData::background_jpgSize);

	// Low/high cut
	lowCutButton_.setName("lowCutButton_");
	lowCutButton_.setButtonText("Low Cut");
	lowCutButton_.setTooltip("Cut off frequencies below 30 Hz");
	lowCutButton_.setClickingTogglesState(true);
	addAndMakeVisible(lowCutButton_);

	lowCutAttachment_.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "lowCut", lowCutButton_));

	highCutButton_.setName("highCutButton_");
	highCutButton_.setButtonText("High Cut");
	highCutButton_.setTooltip("Cut off frequencies above 20000 Hz");
	highCutButton_.setClickingTogglesState(true);
	addAndMakeVisible(highCutButton_);

	highCutAttachment_.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "highCut", highCutButton_));

	// Equalizer
	const int frequencyBands[10]{ 31, 63, 125, 250, 500, 1000, 2000, 4000, 8000, 16000 };
	
	for (int i = 0; i < 10; i++)
	{
		equalizerSliders_[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
		equalizerSliders_[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
		addAndMakeVisible(equalizerSliders_[i]);

		equalizerAttachments_[i].reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "equalizer" + std::to_string(i), equalizerSliders_[i]));

		equalizerLabels_[i].setText(std::to_string(frequencyBands[i]), juce::dontSendNotification);
		equalizerLabels_[i].setJustificationType(juce::Justification::centred);
		equalizerLabels_[i].getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
		addAndMakeVisible(equalizerLabels_[i]);
	}

	equalizerHzLabel_.setText("Hz", juce::dontSendNotification);
	equalizerHzLabel_.setJustificationType(juce::Justification::centred);
	equalizerHzLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
	addAndMakeVisible(equalizerHzLabel_);

	// Mid/side
	midSideButtons_[0].setName("midButton");
	midSideButtons_[0].setButtonText("Mid");
	midSideButtons_[0].setTooltip("Mid-only equalizer");
	midSideButtons_[0].setClickingTogglesState(true);
	midSideButtons_[0].setRadioGroupId(1001);
	addAndMakeVisible(midSideButtons_[0]);

	midSideButtons_[1].setName("midSideButton");
	midSideButtons_[1].setButtonText("Mid/Side");
	midSideButtons_[1].setTooltip("Global equalizer");
	midSideButtons_[1].setClickingTogglesState(true);
	midSideButtons_[1].setRadioGroupId(1001);
	addAndMakeVisible(midSideButtons_[1]);

	midSideButtons_[2].setName("sideButton");
	midSideButtons_[2].setButtonText("Side");
	midSideButtons_[2].setTooltip("Side-only equalizer");
	midSideButtons_[2].setClickingTogglesState(true);
	midSideButtons_[2].setRadioGroupId(1001);
	addAndMakeVisible(midSideButtons_[2]);

	midSideAttachments_[0].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "mid", midSideButtons_[0]));
	midSideAttachments_[1].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "midSide", midSideButtons_[1]));
	midSideAttachments_[2].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "side", midSideButtons_[2]));
	// TODO: make radio button group attachment class
	//midSideAttachment_.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "mid", midButton_));

	// Compressors
	optoSlider_.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	optoSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
	addAndMakeVisible(optoSlider_);

	optoAttachment_.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "opto", optoSlider_));

	optoLabel_.setText("Opto", juce::dontSendNotification);
	optoLabel_.setJustificationType(juce::Justification::centred);
	optoLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
	addAndMakeVisible(optoLabel_);

	glueSlider_.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	glueSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
	addAndMakeVisible(glueSlider_);

	glueAttachment_.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "glue", glueSlider_));

	glueLabel_.setText("Glue", juce::dontSendNotification);
	glueLabel_.setJustificationType(juce::Justification::centred);
	glueLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
	addAndMakeVisible(glueLabel_);

	// Author's logo
	//authorLogoLabel_.setImage(juce::ImageCache::getFromFile(juce::File::getCurrentWorkingDirectory().getChildFile("Images/logo.png"));
	authorLogoLabel_.setImage(juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));
	addAndMakeVisible(authorLogoLabel_);

	//svgTest_ = juce::Drawable::createFromImageData(BinaryData::testsvgrepocom_svg, BinaryData::testsvgrepocom_svgSize);

	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(800, 600);
}

CossackAudioProcessorEditor::~CossackAudioProcessorEditor()
{
}

//==============================================================================
void CossackAudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    //g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
	//g.fillAll(juce::Colours::silver);
/*
	juce::ColourGradient cg(
		juce::Colour(0xFFF0F0F0), 400, 0,
		juce::Colour(0xFF000000), 400 + 2000, 0 - 750,
		true);

	g.setGradientFill(cg);
	g.fillAll();
*/
	g.drawImageAt(background_, 0, 0);
	//svgTest_->draw(g, 1.f);

	//lowCutShadow_.drawForRectangle(g, lowCutButton_.getBounds());
	
	//static const unsigned char pathData[] = { 110,109,0,0,0,0,0,0,0,0,108,0,0,0,0,0,0,0,0,99,101,0,0 };

	//juce::Path path;
	//path.loadPathFromData (pathData, sizeof (pathData));
	//g.fillPath(path);
	//g.strokePath(path, juce::PathStrokeType(5.0f));
		
	//g.setColour (juce::Colours::white);
    //g.setFont (16.0f);
    //g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
}

void CossackAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..

	// Low/high cut
	const int cutButtonWidth = 90;
	const int cutButtonHeight = 60;

	lowCutButton_.setBounds(
		30,
		90,
		cutButtonWidth,
		cutButtonHeight);

	highCutButton_.setBounds(
		800 - 30 - cutButtonWidth,
		90,
		cutButtonWidth,
		cutButtonHeight);

	// Equalizer
	const int equalizerKnobDiameter = 65;
	const int equalizerKnobSpacing = 10;
	const int equalizerLabelHeight = 30;

	for (int i = 0; i < 10; i++)
	{
		equalizerSliders_[i].setBounds(
			30 + i * (equalizerKnobDiameter + equalizerKnobSpacing),
			240,
			equalizerKnobDiameter,
			equalizerKnobDiameter);

		equalizerLabels_[i].setBounds(
			equalizerSliders_[i].getX(),
			equalizerSliders_[i].getY() + equalizerSliders_[i].getHeight(),
			equalizerSliders_[i].getWidth(),
			equalizerLabelHeight);
	}

	equalizerHzLabel_.setBounds(
		0,
		equalizerLabels_[0].getY(),
		30,
		equalizerLabelHeight);

	// Mono/stereo
	const int msButtonWidth = 70;
	const int msButtonHeight = 50;

	//juce::Path msButtonPath;
	//msButtonPath.addEllipse(0, 0, msButtonWidth, msButtonHeight);

	//midButton_.setShape(msButtonPath, true, false, true);
	//midButton_.setTopLeftPosition(160, 160);
	midSideButtons_[0].setBounds(
		400 - msButtonWidth * 3 / 2,
		160,
		msButtonWidth,
		msButtonHeight);

	midSideButtons_[1].setBounds(
		midSideButtons_[0].getX() + msButtonWidth,
		160,
		msButtonWidth,
		msButtonHeight);

	midSideButtons_[2].setBounds(
		midSideButtons_[1].getX() + msButtonWidth,
		160,
		msButtonWidth,
		msButtonHeight);

	// Compressors
	const int compressorSliderWidth = 70;
	const int compressorSliderHeight = 200;
	const int compressorLabelHeight = 30;

	optoSlider_.setBounds(
		160,
		600 - compressorSliderHeight - 30,
		compressorSliderWidth,
		compressorSliderHeight);
	
	optoLabel_.setBounds(
		0,
		optoSlider_.getY() + (optoSlider_.getHeight() - compressorLabelHeight) / 2,
		160,
		compressorLabelHeight);

	glueSlider_.setBounds(
		800 - compressorSliderWidth - 160,
		600 - compressorSliderHeight - 30,
		compressorSliderWidth,
		compressorSliderHeight);

	glueLabel_.setBounds(
		glueSlider_.getX() + glueSlider_.getWidth(),
		glueSlider_.getY() + (glueSlider_.getHeight() - compressorLabelHeight) / 2,
		160,
		compressorLabelHeight);

	// Signal power indicator

	// Author's logo
	const int logoWidth = 390 * 0.2;
	const int logoHeight = 442 * 0.2;

	authorLogoLabel_.setBounds(400 - logoWidth / 2, 40, logoWidth, logoHeight);
}
