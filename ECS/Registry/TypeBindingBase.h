#pragma once

class TypeBindingBase
{
public:

	TypeBindingBase()										= default;

	TypeBindingBase(const TypeBindingBase&)					= default;
	TypeBindingBase(TypeBindingBase&&) noexcept				= default;
	TypeBindingBase& operator=(const TypeBindingBase&)		= default;
	TypeBindingBase& operator=(TypeBindingBase&&) noexcept	= default;
	virtual ~TypeBindingBase()								= default;

};