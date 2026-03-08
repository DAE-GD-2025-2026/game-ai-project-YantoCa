#include "SteeringBehaviors.h"
#include "GameAIProg/Movement/SteeringBehaviors/SteeringAgent.h"

//SEEK
//*******
#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>

#include "DrawDebugHelpers.h"

// SEEK
SteeringOutput Seek::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	steering.LinearVelocity = Target.Position - Agent.GetPosition();

	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugLine(
			Agent.GetWorld(),
			FVector(Agent.GetPosition(), 0),
			FVector(Target.Position, 0),
			FColor::Green,
			false,
			-1,
			0,
			2.f
		);
	}

	steering.IsValid = true;
	return steering;
}

// Flee
SteeringOutput Flee::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};
	FVector2D direction = Agent.GetPosition() - Target.Position;
	// TODO maybe limit how far it runs and not just infinitly
	
	if (Agent.GetDebugRenderingEnabled())
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Target.Position, 0), FColor::Green, false, -1, 0, 2.f);

	
	return steering;
}
//Arrive
SteeringOutput Arrive::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};
	
	if (DefaultSpeed < 0.f) 
		DefaultSpeed = Agent.GetMaxLinearSpeed();
	
	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	float Distance = ToTarget.Length();
	
	float AgentSpeed;
	
	/// Radius around player
	if (Distance > SlowRadius)
	{ // Outside Radius
		AgentSpeed = DefaultSpeed;
	}else if (Distance <= TargetRadius)
	{ // Inside TargetRadius
		AgentSpeed = 0.f;
	}else
	{
		float Ratio = (Distance - TargetRadius) / (SlowRadius - TargetRadius);
		AgentSpeed = DefaultSpeed * Ratio;
	}
	
	// Set agents new speed
	Agent.SetMaxLinearSpeed(AgentSpeed);
	
	steering.LinearVelocity = ToTarget;
	
	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* world = Agent.GetWorld();
		FVector Center = FVector(Target.Position, 0.f);
	
		DrawDebugCircle(world, Center, SlowRadius, 32, FColor::Green, false, -1, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
		DrawDebugCircle(world, Center, TargetRadius, 32, FColor::Red, false, -1, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
		
		DrawDebugLine(world, FVector(Agent.GetPosition(),0), Center, FColor::Blue, false, -1, 0, 2.f );
	}
	
	return steering;
}

// FACE
SteeringOutput Face::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	float CurrentAngle = Agent.GetRotation();
	float TargetAngle = FMath::RadiansToDegrees(FMath::Atan2(ToTarget.Y, ToTarget.X));
	
	// Normalize angle
	float AngleDiff = FMath::UnwindDegrees(TargetAngle - CurrentAngle);
	
	steering.AngularVelocity = AngleDiff;
	
	if (Agent.GetDebugRenderingEnabled())
	{
		DrawDebugLine(Agent.GetWorld(), FVector(Agent.GetPosition(), 0), FVector(Agent.GetPosition() + ToTarget.GetSafeNormal() * 100.f,0), FColor::Magenta, false, -1, 0, 2.f);
	}
	
	return steering;
}

// Pursuit 
SteeringOutput Pursuit::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};
	
	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	float Distance = ToTarget.Length();
	float Speed = Agent.GetMaxLinearSpeed();
	float PredictionTime;
	
	Speed <= (Distance / MaxPredictionTime) ? PredictionTime =  MaxPredictionTime : PredictionTime = Distance / Speed;
	
	FVector2D PredictedPosition = Target.Position + (Target.LinearVelocity * PredictionTime);
	
	steering.LinearVelocity = PredictedPosition - Agent.GetPosition();
	
	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* world = Agent.GetWorld();
		DrawDebugCircle(world, FVector(PredictedPosition, 0), 25.f, 32, FColor::Red, false, -1, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
		
		DrawDebugLine(world, FVector(Target.Position,0), FVector(PredictedPosition,0), FColor::Blue, false, -1, 0, 1.f);
	}
	
	return steering;
}

// Evade
SteeringOutput Evade::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	SteeringOutput steering{};

	FVector2D ToTarget = Target.Position - Agent.GetPosition();
	float Distance = ToTarget.Length();
	float Speed = Agent.GetMaxLinearSpeed();
	float PredictionTime;

	Speed <= (Distance / MaxPredictionTime) ? PredictionTime =  MaxPredictionTime : PredictionTime = Distance / Speed;

	FVector2D PredictedPosition = Target.Position + (Target.LinearVelocity * PredictionTime);

	steering.LinearVelocity = Agent.GetPosition() - PredictedPosition;

	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* world = Agent.GetWorld();
		DrawDebugCircle(world, FVector(PredictedPosition, 0), 25.f, 32, FColor::Red, false, -1, 0, 2.f, FVector(0,1,0), FVector(1,0,0));
	
		DrawDebugLine(world, FVector(Target.Position,0), FVector(PredictedPosition,0), FColor::Blue, false, -1, 0, 1.f);
	}

	return steering;
}

SteeringOutput Wander::CalculateSteering(float DeltaT, ASteeringAgent& Agent)
{
	float Jitter = FMath::FRandRange(-MaxAngleChange, MaxAngleChange);
	WanderAngle += Jitter;
	
	float AgentRotation = Agent.GetRotation();
	FVector2D Forward;
	Forward.X = FMath::Cos(AgentRotation * (PI / 180.f));
	Forward.Y = FMath::Sin(AgentRotation * (PI / 180.f));
	
	FVector2D Center = Agent.GetPosition() + Forward * Offset;
	
	float WanderRad = FMath::DegreesToRadians(WanderAngle);
	
	FVector2D Displacement;
	Displacement.X = FMath::Cos(WanderRad) * Radius; //
	Displacement.Y = FMath::Sin(WanderRad) * Radius;
	
	FVector2D WanderTarget = Center + Displacement;
	Target.Position = WanderTarget;
	
	if (Agent.GetDebugRenderingEnabled())
	{
		UWorld* world = Agent.GetWorld();
		
		DrawDebugCircle(world,FVector(Center,0), Radius,32,FColor::Green,false,-1,0,2,FVector(1,0,0),FVector(0,1,0),false);
		DrawDebugSphere(world,FVector(WanderTarget,0),20,8,FColor::Red,false);
	}
	
	return Seek::CalculateSteering(DeltaT, Agent);
}