//
// Created by tar on 10/11/22.
//

#ifndef SUCCULENT_XYCONTROLLER_H
#define SUCCULENT_XYCONTROLLER_H

#include <juce_gui_basics/juce_gui_basics.h>

class XYController : public juce::Component {
public:
    explicit XYController(uint maxNumNodes = 0);

    void paint(juce::Graphics &g) override;

    void mouseDown(const juce::MouseEvent &event) override;

    void resized() override;

    /**
     * Exposing this publicly is probably a bad idea, but it provides an easy
     * way of rolling back the creation of a node if some important condition
     * isn't satisfied.
     * @param index
     */
    void removeNode(uint index);

    std::function<void(uint nodeIndex, juce::Point<float>)> onValueChange;

    std::function<void(uint nodeIndex)> onAddNode;

    std::function<void(uint nodeIndex)> onRemoveNode;

protected:
    class Node : public Component {
    private:
        struct Value {
            float x, y;
        };
    public:
        explicit Node(Value val, uint idx = 0);

        void paint(juce::Graphics &g) override;

        void mouseDown(const juce::MouseEvent &event) override;

        void mouseDrag(const juce::MouseEvent &event) override;

        std::function<void(Node *)> onMove;

        std::function<void(Node *)> onRemove;
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Node)

        void setBounds();

        static constexpr float NODE_WIDTH{50.f};

        uint index{0};
        Value value{};

        friend class XYController;

        static float clamp(float val, float min, float max);
    };

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (XYController)

    std::unordered_map<uint, std::unique_ptr<Node>> nodes;

    void createNode(juce::Point<float> value);

    void normalisePosition(juce::Point<float> &position);

    void removeNode(Node *node);

    uint getNextAvailableNodeID();

    void removeAllNodes();

    std::unordered_map<uint, std::unique_ptr<XYController::Node>>::iterator
    removeNodeByIterator(std::unordered_map<uint, std::unique_ptr<XYController::Node>>::iterator it);

    uint maxNodes;

    bool canAddNode();
};


#endif //SUCCULENT_XYCONTROLLER_H
