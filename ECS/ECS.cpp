
#include <iostream>

#include <vld.h>

#include "Registry/EntityRegistry.h"
#include "Registry/TypeView.h"
#include "Entity/Entity.h"
#include "TestClasses/Transform.h"
#include "TestClasses/Render.h"

int main()
{
    EntityRegistry registry;
    
    for (int i{}; i < 16; ++i)
    	registry.CreateEntity();

    auto& transforms = registry.AddView<Transform>();
    auto& renders = registry.AddView<Render>();

    for (auto entity : registry.GetEntities())
        transforms.Add(entity);

    for (size_t i{}; i < 8; ++i)
        renders.Add(registry.GetEntities()[i]);

    auto& binding = registry.AddBinding<Transform, Render>();

    for (size_t i{8}; i < 16; ++i)
        renders.Add(registry.GetEntities()[i]);

    for (auto& transform : transforms)
    {
        std::cout << transform.position[0] << '\n';
    }

    for (auto& render : renders)
    {
        std::cout << render.location[0] << '\n';
    }

    for (size_t i{ }; i < 16; ++i)
    {
        auto refRender = renders.Get(0);
        std::cout << refRender->location[0] << '\n';
    }

    for (auto& transformsRender : binding)
    {
        auto render = transformsRender.Get<Render>();
        auto transform = transformsRender.Get<Transform>();
        std::cout << render->location[0] << '\n';
        std::cout << transform->position[0] << '\n';
    }

}

