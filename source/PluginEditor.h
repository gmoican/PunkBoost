#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"

#define DEG2RADS 0.0174533f

//==============================================================================
/**
*/
class PunkBoostEditor  : public juce::AudioProcessorEditor
{
public:
    PunkBoostEditor (PunkBoostProcessor&);
    ~PunkBoostEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //=================== PARAMETER MANIPULATION ===================================
    void setSliderComponent(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName);
    void setToggleComponent(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& buttonAttachment, juce::String paramName);
    juce::AffineTransform knobRotation(float radians, float posX, float posY, float scaleFactor);
    
private:
    // Parameters
    juce::Slider charKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> charKnobAttachment;
    
    juce::Slider boostKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> boostKnobAttachment;
    
    juce::Slider midGainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midGainKnobAttachment;
    
    juce::Slider midFreqKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midFreqKnobAttachment;
    
    juce::Slider bassGainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> bassGainKnobAttachment;
    
    juce::Slider trebleGainKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> trebleGainKnobAttachment;
    
    juce::ToggleButton onToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onToggleAttachment;
    
    // Assets
    juce::Image background;
    juce::Image lightOff;
    juce::Image knobImage;
        
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PunkBoostProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkBoostEditor)
};
