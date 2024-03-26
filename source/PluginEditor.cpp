#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkBoostEditor::PunkBoostEditor (PunkBoostProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    juce::ignoreUnused(audioProcessor);
    
    // ================= PARAMETERS ====================
    setSliderComponent(charKnob, charKnobAttachment, "CHARACTER");
    setSliderComponent(boostKnob, boostKnobAttachment, "BOOST");
    
    setSliderComponent(midGainKnob, midGainKnobAttachment, "MID_G");
    setSliderComponent(midFreqKnob, midFreqKnobAttachment, "MID_F");
    
    setSliderComponent(bassGainKnob, bassGainKnobAttachment, "BASS");
    setSliderComponent(trebleGainKnob, trebleGainKnobAttachment, "TREBLE");

    setToggleComponent(onToggle, onToggleAttachment, "ONOFF");

    // ================= ASSETS =======================
    background = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    lightOff = juce::ImageCache::getFromMemory(BinaryData::lightOff_png, BinaryData::lightOff_pngSize);
    knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
        
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (180, 320);
}

PunkBoostEditor::~PunkBoostEditor()
{
}

//==============================================================================
void PunkBoostEditor::paint (juce::Graphics& g)
{
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
        
    // =========== On/Off state ====================
    if (!onToggle.getToggleState()) {
        juce::AffineTransform t;
        t = t.scaled(0.485f);
        t = t.translated(75.5, 163.5);
        g.drawImageTransformed(lightOff, t);
    }
    
    // ========== Parameter knobs angle in radians ==================
    auto charRadians = juce::jmap(charKnob.getValue(), 0.0, 10.0, -150.0, 150.0) * DEG2RADS;
    auto boostRadians = juce::jmap(boostKnob.getValue(), -18.0, 18.0, -150.0, 150.0) * DEG2RADS;
    auto midGainRadians = juce::jmap(midGainKnob.getValue(), -10.0, 10.0, -150.0, 150.0) * DEG2RADS;
    auto midFreqRadians = juce::jmap(midFreqKnob.getValue(), 250.0, 2500.0, -150.0, 150.0) * DEG2RADS;
    auto bassGainRadians = juce::jmap(bassGainKnob.getValue(), -10.0, 10.0, -150.0, 150.0) * DEG2RADS;
    auto trebleGainRadians = juce::jmap(trebleGainKnob.getValue(), -10.0, 10.0, -150.0, 150.0) * DEG2RADS;
    
    // ========== Draw parameter knobs ==================
    g.drawImageTransformed(knobImage, knobRotation(charRadians, 23.5, 23.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(boostRadians, 112.5, 23.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(midGainRadians, 23.5, 91.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(midFreqRadians, 112.5, 91.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(bassGainRadians, 23.5, 158.0, 0.24));
    g.drawImageTransformed(knobImage, knobRotation(trebleGainRadians, 132.5, 158.0, 0.24));
}

void PunkBoostEditor::resized()
{
    // Upper row
    charKnob.setBounds(24, 23, 46, 46);
    boostKnob.setBounds(113, 23, 46, 46);
    
    // Middle row
    midGainKnob.setBounds(24, 91, 46, 46);
    midFreqKnob.setBounds(113, 91, 46, 46);
    
    // Bottom row
    bassGainKnob.setBounds(24, 158, 24, 24);
    trebleGainKnob.setBounds(133, 158, 24, 24);
    
    // OnOff
    onToggle.setBounds(65, 240, 50, 50);
}

void PunkBoostEditor::setSliderComponent(juce::Slider &slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> &sliderAttachment, juce::String paramName){
    sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, paramName, slider);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);
    slider.setAlpha(0);
}

void PunkBoostEditor::setToggleComponent(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& buttonAttachment, juce::String paramName){
    buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.state, paramName, button);
    addAndMakeVisible(button);
    button.setAlpha(0);
}

juce::AffineTransform PunkBoostEditor::knobRotation(float radians, float posX, float posY, float scaleFactor){
    juce::AffineTransform t;
    t = t.rotated(radians, 46.0f, 46.0f);
    t = t.scaled(scaleFactor);
    t = t.translated(posX, posY);
    return t;
}
