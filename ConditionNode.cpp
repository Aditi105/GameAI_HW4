// ConditionNode.cpp
#include "ConditionNode.hpp"

ConditionNode::ConditionNode(Predicate predicate,
                             DecisionNode* onTrue,
                             DecisionNode* onFalse)
  : predicate_(predicate)
  , trueBranch_(onTrue)
  , falseBranch_(onFalse)
{}

DecisionNode* ConditionNode::evaluate(const State& state) {
    return predicate_(state) ? trueBranch_ : falseBranch_;
}
