# ECS

An Entity Component System (ECS) made by Connor De Meyer. An ECS is a software architecture used primarly for video games where an entities are comprised by components of data. This data is stored as closely to each other as possible for easy and fast reading and modification. This architecture heavily benefits from CPU memory caching which improves performance.

## Entity

An Entity consist of 2 elements:
 - `entityId`: either a 4 or 8 byte unsigned integer depending on the system architecture
 - `EntityRegistry` reference: A Reference to the Entity Registry it is contained in

The entityId is the most important part of the Entity class. When interacting with the Entity Registry it is only neccesary to have the entityId to add/remove/get Components.

### Game Object

The Game Object class is a wrapper class for the Entity class. It allows for easy adding/getting/removing of Components and will remove itself from the registry upon destruction.

## Component

A Component is any class or struct that contains data. It does not have to inherit from any base class.
It should have its move constructor and operator= intact for it to be able to work.

Member methods can be added to personalize functionality for various stages of the Components lifetime:
 - `Serialize(std::ostream&)`: define custom Serialize logic for when the the Component has to be serialized to a stream.
 - `Deserialize(std::istream&)`: define custom logic for initializing Component reading from a stream.
 - `Initialize(EntityRegistry*)`: custom logic for when the Component is added to the registry.
 - `bool SortCompare(const Component&, const Component&)`: custom logic for when Components have to exist in a sorted state as much as possible.
 - Default System Methods: methods that will automatically be called without having to create a system for it.

### References

Because Data is stored in contigious array, they will sometimes have to move whenever the underlying array has to resize. Because of that you references/pointers to components are passed may be passed around by `Reference<Component>`. It contains a pointer to the class `ReferencePointer<Component>` that is static in memory and contains the pointer to the Component. Every time the underlying array gets resized, all the `ReferencePointer<Component>` get their pointers updated. These are similar to `std::shared_ptr` as they will always point to either a valid component or a nullptr in case the Component has been removed. They will also only be deleted from memory when no reference exists that points to it.

## Entity Registry

The `EntityRegistry` is the class that contains all the Entities, TypeViews, and Systems. It is a controller responsible for managing most resources and is the most important part of the ECS. It contains an `Update(float deltaTime)` method that should be called with the deltaTime whenever you want to update the Entities, Components and Systems.

### Type View

The `TypeView<Component>` class is the container for all the Components in a registry. It is respnsible for managing the `References` and resizing data whenever it needs to.

### Type Binding

`TypeBinding<Components...>` are similar to Type Views as they allow quickly accessing multiple Components that are all connected to the same Entity. Type bindings can be initialized with any amount of Components as long as the number is bigger than 1.
example:
```
auto typeBinding = TypeBinding<Transform, Physics, Render>{...}
```
will create a TypeBinding that contains references to the Components `Transform`, `Physics`, `Render`. Whenever an entity exists with these components it will be added to the TypeBinding. You may then access the references and call functions on them and/or transfer data between them.
It also contains a method to call function on them. For example:
```
ApplyFunctionOnAll<Transform, Physics, Render>([](Transform& transform, Physics& physics, Render& render) {...} );
```
Will call the lambda function on the mentioned Components of an Entity that contains all of them.

**Warning**: you may only have one TypeBinding with the specific Components. You may not have `TypeBinding<Transform, Render>` and `TypeBinding<Render, Transform>` at the same time. This also applies for Systems.

## System

A system is a process that modifies or acts on one or multiple components.
All Systems inherit from `SystemBase`, an abstract class that contains the following virtual methods that can be overriden in a custom System:
 - `Execute()`: Should contain the funcionality of the system, modifying or using the components.
 - `Initialize()`: Is called after construction when the TypeView or TypeBinding is set

When constructing a System it requires `SystemParameters` which constains the following information:
 - Name of the system
 - Execution time: an integer to specify when it should execute compared to other systems. Systems with lower Execution time will execute before Systems with higher Execution time.
 - Update interval: How long it takes between each Execute call. if 0 it will execute every frame.

Systems also have the method `GetDeltaTime()` which returns the deltaTime variable. This may be different for each system depending on the Update Interval.

**All Systems should have a constructor taking only `const SystemParameters&`**

### View System

System that contains a reference to a `TypeView` and used to act on a specific Component. You can get the Type View by using the `GetTypeView()` method.

### Binding System

System that contains a reference to a `TypeBinding` and used to act on multiple Components at the same time. You can get the Type Binding by using the `GetTypeBinding()` method. 

### Dynamic Systems

Dynamic Systems are either View Systems or Binding Systems that are easy to create as they only need a function taking the Component References as input parameters.

There also exist Dynamic Systems DT (deltaTime) taking the same type of functions but with a float parameter at the start.

### Sub Systems

Sub Systems are systems that act on Components that inherit from other Components. A system will get made calling the same function on the derived class. this way you can still get access to polymorphic function calling.

### Default Systems

These systems are created whenever a specific method exists in the Component. When any of the following methods exists a system will automatically be created to call them:
 - `PreUpdate(float deltaTime)`
 - `Update(float deltaTime)`
 - `LateUpdate(float deltaTime)`
 - `Render()`
 - `LateRender()`

You can also specify the Update interval by creating a static floatingPoint variable inside the class given the corresponding name:
 - `PreUpdateInterval`
 - `UpdateInterval`
 - `LateUpdateInterval`
 - `RenderInterval`
 - `LateRenderInterval`

example:
```
struct Transform
{
    static constexpr float UpdateInterval{ 0.16f }
    void Update(float deltaTime);
};
```

## Sorting

Whenever a Components have to exist in a sorted state you can specify a function by the signature of `bool SortCompare(const Component&, const Component&)`. If this function exists they Components will try to stay in a sorted state as much as possible.
You can query the sorting state of a TypeView using the function `GetDataFlag()` and the data flag id using `GetDataFlagId()`. The data flag Id changes whenever the data becomes dirty again. This way you can check in between the data being dirty if it changed again.

## Reflection



## Serializing
