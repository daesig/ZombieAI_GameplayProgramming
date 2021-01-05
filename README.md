# Goal Oriented Action Planner (GOAP)

## What is a GOAP

A Goal Oriented Action Planner (abbreviated GOAP) is a technique commonly used to make a plan of actions.
It is most useful in area's like game development where there are 1 or more agents that require behaviors / actions.

1 of the more difficult tasks is telling your agent what to do at what moment. This is where a GOAP comes in.
Usually these types of problems can be solved with usage of a Finite State Machine (FSM), BehaviorTree (BT) or other common structures.

There are some problems with using a FSM or a BT so let's dive straight into the pro's and con's of using a GOAP!

## Pro's and Con's

For these examples we will compare a GOAP to a standalone FSM.

### Pro's
* Finite State Machines can quickly become complicated to manage, adding more actions requires more transitions. It becomes harder and harder the more actions you add to your Finite State Machine and transitions become almost incomprehensible. A GOAP system has actions completely independant from each other. This means that adding a new action is a quick and easy process.
* Not having to wory about transitions means that the GOAP will plan out it's own actions according to a search algorithm. This means your agent will be more suited to plan around the world state compared to a predetermined set of transitions.

### Con's
* A GOAP doesn't come standalone and requires a very basic implementation of a Finite State Machine of any other compatible management structure. The FSM requires at least 2-3 states which we will expand on later.
* A GOAP requires a good framework before it becomes worth the effort. This can take some time to implement so if your agent doesn't require a lot of actions, just using a FSM would be cheaper.

## Implementation

