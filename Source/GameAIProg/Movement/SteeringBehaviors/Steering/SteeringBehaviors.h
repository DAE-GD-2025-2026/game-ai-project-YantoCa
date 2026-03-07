#pragma once

#include <Movement/SteeringBehaviors/SteeringHelpers.h>
#include "Kismet/KismetMathLibrary.h"

class ASteeringAgent;

// SteeringBehavior base, all steering behaviors should derive from this.
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	// Override to implement your own behavior
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent & Agent) = 0;

	void SetTarget(const FTargetData& NewTarget) { Target = NewTarget; }
	
	template<class T, std::enable_if_t<std::is_base_of_v<ISteeringBehavior, T>>* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	FTargetData Target;
};

// Your own SteeringBehaviors should follow here...

// SEEK
class Seek: public ISteeringBehavior{
public:
	Seek() = default;
	virtual ~Seek() override = default;

	// steering
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// FLEE
class Flee: public ISteeringBehavior
{
	public:
	Flee() = default;
	virtual ~Flee() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Arrive
class Arrive: public ISteeringBehavior
{
	public:
	Arrive() = default;
	virtual ~Arrive() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
	
protected:
	float DefaultSpeed = -1.f;
	float SlowRadius = 300.f;
	float TargetRadius = 100.f;
};

// Face
class Face: public ISteeringBehavior
{
	public: 
	Face() = default;
	virtual ~Face() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;
};

// Pursuit
class Pursuit: public ISteeringBehavior
{
	public:
	Pursuit() = default;
	virtual ~Pursuit() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

protected:
	float MaxPredictionTime = 2.f;
};

// Evade
class Evade: public ISteeringBehavior
{
	public:
	Evade() = default;
	virtual ~Evade() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

protected:
	float MaxPredictionTime = 2.f;
};

// Wander
class Wander: public Seek
{
	public:
	Wander() = default;
	virtual ~Wander() override = default;
	
	virtual SteeringOutput CalculateSteering(float DeltaT, ASteeringAgent& Agent) override;

protected:
	float Offset = 400.f;
	float Radius = 200.f;
	float MaxAngleChange = 45.f;
	float WanderAngle = 0.f;
};
