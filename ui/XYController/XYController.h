//
// Created by tar on 10/11/22.
//

#ifndef SUCCULENT_XYCONTROLLER_H
#define SUCCULENT_XYCONTROLLER_H

#include <juce_gui_basics/juce_gui_basics.h>

/*
 * TODO: document, and allow creation of non-removable nodes.
 */
class XYController : public juce::Component {
protected:
public:
    explicit XYController(int maxNumNodes = -1);

    void paint(juce::Graphics &g) override;

    void mouseDown(const juce::MouseEvent &event) override;

    void resized() override;

    void addNode(juce::Point<float> normalisedValue = {0.f, 0.f});

    /**
     * Exposing this publicly is probably a bad idea, but it provides an easy
     * way of rolling back the creation of a node if some important condition
     * isn't satisfied.
     * @param index
     */
    void removeNode(uint index);

    std::function<void(uint nodeIndex, juce::Point<float>)> onValueChange;

    std::function<void(uint nodeIndex)> onRemoveNode;

    class Node : public juce::Component, public juce::AsyncUpdater {
    public:
        Node(XYController &controller, juce::Point<float> val, uint idx = 0);

        void paint(juce::Graphics &g) override;

        void handleAsyncUpdate() override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseUp(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        void setValueX(float newValue, juce::NotificationType notification);

        void setValueY(float newValue, juce::NotificationType notification);

        juce::Point<float> getValue();

        std::function<void()> onValueChange;

        class Listener {
        public:
            virtual ~Listener() = default;

            //==============================================================================
            /** Called when the node's value is changed.

                This may be caused by dragging it or by a call to Node::setValue().

                You can find out the new value using Node::getValue().

                @see Slider::valueChanged
            */
            virtual void nodeValueChanged(Node *node) = 0;

            //==============================================================================
            /** Called when the slider is about to be dragged.

                This is called when a drag begins, then it's followed by multiple calls
                to nodeValueChanged(), and then nodeDragEnded() is called after the
                user lets go.

                @see sliderDragEnded, Slider::startedDragging
            */
            virtual void nodeDragStarted(Node *) {}

            /** Called after a drag operation has finished.
                @see sliderDragStarted, Slider::stoppedDragging
            */
            virtual void nodeDragEnded(Node *) {}
        };

        class JUCE_API  ScopedDragNotification {
        public:
            explicit ScopedDragNotification(Node &);

            ~ScopedDragNotification();

        private:
            Node &nodeBeingDragged;

            JUCE_DECLARE_NON_MOVEABLE (ScopedDragNotification)

            JUCE_DECLARE_NON_COPYABLE (ScopedDragNotification)
        };

        /** Adds a listener to be called when this controller's value changes. */
        void addListener(Listener *listener);

        /** Removes a previously-registered listener. */
        void removeListener(Listener *listener);

        void setBounds();

        void valueChanged() {};

        uint getIndex() const;

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Node)

        void triggerChangeMessage(juce::NotificationType notification);

        static constexpr float NODE_WIDTH{44.f}, NODE_WIDTH_HALF{NODE_WIDTH / 2.f};

        XYController &owner;
        uint index{0};
        juce::Point<float> value{};

        friend class XYController;

        static float clamp(float val, float min, float max);

        juce::ListenerList<Listener> listeners;


        void sendDragStart();

        void sendDragEnd();

        std::unique_ptr<ScopedDragNotification> currentDrag;
    };

    std::function<void(XYController::Node &)> onAddNode;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYController)

    std::unordered_map<uint, std::unique_ptr<Node>> nodes;

    void createNode(juce::Point<float> value);

    void normalisePosition(juce::Point<float> &position);

    void removeNode(Node *node);

    uint getNextAvailableNodeID();

    void removeAllNodes();

    std::unordered_map<uint, std::unique_ptr<Node>>::iterator
    removeNodeByIterator(std::unordered_map<uint, std::unique_ptr<Node>>::iterator it);

    int maxNodes;

    bool canAddNode();
};


#endif //SUCCULENT_XYCONTROLLER_H
