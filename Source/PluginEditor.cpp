/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "Common.h"

//==============================================================================
CossackAudioProcessorEditor::CossackAudioProcessorEditor (CossackAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor_ (p)
{
	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setSize(1000, 750);

	auto& apvst = audioProcessor_.getValueTreeState();

	background_ = juce::ImageCache::getFromMemory(BinaryData::background_jpg, BinaryData::background_jpgSize);

	//
	// Low/high cut
	//

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

	//
	// Mid/side
	//

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

	//
	// Equalizer & harmonics
	//

	for (int i = 0; i < 10; i++)
	{
		equalizerMidSliders_[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
		equalizerMidSliders_[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
		addAndMakeVisible(equalizerMidSliders_[i]);

		equalizerMidAttachments_[i].reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "equalizerMid" + std::to_string(i), equalizerMidSliders_[i]));

		equalizerSideSliders_[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
		equalizerSideSliders_[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
		addAndMakeVisible(equalizerSideSliders_[i]);

		equalizerSideAttachments_[i].reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "equalizerSide" + std::to_string(i), equalizerSideSliders_[i]));

		equalizerLabels_[i].setText(std::to_string(g_frequencyBands[i]), juce::dontSendNotification);
		equalizerLabels_[i].setJustificationType(juce::Justification::centred);
		equalizerLabels_[i].getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
		addAndMakeVisible(equalizerLabels_[i]);

		// Harmonics
		harmonicsMidButtons_[i].setName("harmonicMidButton" + std::to_string(i));
		harmonicsMidButtons_[i].setButtonText("M");
		harmonicsMidButtons_[i].setTooltip("Mid harmonic enhancement for this frequency");
		harmonicsMidButtons_[i].setClickingTogglesState(false);
		addAndMakeVisible(harmonicsMidButtons_[i]);

		harmonicsMidAttachments_[i].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "harmonicMid" + std::to_string(i), harmonicsMidButtons_[i]));

		if (i >= 2)
		{
			harmonicsSideButtons_[i - 2].setName("harmonicSideButton" + std::to_string(i - 2));
			harmonicsSideButtons_[i - 2].setButtonText("S");
			harmonicsSideButtons_[i - 2].setTooltip("Side harmonic enhancement for this frequency");
			harmonicsSideButtons_[i - 2].setClickingTogglesState(false);
			addAndMakeVisible(harmonicsSideButtons_[i - 2]);

			harmonicsSideAttachments_[i - 2].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "harmonicSide" + std::to_string(i - 2), harmonicsSideButtons_[i - 2]));
		}
	}

	//equalizerHzLabel_.setText("Hz", juce::dontSendNotification);
	//equalizerHzLabel_.setJustificationType(juce::Justification::centred);
	//equalizerHzLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
	//addAndMakeVisible(equalizerHzLabel_);

	//
	// Compressors
	//

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

	//
	// Author & plugin info
	//

	//pluginNameLabel_.setText("COSSACK", juce::dontSendNotification);
	//pluginNameLabel_.setJustificationType(juce::Justification::centred);
	//pluginNameLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
	//addAndMakeVisible(pluginNameLabel_);

	//authorLogoLabel_.setImage(juce::ImageCache::getFromFile(juce::File::getCurrentWorkingDirectory().getChildFile("Images/logo.png"));
	authorLogoLabel_.setImage(juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));
	addAndMakeVisible(authorLogoLabel_);

	authorNameLabel_.setText("PAUL\nDUBROVSKY", juce::dontSendNotification);
	authorNameLabel_.setJustificationType(juce::Justification::centred);
	authorNameLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);

	juce::Font font{ juce::Typeface::createSystemTypefaceFor(BinaryData::CollonseHollow_ttf, BinaryData::CollonseHollow_ttfSize) };
	font.setHeight(getBounds().getWidth() * 0.05f);
	authorNameLabel_.setFont(font);
	addAndMakeVisible(authorNameLabel_);

	//svgTest_ = juce::Drawable::createFromImageData(BinaryData::testsvgrepocom_svg, BinaryData::testsvgrepocom_svgSize);
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

	// Editor rectangle (main window)
	const auto editor{ getLocalBounds() };

	//
	// Low/high cut
	//

	//const int cutButtonWidth = 90;
	//const int cutButtonHeight = 60;
	//const int cutButtonOffset = 30;
	const int cutButtonX = editor.getWidth() * 0.0375f;
	const int cutButtonY = editor.getHeight() * 0.0375f;
	const int cutButtonWidth = editor.getWidth() * 0.1f;
	const int cutButtonHeight = editor.getHeight() * 0.1f;

	lowCutButton_.setBounds(
		cutButtonX,
		cutButtonY,
		cutButtonWidth,
		cutButtonHeight);

	highCutButton_.setBounds(
		editor.getWidth() - cutButtonX - cutButtonWidth,
		cutButtonY,
		cutButtonWidth,
		cutButtonHeight);

	//
	// Mid/side
	//

	//const int msButtonWidth = 70;
	//const int msButtonHeight = 50;
	const int msButtonWidth = editor.getWidth() * 0.0875f;
	const int msButtonHeight = editor.getWidth() * 0.085f;
	const int msButtonX = editor.getCentreX() - msButtonWidth * 1.5f;
	const int msButtonY = editor.getHeight() * 0.2f;

	midSideButtons_[0].setBounds(
		msButtonX,
		msButtonY,
		msButtonWidth,
		msButtonHeight);

	midSideButtons_[1].setBounds(
		msButtonX + msButtonWidth,
		msButtonY,
		msButtonWidth,
		msButtonHeight);

	midSideButtons_[2].setBounds(
		msButtonX + msButtonWidth * 2,
		msButtonY,
		msButtonWidth,
		msButtonHeight);

	//
	// Equalizer
	//

	//const int equalizerX = 30;
	const float equalizerX = editor.getWidth() * 0.0375f;
	const float equalizerY = editor.getHeight() * 0.35f;
	const float equalizerMidDiameter = editor.getWidth() * 0.08f;
	const float equalizerSideDiameter = editor.getWidth() * 0.05f;
	//const int equalizerSpacing = 10;
	const float equalizerSpacing = (editor.getWidth() - equalizerX * 2.f) / 10.f - equalizerMidDiameter;
	const int equalizerLabelHeight = editor.getHeight() * 0.05f;

	for (int i = 0; i < 10; i++)
	{
		// Amplitude
		equalizerMidSliders_[i].setBounds(
			equalizerX + i * (equalizerMidDiameter + equalizerSpacing),
			equalizerY,
			equalizerMidDiameter,
			equalizerMidDiameter);

		const int offset = (equalizerMidDiameter - equalizerSideDiameter) / 2;
		
		equalizerSideSliders_[i].setBounds(
			equalizerMidSliders_[i].getX() + offset,
			equalizerMidSliders_[i].getY() + offset,
			equalizerSideDiameter,
			equalizerSideDiameter);

		equalizerLabels_[i].setBounds(
			equalizerMidSliders_[i].getX(),
			equalizerMidSliders_[i].getY() + equalizerMidSliders_[i].getHeight(),
			equalizerMidSliders_[i].getWidth(),
			equalizerLabelHeight);
		
		// Harmonics
		juce::Rectangle<int> harmonicsMidBounds;

		harmonicsMidBounds.setY(equalizerLabels_[i].getY() + equalizerLabels_[i].getHeight());
		harmonicsMidBounds.setWidth(equalizerMidSliders_[i].getWidth() * 0.35f);
		harmonicsMidBounds.setHeight(editor.getHeight() * 0.025f);

		const int harmonicButtonX = (equalizerMidSliders_[i].getX() + equalizerMidSliders_[i].getWidth() / 2) - harmonicsMidBounds.getWidth() / 2;

		if (i >= 2)
		{
			harmonicsMidBounds.setX(harmonicButtonX - harmonicsMidBounds.getWidth() / 2);

			harmonicsSideButtons_[i - 2].setBounds(
				harmonicsMidBounds.getX() + harmonicsMidBounds.getWidth(),
				harmonicsMidBounds.getY(),
				harmonicsMidBounds.getWidth(),
				harmonicsMidBounds.getHeight());
		}
		else
		{
			harmonicsMidBounds.setX(harmonicButtonX);
		}

		harmonicsMidButtons_[i].setBounds(harmonicsMidBounds);
	}

	//equalizerHzLabel_.setBounds(
	//	0,
	//	equalizerLabels_[0].getY(),
	//	30,
	//	equalizerLabels_[0].getHeight());

	//
	// Compressors
	//

	//const int compressorSliderWidth = 70;
	const int compressorSliderWidth = editor.getWidth() * 0.0875f;
	const int compressorSliderHeight = editor.getHeight() * 0.35f;
	const int compressorSliderX = editor.getWidth() * 0.2f;
	const int compressorSliderY = editor.getHeight() * 0.95f - compressorSliderHeight;
	const int compressorLabelHeight = editor.getHeight() * 0.05f;

	optoSlider_.setBounds(
		compressorSliderX,
		compressorSliderY,
		compressorSliderWidth,
		compressorSliderHeight);
	
	optoLabel_.setBounds(
		0,
		optoSlider_.getY() + (optoSlider_.getHeight() - compressorLabelHeight) / 2,
		compressorSliderX,
		compressorLabelHeight);

	glueSlider_.setBounds(
		editor.getWidth() - compressorSliderWidth - compressorSliderX,
		compressorSliderY,
		compressorSliderWidth,
		compressorSliderHeight);

	glueLabel_.setBounds(
		glueSlider_.getX() + glueSlider_.getWidth(),
		glueSlider_.getY() + (glueSlider_.getHeight() - compressorLabelHeight) / 2,
		compressorSliderX,
		compressorLabelHeight);

	//
	// Signal power indicator
	//

	//
	// Author & plugin info
	//

	const int logoWidth = editor.getWidth() * 0.1;
	const int logoHeight = logoWidth * (442.f / 390.f);

	authorLogoLabel_.setBounds(editor.getCentreX() - logoWidth / 2, optoSlider_.getY(), logoWidth, logoHeight);

	const int nameX = optoSlider_.getX() + optoSlider_.getWidth();
	const int nameY = authorLogoLabel_.getY() + authorLogoLabel_.getHeight();
	
	authorNameLabel_.setBounds(
		nameX,
		nameY,
		glueSlider_.getX() - nameX,
		editor.getHeight() - nameY);
}
