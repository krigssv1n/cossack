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
	//juce::LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypeface(avenirNextCyrRegularFont.getTypefacePtr());
	//getLookAndFeel().setDefaultSansSerifTypeface(avenirNextCyrRegularFont.getTypefacePtr());

	auto& apvst = audioProcessor_.getValueTreeState();

	background_ = juce::ImageCache::getFromMemory(BinaryData::background_jpg, BinaryData::background_jpgSize);

	//avenirNextCyrRegularFont_ = juce::Font{ juce::Typeface::createSystemTypefaceFor(BinaryData::AvenirNextCyrRegular_ttf, BinaryData::AvenirNextCyrRegular_ttfSize) };
	collonseFont_ = juce::Font{ juce::Typeface::createSystemTypefaceFor(BinaryData::Collonse_ttf, BinaryData::Collonse_ttfSize) };
	collonseBoldFont_ = juce::Font{ juce::Typeface::createSystemTypefaceFor(BinaryData::CollonseBoldBold_ttf, BinaryData::CollonseBoldBold_ttfSize) };
	collonseHollowFont_ = juce::Font{ juce::Typeface::createSystemTypefaceFor(BinaryData::CollonseHollow_ttf, BinaryData::CollonseHollow_ttfSize) };

	// Make sure that before the constructor has finished, you've set the
	// editor's size to whatever you need it to be.
	setResizable(true, true);
	setResizeLimits(800, 600, 1200, 900);
	getConstrainer()->setFixedAspectRatio(1.333333333333);
	setSize(1000, 750);

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
	midSideButtons_[1].setButtonText("=");
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

	const char *harmonicsNames[10]{ "Burning", "Hot", "Warm", "Nice", "Mild", "Cool", "Brisk", "Chilly", "Cold", "Freezing" };
	const juce::Colour harmonicsButtonColours[10]{
		// set 1 (from flower picture)
		//{ 64, 64, 64 },
		//{ 146, 46, 0 },
		//{ 255, 0, 0 },
		//{ 255, 141, 0 },
		//{ 255, 255, 0 },
		//{ 9, 191, 0 },
		//{ 0, 0, 255 },
		//{ 199, 0, 199 },
		//{ 179, 179, 179 },
		//{ 255, 255, 255 }

		// set 2 (from light spectre)
		{ 255, 51, 51 },
		{ 255, 76, 63 },
		{ 255, 101, 69 },
		{ 255, 144, 68 },
		{ 255, 232, 100 },
		{ 195, 255, 85 },
		{ 110, 255, 132 },
		{ 74, 203, 255 },
		{ 121, 189, 255 },
		{ 119, 83, 255 }
	};
	const juce::Colour harmonicsTextColours[10]{
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 },
		//{ 255, 255, 255 }
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 },
		{ 0, 0, 0 }
	};

	for (int i = 0; i < 10; i++)
	{
		// Amplitude
		equalizerSideSliders_[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
		equalizerSideSliders_[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
		addAndMakeVisible(equalizerSideSliders_[i]);

		equalizerSideAttachments_[i].reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "equalizerSide" + std::to_string(i), equalizerSideSliders_[i]));

		equalizerMidSliders_[i].setSliderStyle(juce::Slider::SliderStyle::Rotary);
		equalizerMidSliders_[i].setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
		addAndMakeVisible(equalizerMidSliders_[i]);

		equalizerMidAttachments_[i].reset(new juce::AudioProcessorValueTreeState::SliderAttachment(apvst, "equalizerMid" + std::to_string(i), equalizerMidSliders_[i]));

		equalizerLabels_[i].setText(std::to_string(CossackAudioProcessor::frequencyBands[i]), juce::dontSendNotification);
		equalizerLabels_[i].setJustificationType(juce::Justification::centred);
		equalizerLabels_[i].setColour(juce::Label::textColourId, juce::Colours::black);
		addAndMakeVisible(equalizerLabels_[i]);

		// Harmonics
		harmonicsMidButtons_[i].setName("harmonicMidButton" + std::to_string(i));
		harmonicsMidButtons_[i].setColour(juce::TextButton::buttonColourId, harmonicsButtonColours[i].darker());
		harmonicsMidButtons_[i].setColour(juce::TextButton::buttonOnColourId, harmonicsButtonColours[i]);
		harmonicsMidButtons_[i].setColour(juce::TextButton::textColourOffId, { 0, 0, 0 });
		harmonicsMidButtons_[i].setColour(juce::TextButton::textColourOnId, { 255, 255, 255 });
		//harmonicsMidButtons_[i].setColour(juce::TextButton::textColourOffId, harmonicsTextColours[i].darker());
		//harmonicsMidButtons_[i].setColour(juce::TextButton::textColourOnId, harmonicsTextColours[i]);
		harmonicsMidButtons_[i].setButtonText("M");
		harmonicsMidButtons_[i].setTooltip("Mid harmonic enhancement for this frequency");
		harmonicsMidButtons_[i].setClickingTogglesState(true);
		addAndMakeVisible(harmonicsMidButtons_[i]);

		harmonicsMidAttachments_[i].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "harmonicsMid" + std::to_string(i), harmonicsMidButtons_[i]));

		if (i >= 2)
		{
			harmonicsSideButtons_[i - 2].setName("harmonicSideButton" + std::to_string(i));
			harmonicsSideButtons_[i - 2].setColour(juce::TextButton::buttonColourId, harmonicsButtonColours[i].darker());
			harmonicsSideButtons_[i - 2].setColour(juce::TextButton::buttonOnColourId, harmonicsButtonColours[i]);
			harmonicsSideButtons_[i - 2].setColour(juce::TextButton::textColourOffId, { 0, 0, 0 });
			harmonicsSideButtons_[i - 2].setColour(juce::TextButton::textColourOnId, { 255, 255, 255 });
			//harmonicsSideButtons_[i - 2].setColour(juce::TextButton::textColourOffId, harmonicsTextColours[i].darker());
			//harmonicsSideButtons_[i - 2].setColour(juce::TextButton::textColourOnId, harmonicsTextColours[i]);
			harmonicsSideButtons_[i - 2].setButtonText("S");
			harmonicsSideButtons_[i - 2].setTooltip("Side harmonic enhancement for this frequency");
			harmonicsSideButtons_[i - 2].setClickingTogglesState(true);
			addAndMakeVisible(harmonicsSideButtons_[i - 2]);

			harmonicsSideAttachments_[i - 2].reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(apvst, "harmonicsSide" + std::to_string(i), harmonicsSideButtons_[i - 2]));
		}

		harmonicsLabels_[i].setText(harmonicsNames[i], juce::dontSendNotification);
		harmonicsLabels_[i].setJustificationType(juce::Justification::centred);
		harmonicsLabels_[i].setColour(juce::Label::textColourId, juce::Colours::black);
		addAndMakeVisible(harmonicsLabels_[i]);
	}

	//equalizerHzLabel_.setText("Hz", juce::dontSendNotification);
	//equalizerHzLabel_.setJustificationType(juce::Justification::centred);
	//equalizerHzLabel_.setColour(juce::Label::textColourId, juce::Colours::black);
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
		optoLabel_.setText("OPTO", juce::dontSendNotification);
		optoLabel_.setJustificationType(juce::Justification::centred);
		optoLabel_.setColour(juce::Label::textColourId, juce::Colours::black);
		//optoLabel_.setColour(juce::Label::outlineColourId, juce::Colours::black);
		addAndMakeVisible(optoLabel_);

		glueLabel_.setText("GLUE", juce::dontSendNotification);
		glueLabel_.setJustificationType(juce::Justification::centred);
		glueLabel_.setColour(juce::Label::textColourId, juce::Colours::black);
		//glueLabel_.setColour(juce::Label::outlineColourId, juce::Colours::black);
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
		authorNameLabel_.setColour(juce::Label::textColourId, juce::Colours::black);
		addAndMakeVisible(authorNameLabel_);
	}

	{
		pluginNameLabel_.setText("COSSACK", juce::dontSendNotification);
		pluginNameLabel_.setJustificationType(juce::Justification::centred);
		pluginNameLabel_.setColour(juce::Label::textColourId, juce::Colours::black);
		addAndMakeVisible(pluginNameLabel_);
	}

	//svgTest_ = juce::Drawable::createFromImageData(BinaryData::testsvgrepocom_svg, BinaryData::testsvgrepocom_svgSize);
}

CossackAudioProcessorEditor::~CossackAudioProcessorEditor()
{
	// Guarantee the deletion of attachments before the UI & listener objects as JUCE suggests

	//
	// Low/high cut
	//

	lowCutAttachment_.reset(nullptr);
	highCutAttachment_.reset(nullptr);
	//rolloffFactorAttachment_.reset(nullptr);

	//
	// Mid/side
	//

	int i;

	for (i = 0; i < 3; i++)
		midSideAttachments_[i].reset(nullptr);

	for (i = 0; i < 10; i++)
	{
		//
		// Equalizer
		//

		equalizerSideAttachments_[i].reset(nullptr);
		equalizerMidAttachments_[i].reset(nullptr);

		//
		// Harmonics
		//

		harmonicsMidAttachments_[i].reset(nullptr);

		if (i >= 2)
			harmonicsSideAttachments_[i - 2].reset(nullptr);
	}

	//
	// Compressors (LA-2A & SSL G-Master)
	//

	optoAttachment_.reset(nullptr);
	glueAttachment_.reset(nullptr);
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
	g.drawImage(background_, getLocalBounds().toFloat());
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
	const float equalizerY = editor.getHeight() * 0.35f;
	const float equalizerMidDiameter = editor.getWidth() * 0.05f;
	const float equalizerSideDiameter = editor.getWidth() * 0.08f;
	//const float equalizerSpacing = 10.f;
	const float equalizerSpacing = (editor.getWidth() - equalizerX * 2.f) / 10.f - equalizerSideDiameter;
	const float equalizerLabelHeight = editor.getHeight() * 0.05f;

	for (int i = 0; i < 10; i++)
	{
		// Amplitude
		equalizerSideSliders_[i].setBounds(
			equalizerX + i * (equalizerSideDiameter + equalizerSpacing),
			equalizerY,
			equalizerSideDiameter,
			equalizerSideDiameter);

		const float offset = (equalizerSideDiameter - equalizerMidDiameter) * 0.5f;
		
		equalizerMidSliders_[i].setBounds(
			equalizerSideSliders_[i].getX() + offset,
			equalizerSideSliders_[i].getY() + offset,
			equalizerMidDiameter,
			equalizerMidDiameter);

		equalizerLabels_[i].setBounds(
			equalizerSideSliders_[i].getX(),
			equalizerSideSliders_[i].getY() + equalizerSideSliders_[i].getHeight(),
			equalizerSideSliders_[i].getWidth(),
			equalizerLabelHeight);
		
		// Harmonics
		const float harmonicButtonWidth = equalizerSideSliders_[i].getWidth() * 0.45f;
		const float harmonicButtonHeight = editor.getHeight() * 0.0333f;
		float harmonicButtonX = (equalizerSideSliders_[i].getX() + equalizerSideSliders_[i].getWidth() * 0.5f) - harmonicButtonWidth * 0.5f;
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

		harmonicsLabels_[i].setBounds(
			equalizerSideSliders_[i].getX(),
			harmonicsMidButtons_[i].getY() + harmonicsMidButtons_[i].getHeight(),
			equalizerSideSliders_[i].getWidth(),
			equalizerLabelHeight);
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
	const float compressorSliderHeight = editor.getHeight() * 0.3f;
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

	collonseFont_.setHeight(editor.getHeight() * 0.035f);
	optoLabel_.setFont(collonseFont_);
	glueLabel_.setFont(collonseFont_);

	//
	// Signal power indicator
	//

	//
	// Author & plugin info
	//

	// Author name
	const float authorNameWidth = editor.getWidth() * 0.3f;
	const float authorNameHeight = editor.getHeight() * 0.1f;

	authorNameLabel_.setBounds(
		editor.getCentreX() - authorNameWidth * 0.5f,
		editor.getHeight() - borderOffsetY - authorNameHeight,
		authorNameWidth,
		authorNameHeight);

	collonseHollowFont_.setHeight(editor.getHeight() * 0.045f);
	authorNameLabel_.setFont(collonseHollowFont_);

	// Author logo
	const float authorLogoWidth = editor.getWidth() * 0.09f;
	const float authorLogoHeight = authorLogoWidth * (442.f / 390.f);

	authorLogoLabel_.setBounds(
		editor.getCentreX() - authorLogoWidth * 0.5f,
		authorNameLabel_.getY() - authorLogoHeight * 1.5f,
		authorLogoWidth,
		authorLogoHeight);

	const float pluginNameWidth = editor.getWidth() * 0.3f;

	// Plugin name
	pluginNameLabel_.setBounds(
		editor.getCentreX() - pluginNameWidth * 0.5f,
		borderOffsetY,
		pluginNameWidth,
		editor.getHeight() * 0.05f);

	collonseBoldFont_.setHeight(editor.getHeight() * 0.07f);
	pluginNameLabel_.setFont(collonseBoldFont_);
}
