//
// Created by tar on 10/11/22.
//

#include "XYController.h"

XYController::XYController(int maxNumNodes) : maxNodes(maxNumNodes) {}

void XYController::paint(juce::Graphics &g) {
    Component::paint(g);
//    g.fillAll(juce::Colours::whitesmoke);

//    g.setColour(juce::Colours::lightgrey);
//    g.drawRect(getLocalBounds(), 1);

    g.fillAll(juce::Colour{0.f, 0.f, 0.f, .1f});

    // Nodes are automatically painted, as they are added as child components.
}

void XYController::resized() {
    for (auto &node: nodes) {
        node.second->setBounds();
    }
}

void XYController::mouseDown(const juce::MouseEvent &event) {
    if (event.mods.isPopupMenu()) {
        // Maybe add a node
        juce::PopupMenu m;
        if (event.originalComponent == this) {
            if (canAddNode()) { m.addItem(1, "Create node"); }
            if (!nodes.empty()) { m.addItem(2, "Remove all nodes"); }
            m.showMenuAsync(juce::PopupMenu::Options(), [this, event](int result) {
                if (result == 1) {
                    // Find position centered on the location of the click.
                    createNode(event.position);
                } else if (result == 2) {
                    removeAllNodes();
                }
            });
        }
    }
}

void XYController::createNode(juce::Point<float> position) {
    // Normalise the position to get the value.
    auto bounds{getBounds().toFloat()};
    juce::Point<float> value{position.x / bounds.getWidth(), 1 - position.y / bounds.getHeight()};

    auto key{getNextAvailableNodeID()};
//    DBG("XYController: Adding node with ID " << juce::String(key));
    auto it{nodes.insert(NodeMap::value_type(key, std::make_unique<Node>(*this, value, key)))};
    auto node{it.first->second.get()};

    addAndMakeVisible(node);
    node->setBounds();

    if (onValueChange != nullptr) {
        onValueChange(key, node->value);
    }

    if (onAddNode != nullptr) {
        onAddNode(*node);
    }

    // Repaint just the area where the new node resides.
    repaint(node->getBounds());
}

void XYController::normalisePosition(juce::Point<float> &position) {
    position.x /= static_cast<float>(getWidth());
    position.y = 1 - position.y / static_cast<float>(getHeight());
}

void XYController::removeNode(Node *const node) {
    // Silly. Maybe dispense with.
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if (it->second.get() == node) {
            removeNodeByIterator(it);
            return;
        }
    }
}

void XYController::removeAllNodes() {
    auto it{nodes.begin()};
    while (it != nodes.end()) {
        it = removeNodeByIterator(it);
    }
}

XYController::NodeMap::iterator XYController::removeNodeByIterator(XYController::NodeMap::iterator it) {
    auto index{it->first};
    it = nodes.erase(it);
    if (onRemoveNode != nullptr) {
        DBG("XYController: Removing node with ID " << juce::String(index));
        onRemoveNode(index);
    }
    return it;
}

uint XYController::getNextAvailableNodeID() {
    uint nextID{0};
    while (nodes.find(nextID) != nodes.end()) { nextID++; }
    return nextID;
}

void XYController::removeNode(uint index) {
    nodes.erase(index);
    if (onRemoveNode != nullptr) {
        DBG("XYController: Removing node with ID " << juce::String(index));
        onRemoveNode(index);
    }
}

bool XYController::canAddNode() {
    return maxNodes < 0 || static_cast<int>(nodes.size()) < maxNodes;
}

void XYController::addNode(juce::Point<float> normalisedValue) {
    auto b{getBounds().toFloat()};
    createNode({normalisedValue.x * b.getWidth(), (1.f - normalisedValue.y) * b.getHeight()});
}

XYController::Node::Node(XYController &controller, juce::Point<float> val, uint idx) :
        owner(controller),
        index(idx),
        value(val) {}

void XYController::Node::paint(juce::Graphics &g) {
    auto colour{
            juce::Colours::steelblue
                    .withRotatedHue(static_cast<float>(index) * 1 / juce::MathConstants<float>::twoPi)
                    .withSaturation(.5f)
                    .withAlpha(.75f)
    };
    g.setColour(colour);
    g.fillEllipse(getLocalBounds().toFloat());
    g.setColour(colour.darker(.25));
    g.drawEllipse(getLocalBounds().withSizeKeepingCentre(getWidth() - 2, getHeight() - 2).toFloat(), 2.f);
    g.setColour(juce::Colours::white);
    g.setFont(20);
    g.drawText(juce::String(index + 1), getLocalBounds(), juce::Justification::centred);
}

void XYController::Node::mouseDown(const juce::MouseEvent &event) {
    if (event.mods.isPopupMenu()) {
        // Try to remove this node.
        juce::PopupMenu m;
        m.addItem(1, "Remove node");
        // TODO: expose possibility of adding more menu items via a callback.
        m.showMenuAsync(juce::PopupMenu::Options(), [this, event](int result) {
            if (result == 1) {
                owner.removeNode(index); // Schedule removal?... Call onRemove()?
            }
        });
    } else {
        currentDrag.reset();

        currentDrag = std::make_unique<ScopedDragNotification>(*this);
//        mouseDrag(event);
    }
}

void XYController::Node::mouseDrag(const juce::MouseEvent &event) {
    if (event.mods.isLeftButtonDown()) {
        auto parent{getParentComponent()};
        auto parentBounds{parent->getBounds().toFloat()};

        auto parentBottomRight{juce::Point<float>{parentBounds.getWidth(), parentBounds.getHeight()}};

        auto newVal{event.getEventRelativeTo(parent).position / parentBottomRight};

        // Set node value.
        setValueX(clamp(newVal.x, 0., 1.), juce::sendNotificationSync);
        setValueY(clamp(1 - newVal.y, 0., 1.), juce::sendNotificationSync);

        setBounds();

        if (owner.onValueChange != nullptr) {
            owner.onValueChange(index, value);
        }
    }
}

void XYController::Node::mouseUp(const juce::MouseEvent &) {
//    if (sendChangeOnlyOnRelease && valueOnMouseDown != static_cast<double> (currentValue.getValue()))
//        triggerChangeMessage(sendNotificationAsync);

    currentDrag.reset();
}

// TODO: move to Utils
float XYController::Node::clamp(float val, float min, float max) {
    if (val >= max) {
        val = max;
    } else if (val <= min) {
        val = min;
    }
    return val;
}

void XYController::Node::handleAsyncUpdate() {
    cancelPendingUpdate();

    Component::BailOutChecker checker(this);
    listeners.callChecked(checker, [&](Node::Listener &l) {
        l.nodeValueChanged(this);
    });

    if (checker.shouldBailOut())
        return;

    if (onValueChange != nullptr)
        onValueChange();

    if (checker.shouldBailOut())
        return;

    if (auto *handler = getAccessibilityHandler())
        handler->notifyAccessibilityEvent(juce::AccessibilityEvent::valueChanged);
}

void XYController::Node::setBounds() {
    auto bounds{getParentComponent()->getBounds().toFloat()};
    Component::setBounds(
            static_cast<int>(roundf(bounds.getWidth() * value.x - NODE_WIDTH_HALF)),
            static_cast<int>(roundf((bounds.getHeight() - bounds.getHeight() * value.y) - NODE_WIDTH_HALF)),
            NODE_WIDTH,
            NODE_WIDTH
    );
}

void XYController::Node::setValueX(float newValue, juce::NotificationType notification) {
//    if (newValue != lastCurrentValue)
//    {
//        lastCurrentValue = newValue;
//
//        // Need to do this comparison because the Value will use equalsWithSameType to compare
//        // the new and old values, so will generate unwanted change events if the type changes.
//        // Cast to double before comparing, to prevent comparing as another type (e.g. String).
//        if (static_cast<double> (currentValue.getValue()) != newValue)
//            currentValue = newValue;
//    }

    value.x = newValue;

//    owner.repaint();

    triggerChangeMessage(notification);
}

void XYController::Node::triggerChangeMessage(juce::NotificationType notification) {
    if (notification != juce::dontSendNotification) {
        valueChanged();

        if (notification == juce::sendNotificationSync)
            handleAsyncUpdate();
        else
            triggerAsyncUpdate();
    }
}

void XYController::Node::setValueY(float newValue, juce::NotificationType notification) {
    value.y = newValue;

//    owner.repaint();

    triggerChangeMessage(notification);
}

void XYController::Node::addListener(XYController::Node::Listener *listener) {
    listeners.add(listener);
}

void XYController::Node::removeListener(XYController::Node::Listener *listener) {
    listeners.remove(listener);
}

juce::Point<float> XYController::Node::getValue() {
    return value;
}

uint XYController::Node::getIndex() const {
    return index;
}

void XYController::Node::sendDragStart() {
//    startedDragging();

    Component::BailOutChecker checker(this);
    listeners.callChecked(checker, [&](XYController::Node::Listener &l) { l.nodeDragStarted(this); });

    if (checker.shouldBailOut())
        return;

//    if (owner.onDragStart != nullptr)
//        owner.onDragStart();
}

void XYController::Node::sendDragEnd() {
//    stoppedDragging();
//    nodeBeingDragged = -1;

    Component::BailOutChecker checker(this);
    listeners.callChecked(checker, [&](XYController::Node::Listener &l) { l.nodeDragEnded(this); });

    if (checker.shouldBailOut())
        return;

//    if (owner.onDragEnd != nullptr)
//        owner.onDragEnd();
}

XYController::Node::ScopedDragNotification::ScopedDragNotification(Node &n)
        : nodeBeingDragged(n) {
    nodeBeingDragged.sendDragStart();
}

XYController::Node::ScopedDragNotification::~ScopedDragNotification() {
//    if (nodeBeingDragged.pimpl != nullptr)
    nodeBeingDragged.sendDragEnd();
}