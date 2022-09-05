#pragma once
#include "../Components/Render.h"
#include "../Components/Transform.h"
#include "../Components/RenderModifiers.h"
#include "../Components/TransformModifiers.h"

inline RegisterDynamicSystem<Render, Transform> renderTransformTransfer(
	SystemParameters{ "RenderTransformUpdate", int32_t(ExecutionTime::LateUpdate) },
	[](Render& render, Transform& transform)
	{
		render.Transform = transform.transform;
	});

inline RegisterDynamicSystem<Render, RenderModifiers> renderModifiers(
	SystemParameters{ "RenderModifier" },
	[](Render& render, RenderModifiers& modifier)
	{
		render.Color.x = std::fmod(render.Color.x + modifier.deltaColor.x, 1.f);
		render.Color.y = std::fmod(render.Color.y + modifier.deltaColor.y, 1.f);
		render.Color.z = std::fmod(render.Color.z + modifier.deltaColor.z, 1.f);
		render.Color.w = std::fmod(render.Color.w + modifier.deltaColor.w, 1.f);
		render.Pivot.x = std::fmod(render.Pivot.x + modifier.deltaPivot.x + 2.f, 4.f) - 2.f;
		render.Pivot.y = std::fmod(render.Pivot.y + modifier.deltaPivot.y + 2.f, 4.f) - 2.f;
		render.Depth = std::fmod(render.Depth + modifier.deltaDepth,1.f);
	});

inline RegisterDynamicSystem<Transform, MoveScaleRotate> moveScaleRotate(
	SystemParameters{ "MoveScaleRotate" }, 
	[](Transform& transform, MoveScaleRotate& msr)
	{
		transform.Move(msr.deltaPos);
		transform.Scale(msr.deltaScale);
	});

inline RegisterDynamicSystem<Transform> posModular(
	SystemParameters{ "PositionModulo", int32_t(ExecutionTime::LateUpdate) - 1, 0.1f},
	[](Transform& transform)
	{
		transform.transform[2][0] = std::fmod(transform.transform[2][0] + 1280.f, 1280.f);
		transform.transform[2][1] = std::fmod(transform.transform[2][1] + 720.f, 720.f);
		transform.transform[1][1] = transform.transform[0][0] = std::abs(std::fmod(transform.transform[0][0] - 0.25f, 200.f) + 0.25f);
	});

//class RenderTransformUpdateSystem final : public BindingSystem<Render, Transform>
//{
//public:
//
//	RenderTransformUpdateSystem(const SystemParameters& parameters) : BindingSystem<Render, Transform>(parameters) {}
//	~RenderTransformUpdateSystem() override = default;
//
//public:
//
//	void Execute() override;
//};
