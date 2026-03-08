
#include "CombinedSteeringBehaviors.h"
#include <algorithm>
#include "../SteeringAgent.h"

BlendedSteering::BlendedSteering(const std::vector<WeightedBehavior>& WeightedBehaviors)
	:WeightedBehaviors(WeightedBehaviors)
{};

//****************
//BLENDED STEERING
SteeringOutput BlendedSteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput BlendedSteering = {};
	// TODO: Calculate the weighted average steeringbehavior
	BlendedSteering.IsValid = false; // Start invalid

	for (const auto& weightedBehavior : WeightedBehaviors)
	{
		// Skip invalid or zero-weight behaviors
		if (!weightedBehavior.pBehavior || weightedBehavior.Weight <= 0.f)
			continue;
		
		const SteeringOutput singleSteering = 
			weightedBehavior.pBehavior->CalculateSteering(DeltaT, Agent);
		
		// Ignore invalid results
		if (!singleSteering.IsValid)
			continue;
		
		// Accumulate weighted velocities
		BlendedSteering.LinearVelocity += 
			singleSteering.LinearVelocity * weightedBehavior.Weight;

		BlendedSteering.AngularVelocity += 
			singleSteering.AngularVelocity * weightedBehavior.Weight;

		BlendedSteering.IsValid = true;
	}
	
	
	// TODO: Add debug drawing
	if (Agent.GetDebugRenderingEnabled() && BlendedSteering.IsValid)
	{
		const FVector start = Agent.GetActorLocation();
		const FVector end = start +  FVector{BlendedSteering.LinearVelocity, 0.f} *  (Agent.GetMaxLinearSpeed() * DeltaT);
		
		DrawDebugDirectionalArrow(Agent.GetWorld(), start, end,30.f, FColor::Red, false, -1.f, 0, 2.f);
	}
	
	
	return BlendedSteering;
}

float* BlendedSteering::GetWeight(ISteeringBehavior* const SteeringBehavior)
{
	auto it = find_if(WeightedBehaviors.begin(),
		WeightedBehaviors.end(),
		[SteeringBehavior](const WeightedBehavior& Elem)
		{
			return Elem.pBehavior == SteeringBehavior;
		}
	);

	if(it!= WeightedBehaviors.end())
		return &it->Weight;
	
	return nullptr;
}

//*****************
//PRIORITY STEERING
SteeringOutput PrioritySteering::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput Steering = {};

	for (ISteeringBehavior* const pBehavior : m_PriorityBehaviors)
	{
		if (!pBehavior) continue;
		Steering = pBehavior->CalculateSteering(DeltaT, Agent);

		if (Steering.IsValid)
			return Steering;
	}

	//If non of the behavior return a valid output, last behavior is returned
	return Steering;
}