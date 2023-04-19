//
// Created by tar on 16/04/23.
//

#ifndef SUCCULENT_XYCONTROLLERATTACHMENT_H
#define SUCCULENT_XYCONTROLLERATTACHMENT_H

#include <juce_audio_processors/juce_audio_processors.h>
#include "XYController.h"

class XYControllerNodeParameterAttachment : private XYController::Node::Listener {
public:
    XYControllerNodeParameterAttachment(juce::RangedAudioParameter &parameterX,
                                        juce::RangedAudioParameter &parameterY,
                                        XYController::Node &n,
                                        juce::UndoManager *um = nullptr);

    ~XYControllerNodeParameterAttachment() override;

    void sendInitialUpdate();

private:
    void setValueX(float newX);

    void setValueY(float newY);

    void nodeValueChanged(XYController::Node *) override;

    void nodeDragStarted(XYController::Node *) override;

    void nodeDragEnded(XYController::Node *) override;

    XYController::Node &node;
    juce::ParameterAttachment attachmentX, attachmentY;
    bool ignoreCallbacks = false;
};

class XYControllerNodeAttachment {
public:
    XYControllerNodeAttachment(juce::AudioProcessorValueTreeState &state,
                               const juce::String &parameterIDX,
                               const juce::String &parameterIDY,
                               XYController::Node &node);
private:
    std::unique_ptr<XYControllerNodeParameterAttachment> attachment;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYControllerNodeAttachment)
};

#endif //SUCCULENT_XYCONTROLLERATTACHMENT_H
