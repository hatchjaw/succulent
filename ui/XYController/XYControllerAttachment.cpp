//
// Created by tar on 16/04/23.
//

#include "XYControllerAttachment.h"

XYControllerNodeParameterAttachment::XYControllerNodeParameterAttachment(
        juce::RangedAudioParameter &parameterX,
        juce::RangedAudioParameter &parameterY,
        XYController::Node &n,
        juce::UndoManager *um
) : node(n),
    attachmentX(parameterX, [this](float f) { setValueX(f); }, um),
    attachmentY(parameterY, [this](float f) { setValueY(f); }, um) {
    sendInitialUpdate();
    node.valueChanged();
    node.addListener(this);
}

XYControllerNodeParameterAttachment::~XYControllerNodeParameterAttachment() {
    node.removeListener(this);
}


void XYControllerNodeParameterAttachment::nodeValueChanged(XYController::Node *) {
    if (!ignoreCallbacks) {
        auto val{node.getValue()};
        attachmentX.setValueAsPartOfGesture(val.x);
        attachmentY.setValueAsPartOfGesture(val.y);
    }
}

void XYControllerNodeParameterAttachment::nodeDragStarted(XYController::Node *) {
    attachmentX.beginGesture();
    attachmentY.beginGesture();
}

void XYControllerNodeParameterAttachment::nodeDragEnded(XYController::Node *) {
    attachmentX.endGesture();
    attachmentY.endGesture();
}

void XYControllerNodeParameterAttachment::setValueX(float newX) {
    const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
    node.setValueX(newX, juce::sendNotificationSync);
    node.setBounds(); // or repaint? or node.owner.repaint?
}

void XYControllerNodeParameterAttachment::setValueY(float newY) {
    const juce::ScopedValueSetter<bool> svs(ignoreCallbacks, true);
    node.setValueY(newY, juce::sendNotificationSync);
    node.setBounds(); // or repaint? or node.owner.repaint?
}

void XYControllerNodeParameterAttachment::sendInitialUpdate() {
    attachmentX.sendInitialUpdate();
    attachmentY.sendInitialUpdate();
}


XYControllerNodeAttachment::XYControllerNodeAttachment(juce::AudioProcessorValueTreeState &state,
                                                       const juce::String &parameterIDX,
                                                       const juce::String &parameterIDY,
                                                       XYController::Node &node) :
        attachment(std::make_unique<XYControllerNodeParameterAttachment>(*state.getParameter(parameterIDX),
                                                                         *state.getParameter(parameterIDY),
                                                                         node,
                                                                         state.undoManager)) {}