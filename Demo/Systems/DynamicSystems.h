#pragma once
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/RenderModifiers.h"
#include "../Components/TransformModifiers.h"

class RenderTransformUpdateSystem final : public BindingSystem<Render, Transform>
{
public:

	RenderTransformUpdateSystem(const SystemParameters& parameters) : BindingSystem<Render, Transform>(parameters) {}
	~RenderTransformUpdateSystem() override = default;

public:

	void Execute() override;
};
