//
// Created by tar on 23/11/22.
//

#ifndef SUCCULENT_MULTICHANNELAUDIOSOURCE_H
#define SUCCULENT_MULTICHANNELAUDIOSOURCE_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_events/juce_events.h>
#include <juce_audio_formats/juce_audio_formats.h>

using namespace juce;

/**
 * Hat tip: https://forum.juce.com/t/how-best-to-read-multi-mono-wave-files-into-a-single-audio-source
 *
 * Designed to handle monophonic sources. Provided with a multichannel file, it
 * will read the first channel only.
 */
class MultiChannelAudioSource : public PositionableAudioSource, public ChangeBroadcaster {
public:
    explicit MultiChannelAudioSource(uint maxNumSources);

    void prepareToPlay(int samplesPerBlockExpected, double sampleRateToUse) override;

    void releaseResources() override;

    void getNextAudioBlock(const AudioSourceChannelInfo &bufferToFill) override;

    void setNextReadPosition(int64 newPosition) override;

    int64 getNextReadPosition() const override;

    int64 getTotalLength() const override;

    bool isLooping() const override;

    void addSource(uint index, File &file);

    void removeSource(uint index);

    void start();

    void stop();

    void setGain(float newGain);

private:
    bool canAddSource();

    AudioFormatManager formatManager;
    std::unordered_map<uint, std::unique_ptr<AudioFormatReaderSource>> sources;
    AudioBuffer<float> tempBuffer;

    CriticalSection lock;

    int blockSize{0};
    double sampleRate{0.};
    float gain{1.0f}, lastGain{1.0f};
    std::atomic<bool> playing{false}, stopped{true};
    bool isPrepared = false;
    uint maxSources;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MultiChannelAudioSource)
};


#endif //SUCCULENT_MULTICHANNELAUDIOSOURCE_H
