// ConditionNode.hpp
#pragma once

#include "DecisionNode.hpp"
#include <functional>

// A binary decision node: if predicate(state) is true → onTrue, else → onFalse.
class ConditionNode : public DecisionNode {
public:
    using Predicate = std::function<bool(const State&)>;

    ConditionNode(Predicate predicate,
                  DecisionNode* onTrue,
                  DecisionNode* onFalse);

    DecisionNode* evaluate(const State& state) override;

private:
    Predicate       predicate_;
    DecisionNode*   trueBranch_;
    DecisionNode*   falseBranch_;
};
