#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#if (MSVC)
#include "ipps.h"
#endif

#define DEFAULT_BOOST 0.0f
#define DEFAULT_CHAR 5.0f
#define DEFAULT_MID_G 0.0f
#define DEFAULT_MID_FREQ 1375.0f
#define DEFAULT_BASS 0.0f
#define DEFAULT_TREB 0.0f

//==============================================================================
/**
*/

class PunkBoostProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PunkBoostProcessor();
    ~PunkBoostProcessor() override;

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
    
    //=============== MY STUFF =====================================================
    juce::AudioProcessorValueTreeState state;
    
    // Updaters
    void updateOnOff();
    void updateBoost();
    void updateCharacter();
    void updateEq();
    void updateState();
    
    void process(float* samples, int numSamples);

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using WaveShaper = juce::dsp::WaveShaper<float>;
    using Gain = juce::dsp::Gain<float>;
    using Comp = juce::dsp::Compressor<float>;
    using Mix = juce::dsp::DryWetMixer<float>;
    
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand> preEq;
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand, FilterBand> characterEq;
    juce::dsp::ProcessorChain<Comp, WaveShaper, Comp> booster;
    
    Gain boosterLevel;
    Mix dryWetMixer;
    bool on;
    
    // Drive functions
    static float saturator(float sample)
    {
        // TODO: Investigate better way to implement an harmonic exciter
        // Level detector -> Full wave rectifier
        // float fwr = abs(sample);
        
        // Non linearity -> softClipper into hardClipper
        return tanhClipper( softClipper(sample) );
    }
    
    static float softClipper(float sample)
    {
        return sample / (abs(sample) + 1.f);
    }
    
    static float tanhClipper(float sample)
    {
        return 2.f / juce::MathConstants<float>::pi * juce::dsp::FastMathApproximations::tanh(sample);
    }
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkBoostProcessor)
};
