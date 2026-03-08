#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	if (pFlock->GetNrOfNeighbors() <= 0) 
		return SteeringOutput{};
	
	FVector2D targetPos = pFlock->GetAverageNeighborPos();
	SetTarget(FSteeringParams{ targetPos });
	
	return Seek::CalculateSteering(deltaT, pAgent);
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	int neighborCount = pFlock->GetNrOfNeighbors();
	if (neighborCount == 0)
		return steering;

	FVector2D separationForce = FVector2D::ZeroVector;

	for (int i = 0; i < neighborCount; ++i)
	{
		ASteeringAgent* neighbor = pFlock->GetNeighbors()[i];

		FVector2D toAgent = Agent.GetPosition() - neighbor->GetPosition();
		float distance = toAgent.Length();

		if (distance > 0.001f)
		{
			// direction away from neighbor
			FVector2D direction = toAgent / distance;

			// inverse distance weighting (1/x)
			separationForce += direction / distance;
		}
	}

	// average the force
	separationForce /= static_cast<float>(neighborCount);

	// scale to max speed
	steering.LinearVelocity = separationForce.GetSafeNormal() * Agent.GetMaxLinearSpeed();
	steering.IsValid = true;

	return steering;
}


//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, ASteeringAgent& pAgent)
{
	SteeringOutput steering{};
	if (pFlock->GetNrOfNeighbors() <= 0)
		return steering;
	
	steering.LinearVelocity = pFlock->GetAverageNeighborVelocity().GetClampedToMaxSize(pAgent.GetMaxLinearSpeed());
	steering.IsValid = true;
	
	return steering;
}

