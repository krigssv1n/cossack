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

	//juce::Font avenirNextCyrRegularFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::AvenirNextCyrRegular_ttf, BinaryData::AvenirNextCyrRegular_ttfSize) };
	juce::Font collonseFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::Collonse_ttf, BinaryData::Collonse_ttfSize) };
	juce::Font collonseBoldFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::CollonseBoldBold_ttf, BinaryData::CollonseBoldBold_ttfSize) };
	juce::Font collonseHollowFont{ juce::Typeface::createSystemTypefaceFor(BinaryData::CollonseHollow_ttf, BinaryData::CollonseHollow_ttfSize) };

	//juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(avenirNextCyrRegularFont.getTypefacePtr());
	//getLookAndFeel().setDefaultSansSerifTypeface(avenirNextCyrRegularFont.getTypefacePtr());

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
	midSideButtons_[0].setTooltip("Mid-only playback");
	midSideButtons_[0].setClickingTogglesState(true);
	midSideButtons_[0].setRadioGroupId(1001);
	addAndMakeVisible(midSideButtons_[0]);

	midSideButtons_[1].setName("midSideButton");
	midSideButtons_[1].setButtonText("Mid/Side");
	midSideButtons_[1].setTooltip("Normal playback");
	midSideButtons_[1].setClickingTogglesState(true);
	midSideButtons_[1].setRadioGroupId(1001);
	addAndMakeVisible(midSideButtons_[1]);

	midSideButtons_[2].setName("sideButton");
	midSideButtons_[2].setButtonText("Side");
	midSideButtons_[2].setTooltip("Side-only playback");
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
		// Amplitude
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

	glueSlider_.setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
	glueSlider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
	addAndMakeVisible(glueSlider_);

	glueAttachment_.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "glue", glueSlider_));

	{
		collonseFont.setHeight(getBounds().getHeight() * 0.025f);

		optoLabel_.setText("Opto", juce::dontSendNotification);
		optoLabel_.setJustificationType(juce::Justification::centred);
		optoLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
		//optoLabel_.setColour(juce::Label::outlineColourId, juce::Colours::black);
		optoLabel_.setFont(collonseFont);
		addAndMakeVisible(optoLabel_);

		glueLabel_.setText("Glue", juce::dontSendNotification);
		glueLabel_.setJustificationType(juce::Justification::centred);
		glueLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);
		//glueLabel_.setColour(juce::Label::outlineColourId, juce::Colours::black);
		glueLabel_.setFont(collonseFont);
		addAndMakeVisible(glueLabel_);
	}

	//
	// Author & plugin info
	//

	//authorLogoLabel_.setImage(juce::ImageCache::getFromFile(juce::File::getCurrentWorkingDirectory().getChildFile("Images/logo.png"));
	authorLogoLabel_.setImage(juce::ImageCache::getFromMemory(BinaryData::logo_png, BinaryData::logo_pngSize));
	addAndMakeVisible(authorLogoLabel_);

	{
		authorNameLabel_.setText("PAUL\nDUBROVSKY", juce::dontSendNotification);
		authorNameLabel_.setJustificationType(juce::Justification::centred);
		authorNameLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);

		collonseHollowFont.setHeight(getBounds().getHeight() * 0.05f);
		authorNameLabel_.setFont(collonseHollowFont);

		addAndMakeVisible(authorNameLabel_);
	}

	{
		pluginNameLabel_.setText("Cossack", juce::dontSendNotification);
		pluginNameLabel_.setJustificationType(juce::Justification::centred);
		pluginNameLabel_.getLookAndFeel().setColour(juce::Label::textColourId, juce::Colours::black);

		collonseBoldFont.setHeight(getBounds().getHeight() * 0.05f);
		pluginNameLabel_.setFont(collonseBoldFont);

		addAndMakeVisible(pluginNameLabel_);
	}

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

	const float borderOffsetX = editor.getWidth() * 0.07f;
	const float borderOffsetY = editor.getWidth() * 0.07f;

	//
	// Low/high cut
	//

	//const float cutButtonWidth = 90.f;
	//const float cutButtonHeight = 60.f;
	//const float cutButtonOffset = 30.f;
	const float cutButtonWidth = editor.getWidth() * 0.07f;
	const float cutButtonHeight = editor.getHeight() * 0.05f;
	const float cutButtonX = borderOffsetX;
	const float cutButtonY = borderOffsetY;

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

	//const float msButtonWidth = 70.f;
	//const float msButtonHeight = 50.f;
	const float msButtonWidth = editor.getWidth() * 0.07f;
	const float msButtonHeight = editor.getHeight() * 0.05f;
	const float msButtonX = editor.getCentreX() - msButtonWidth * 1.5f;
	const float msButtonY = editor.getHeight() * 0.2f;

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

	//const float equalizerX = 30.f;
	const float equalizerX = borderOffsetX;
	const float equalizerY = editor.getHeight() * 0.3f;
	const float equalizerMidDiameter = editor.getWidth() * 0.08f;
	const float equalizerSideDiameter = editor.getWidth() * 0.05f;
	//const float equalizerSpacing = 10.f;
	const float equalizerSpacing = (editor.getWidth() - equalizerX * 2.f) / 10.f - equalizerMidDiameter;
	const float equalizerLabelHeight = editor.getHeight() * 0.05f;

	for (int i = 0; i < 10; i++)
	{
		// Amplitude
		equalizerMidSliders_[i].setBounds(
			equalizerX + i * (equalizerMidDiameter + equalizerSpacing),
			equalizerY,
			equalizerMidDiameter,
			equalizerMidDiameter);

		const float offset = (equalizerMidDiameter - equalizerSideDiameter) * 0.5f;
		
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
		const float harmonicButtonWidth = equalizerMidSliders_[i].getWidth() * 0.45f;
		const float harmonicButtonHeight = editor.getHeight() * 0.0333f;
		float harmonicButtonX = (equalizerMidSliders_[i].getX() + equalizerMidSliders_[i].getWidth() * 0.5f) - harmonicButtonWidth * 0.5f;
		const float harmonicButtonY = equalizerLabels_[i].getY() + equalizerLabels_[i].getHeight();

		if (i >= 2)
		{
			harmonicButtonX -= harmonicButtonWidth * 0.5f;

			harmonicsSideButtons_[i - 2].setBounds(
				harmonicButtonX + harmonicButtonWidth,
				harmonicButtonY,
				harmonicButtonWidth,
				harmonicButtonHeight);
		}

		harmonicsMidButtons_[i].setBounds(
			harmonicButtonX,
			harmonicButtonY,
			harmonicButtonWidth,
			harmonicButtonHeight);
	}

	//equalizerHzLabel_.setBounds(
	//	0,
	//	equalizerLabels_[0].getY(),
	//	30,
	//	equalizerLabels_[0].getHeight());

	//
	// Compressors
	//

	//const float compressorSliderWidth = 70.f;
	const float compressorSliderWidth = editor.getWidth() * 0.0875f;
	const float compressorSliderHeight = editor.getHeight() * 0.35f;
	const float compressorSliderX = borderOffsetX + editor.getWidth() * 0.1f;
	const float compressorSliderY = editor.getHeight() - borderOffsetY - compressorSliderHeight;
	const float compressorLabelHeight = compressorSliderHeight;

	optoSlider_.setBounds(
		compressorSliderX,
		compressorSliderY,
		compressorSliderWidth,
		compressorSliderHeight);
	
	optoLabel_.setBounds(
		borderOffsetX,
		optoSlider_.getY() + (optoSlider_.getHeight() - compressorLabelHeight) / 2,
		compressorSliderX - borderOffsetX,
		compressorLabelHeight);

	glueSlider_.setBounds(
		editor.getWidth() - compressorSliderX - compressorSliderWidth,
		compressorSliderY,
		compressorSliderWidth,
		compressorSliderHeight);

	glueLabel_.setBounds(
		glueSlider_.getX() + glueSlider_.getWidth(),
		glueSlider_.getY() + (glueSlider_.getHeight() - compressorLabelHeight) / 2,
		editor.getWidth() - glueSlider_.getX() - glueSlider_.getWidth() - borderOffsetX,
		compressorLabelHeight);

	//
	// Signal power indicator
	//

	//
	// Author & plugin info
	//

	const float authorNameWidth = editor.getWidth() * 0.3f;
	const float authorNameHeight = editor.getHeight() * 0.1f;

	authorNameLabel_.setBounds(
		editor.getCentreX() - authorNameWidth * 0.5f,
		editor.getHeight() - borderOffsetY - authorNameHeight,
		authorNameWidth,
		authorNameHeight);

	const float authorLogoWidth = editor.getWidth() * 0.09f;
	const float authorLogoHeight = authorLogoWidth * (442.f / 390.f);

	authorLogoLabel_.setBounds(
		editor.getCentreX() - authorLogoWidth * 0.5f,
		authorNameLabel_.getY() - authorLogoHeight * 1.5f,
		authorLogoWidth,
		authorLogoHeight);

	const float pluginNameWidth = editor.getWidth() * 0.3f;

	pluginNameLabel_.setBounds(
		editor.getCentreX() - pluginNameWidth * 0.5f,
		borderOffsetY,
		pluginNameWidth,
		editor.getHeight() * 0.05f);
}
