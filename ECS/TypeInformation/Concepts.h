#pragma once

class SystemBase;

/**
 * Concepts for TypeVews to add extra functionality to certain steps of the pipeline
 */

/** If the Serializable function given Component as value type exists*/
template <typename T>
concept Serializable = requires(std::ostream & stream, T val) { val.Serialize(stream); };

/** If the Deserializable function given Component as value type exists*/
template <typename T>
concept Deserializable = requires(std::istream & stream, T val) { val.Deserialize(stream); };

/** If the type is both Serializable and Deserializable*/
template <typename T>
concept Streamable = Serializable<T> && Deserializable<T>;

/** If a Class should be initialized it can be given the Initialize function which will be called when it is placed inside of the Type View*/
template <typename T>
concept Initializable = requires(class EntityRegistry* registry, T val) { val.Initialize(registry); };

/** If a function exists called SortCompare that takes (const T&, const T&) as parameters, it will automatically set it as the sorting algorithm*/
template <typename T>
concept Sortable = requires(T val0, T val1) { SortCompare(val0, val1); };

/**
 * Concepts to determine if a type has certain methods that will then automatically be used to create systems from them
 */

/** If a class contains a PreUpdate(float deltaTime) method*/
template <typename T>
concept PreUpdateable = requires(T val, float deltaTime) { val.PreUpdate(deltaTime); };

/** If a class contains a Update(float deltaTime) method*/
template <typename T>
concept Updateable = requires(T val, float deltaTime) { val.Update(deltaTime); };

/** If a class contains a LateUpdate(float deltaTime) method*/
template <typename T>
concept LateUpdateable = requires(T val, float deltaTime) { val.LateUpdate(deltaTime); };

/** If a class contains a Render() method*/
template <typename T>
concept Renderable = requires(T val) { val.Render(); };

/** If a class contains a LateRender() method*/
template <typename T>
concept LateRenderable = requires(T val) { val.LateRender(); };


/**
 * These concepts are to be used with the Updateable/Renderable concepts.
 * You can use them to specify the Update interval for the system
 */

/** If a class contains a public static floating points variable called PreUpdateInterval*/
template <typename T>
concept PreUpdateTimeInterval = std::is_floating_point_v<decltype(T::PreUpdateInterval)>;

/** If a class contains a public static floating points variable called UpdateInterval*/
template <typename T>
concept UpdateTimeInterval = std::is_floating_point_v<decltype(T::UpdateInterval)>;

/** If a class contains a public static floating points variable called LateUpdateInterval*/
template <typename T>
concept LateUpdateTimeInterval = std::is_floating_point_v<decltype(T::LateUpdateInterval)>;

/** If a class contains a public static floating points variable called RenderInterval*/
template <typename T>
concept RenderTimeInterval = std::is_floating_point_v<decltype(T::RenderInterval)>;

/** If a class contains a public static floating points variable called LateRenderInterval*/
template <typename T>
concept LateRenderTimeInterval = std::is_floating_point_v<decltype(T::LateRenderInterval)>;


/**
 * Concepts to check what kind of System a specific System is
 */

template <typename Class>
concept isBindingSystem = std::is_base_of_v<SystemBase, Class> && requires(Class sys) { sys.GetTypeBinding(); };

template <typename Class>
concept isViewSystem = std::is_base_of_v<SystemBase, Class> && requires(Class sys) { sys.GetTypeView(); };