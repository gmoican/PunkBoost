#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkBoostProcessor::PunkBoostProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), state(*this, nullptr, "parameters", createParams())
#endif
{
}

PunkBoostProcessor::~PunkBoostProcessor()
{
}

//==============================================================================
const juce::String PunkBoostProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PunkBoostProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PunkBoostProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PunkBoostProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PunkBoostProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PunkBoostProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PunkBoostProcessor::getCurrentProgram()
{
    return 0;
}

void PunkBoostProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String PunkBoostProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PunkBoostProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

// =========== PARAMETER LAYOUT ====================
juce::AudioProcessorValueTreeState::ParameterLayout PunkBoostProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
    params.push_back(std::make_unique<juce::AudioParameterBool>("ONOFF", "On/Off", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("BOOST", "Boost level", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), DEFAULT_BOOST, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("CHARACTER", "Character", juce::NormalisableRange<float>(0.0f, 10.0f, 0.1f), DEFAULT_CHAR, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MID_G", "Mids gain", juce::NormalisableRange<float>(-10.0f, 10.0f, 0.1f), DEFAULT_MID_G, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("MID_F", "Mids frequency", juce::NormalisableRange<float>(250.0f, 2500.0f, 0.1f), DEFAULT_MID_FREQ, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("BASS", "Bass gain", juce::NormalisableRange<float>(-10.0f, 10.0f, 0.1f), DEFAULT_BASS, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("TREBLE", "Treble gain", juce::NormalisableRange<float>(-10.0f, 10.0f, 0.1f), DEFAULT_TREB, "dB"));
        
    return { params.begin(), params.end() };
}

// ============ VALUE UPDATERS =====================
void PunkBoostProcessor::updateOnOff()
{
    auto ONOFF = state.getRawParameterValue("ONOFF");
    on = ONOFF->load();
}

void PunkBoostProcessor::updateBoost()
{
    auto OUT = state.getRawParameterValue("BOOST");
    boosterLevel.setGainDecibels(OUT->load());
}

void PunkBoostProcessor::updateCharacter()
{
    auto CHAR = state.getRawParameterValue("CHARACTER");
    
    auto charLowGain = juce::jmap(CHAR->load(), 0.f, 10.f, 1.f, 2.2f);
    auto charMidGain = juce::jmap(CHAR->load(), 0.f, 10.f, 1.f, 0.1f);
    auto charMidHiGain = juce::jmap(CHAR->load(), 0.f, 10.f, 1.f, 1.8f);
    auto charHiGain = juce::jmap(CHAR->load(), 0.f, 10.f, 1.f, 2.4f);
    
    double sampleRate = getSampleRate();
    
    *characterEq.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 130.f, 1.f, charLowGain);
    *characterEq.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 500.f, 1.f, charMidGain);
    *characterEq.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 680.f, 1.f, charMidHiGain);
    *characterEq.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 3100.f, 1.f, charHiGain);
}

void PunkBoostProcessor::updateEq()
{
    auto MIDG = state.getRawParameterValue("MID_G");
    auto MIDF = state.getRawParameterValue("MID_F");
    auto BASS = state.getRawParameterValue("BASS");
    auto TREBLE = state.getRawParameterValue("TREBLE");
    
    auto bassGain = juce::Decibels::decibelsToGain(BASS->load());
    auto trebleGain = juce::Decibels::decibelsToGain(TREBLE->load());
    auto midGain = juce::Decibels::decibelsToGain(MIDG->load());
    
    double sampleRate = getSampleRate();
    
    *preEq.get<0>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 80.f, 0.8f, bassGain);
    *preEq.get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, MIDF->load(), 1.f, midGain);
    *preEq.get<2>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 5000.f, 0.6f, trebleGain);
}

void PunkBoostProcessor::updateState()
{
    updateOnOff();
    updateCharacter();
    updateEq();
    updateBoost();
}

//==============================================================================
void PunkBoostProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
    
    preEq.prepare(spec);
    preEq.reset();
    *preEq.get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, 30.f);
    
    characterEq.prepare(spec);
    characterEq.reset();
    *characterEq.get<4>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 12000.f);
    
    boosterLevel.prepare(spec);
    boosterLevel.setRampDurationSeconds(0.1f);
    
    dryWetMixer.prepare(spec);
    dryWetMixer.setWetMixProportion(0.7f);
    
    booster.prepare(spec);
    booster.reset();
    booster.get<0>().setRatio(2.f);
    booster.get<0>().setAttack(1.f);
    booster.get<0>().setThreshold(-20.f);
    booster.get<0>().setRelease(50.f);
    booster.get<1>().functionToUse = saturator;
    booster.get<2>().setRatio(20.f);
    booster.get<2>().setAttack(15.f);
    booster.get<2>().setThreshold(-20.f);
    booster.get<2>().setRelease(100.f);
}

void PunkBoostProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PunkBoostProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PunkBoostProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateState();
    if(on)
    {
        juce::dsp::AudioBlock<float> audioBlock = juce::dsp::AudioBlock<float>(buffer);
        
        preEq.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
        boosterLevel.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
        dryWetMixer.pushDrySamples(audioBlock);
        
        booster.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
        dryWetMixer.mixWetSamples(audioBlock);
        
        characterEq.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
    }
}

//==============================================================================
bool PunkBoostProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PunkBoostProcessor::createEditor()
{
    return new PunkBoostEditor (*this);
}

//==============================================================================
void PunkBoostProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::ignoreUnused(destData);
}

void PunkBoostProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PunkBoostProcessor();
}
