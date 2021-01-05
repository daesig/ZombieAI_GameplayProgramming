# Goal Oriented Action Planner (GOAP)
## The project
The project consists of a world with zombies, houses and items. The agent only knows what is in his current field of view and has to explore the world to survive. 
Given: The world, zombies, items, inventory, navmesh and movement calculations (plugin.h, plugin.cpp)
* Requirements: Give back an Output_Steering struct that has: linearVelocity, angularVelocity, runMode (whether the agent should run and consume stamina to obtain more speed). 
* Goal of the project: Give the agent the desired steering actions to survive in this zombie infested world. 
* Research: Goal Oriented Action Planner

![Gameplay example](https://raw.githubusercontent.com/daesig/ZombieAI_GameplayProgramming/master/ZombieAI.gif)

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

A GOAP system consists of a GOAP-planner, GOAP-actions, worldstates and a Finite State Machine.

### GOAP-planner
The GOAP-planner plans out a path towards a certain goal. The nodes that connect towards the goal are called the GOAP-actions. Each action has **preconditions** and **effects**. In this implementation, we start with a goal we want to fulfill called GOAPSurvive. To survive, the agent requires enough health and food and always wants to keep on moving. We can make 3 **preconditions** out of this: HasEnoughHealth, HasEnoughFood, HasMovementGoal

The planner will now look for actions that fillful these **preconditions** as **effects**. It's the planner's task to keep looking for **preconditions** and **effects** until all the **preconditions** have been satisfied. A good algorithm for this could be the AStar search algorithm. I started of with a base implementation of AStar and altered it to my needs.

The purpose of the planner is to return a list of actions we need to perform in a given order. This is where the FSM comes in.

### GOAP-action
We have aleady mentioned one GOAP-action called GOAPSurvive. Other examples of actions would be GOAPConsumeMedkit, GOAPSearchForMedkit...
Actions require preconditions and effects. They also need to be able to perform.

GOAPConsume medkit would use a medkit in the Perform() function, apply it's effects to the worldstate with ApplyEffects() and let the Finite State Machine know that it's done with the IsDone() function.

If an action like GOAPOpenDoor requires the agent to be next to the door, the RequiresMovement() function will return true and set a movement goal as long as the agent has not arrived. After arriving, the Perform() function can be called like mentioned above.

It is also possible to implement a Plan() function which will check for anything dynamic in the world and return if it's able to perform or not. The GOAP-planner could then call this Plan() to find out which actions are able to run. Note that this may require a lot of resources and just checking for worldstates is less intensive.

### World states
A world state can have all sorts of data. I only stored booleans for simplification in this project. It stores states like: HasEnoughHealth, HasMedkit, HasWeapon. States can be either true or false. 

If the agent picks up a weapon, the state HasWeapon becomes true and the action with the precondition HasWeapon(true) is able to perform. When destroying the weapon, HasWeapon becomes false and the action cannot perform anymore.

We can then simply use IsStateMet(key, value) to check if we have a state with those values.

### Finite State Machine
An implementation of a FSM in for a GOAP usually consists of 3 states: Idlestate, GoTostate and the Performstate.
The idle state asks the GOAP-planner for a new plan and transitions to either the GoTostate or the Performstate once an action has been found. The action in this case is the first action that requires handling in the queue given by the planner.

![Finite State Machine](https://raw.githubusercontent.com/daesig/ZombieAI_GameplayProgramming/master/FSM.png)

The FSM will transition to the GoTostate as long as the action requires movement, otherwise it goes to the Performstate. The FSM can also go back to the GoTostate from the Performstate in case sudden movement is required again. This project required too much custom implementation inside of the movement so most of the movement is handled inside of the actions. It still remains useleful if an action requires simple movement. 

The Finite State Machine is being called upon every frame. This means that the Performstate will also get handled every single frame. The Performstate can last 1 frame or an infinite amount of frames depending on how long the Action needs to perform.

This is important because it means the agent may get stuck inside of 1 action if the agent takes too long to perform. This is why the Performstate returns true or false. False means something out of the ordinary occured. The FSM will go back to idle and ask the GOAP-planner to plan a new set of actions. One of the ways I solved this is by giving an action a certain timer. The action will return false once the timer is up and the GOAP-planner will recalculate it's course. This is to make the agent act more dynamic. 

The FSM will repeatedly check for transitions and also transition back into the Idlestate once the action's IsDone() function returns true.
This will make the FSM get the next action in the list and start performing this one. It will ask for a new plan once all actions have been completed.

## Resources
* [GOAP implementation F.E.A.R.](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&ved=2ahUKEwi57eHroIXuAhUKqaQKHW7vA7wQFjABegQIAhAC&url=https%3A%2F%2Falumni.media.mit.edu%2F~jorkin%2Fgdc2006_orkin_jeff_fear.pdf&usg=AOvVaw1A6V7mt2imaclwaXyjy_vs)
* [GOAP by Brent Owens](https://gamedevelopment.tutsplus.com/tutorials/goal-oriented-action-planning-for-a-smarter-ai--cms-20793)
* [GOAP by Vedant Chaudhari](https://medium.com/@vedantchaudhari/goal-oriented-action-planning-34035ed40d0b)
