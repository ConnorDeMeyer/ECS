#pragma once
class GameComponent
{
public:

	GameComponent() = default;
	virtual ~GameComponent() = default;

	GameComponent(const GameComponent& other) = delete;
	GameComponent(GameComponent&& other) noexcept = default;
	GameComponent& operator=(const GameComponent& other) = delete;
	GameComponent& operator=(GameComponent&& other) noexcept = default;

public:

	/** Updates the component.*/
	virtual void Update(float) {}

	/** Gets called after all object get updated*/
	virtual void LateUpdate() {}

	/**
	* Gets called right after the constructor fires and after the user fields are initialized
	* Used to initialize and ling components that can not be done in the constructor.
	* Do not create more components or objects in this method, as those created will be saved to the game file
	*/
	virtual void Initialize() {}

	/**
	* Gets called when the game is started or when this component was created during gameplay.
	*/
	virtual void BeginPlay() {}

private:



};

