# ECS

An Entity Component System (ECS) made by Connor De Meyer. An ECS is a software architecture used primarly for video games where an entities are comprised by components of data. This data is stored as closely to each other as possible for easy and fast reading and modification. This architecture heavily benefits from CPU memory caching which improves performance.

## Entity

An Entity consist of 2 elements:
 - entityId: either a 4 or 8 byte unsigned integer depending on the system architecture
 - Entity Registry reference: A Reference to the Entity Registry it is contained in

The entityId is the most important part of the Entity class. When interacting with the Entity Registry it is only neccesary to have the entityId to add/remove/get Components.

### Game Object

The Game Object class is a wrapper class for the Entity class. It allows for easy adding/getting/removing of Components and will remove itself from the registry upon destruction.

## Component

A Component is any class or struct that contains data. It does not have to inherit from any base class.
Member methods can be added to personalize functionality for various stages of the Components lifetime:



### References
## Entity Registry
### Type View
### Type Binding
## System
#### Sorting
## Reflection
## Serializing
