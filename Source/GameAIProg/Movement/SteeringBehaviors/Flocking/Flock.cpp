#include "Flock.h"
#include "FlockingSteeringBehaviors.h"
#include "Shared/ImGuiHelpers.h"


Flock::Flock(
	UWorld* pWorld,
	TSubclassOf<ASteeringAgent> AgentClass,
	int FlockSize,
	float WorldSize,
	ASteeringAgent* const pAgentToEvade,
	bool bTrimWorld)
	: pWorld{pWorld}
	, FlockSize{ FlockSize }
	, pAgentToEvade{pAgentToEvade}
{
	Agents.SetNum(FlockSize);
#ifdef GAMEAI_USE_SPACE_PARTITIONING

	// Create the partition grid
	pPartitionedSpace = std::make_unique<CellSpace>(
		pWorld,
		WorldSize,
		WorldSize,
		NrOfCellsX,
		NrOfCellsX,
		FlockSize);

	OldPositions.SetNum(FlockSize);

#else

	Neighbors.SetNum(FlockSize);

#endif	
		
	// Create behaviors
	pSeparationBehavior = std::make_unique<Separation>(this);
	pCohesionBehavior = std::make_unique<Cohesion>(this);
	pVelMatchBehavior = std::make_unique<VelocityMatch>(this);
	pSeekBehavior = std::make_unique<Seek>();
	pWanderBehavior = std::make_unique<Wander>();
	pEvadeBehavior = std::make_unique<Evade>();
	
	// Combine flocking behaviors
	pBlendedSteering = std::make_unique<BlendedSteering>(std::vector<BlendedSteering::WeightedBehavior>{
		{pSeparationBehavior.get(), 1.f},
		{pCohesionBehavior.get(), 0.2f},
		{pVelMatchBehavior.get(), 0.2f},
		{pWanderBehavior.get(), 0.1f},
		{pSeekBehavior.get(), 0.0f}
	});
	
	// Evade first, otherwise use flocking
	pPrioritySteering = std::make_unique<PrioritySteering>(std::vector<ISteeringBehavior*>{
		pEvadeBehavior.get(),
		pBlendedSteering.get()
	});
	
 // TODO: initialize the flock and the memory pool
	// Initialize flock
	for (int i = 0; i < FlockSize; ++i)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
		// Random spawn position
		FVector RandomPos = FVector(FMath::RandRange(-500.f, 500.f), FMath::RandRange(-500.f, 500.f), 0.f);
    
		Agents[i] = pWorld->SpawnActor<ASteeringAgent>(AgentClass, RandomPos, FRotator::ZeroRotator, SpawnParams);
    
		if (Agents[i])
		{
			Agents[i]->SetSteeringBehavior(pPrioritySteering.get());
			Agents[i]->SetDebugRenderingEnabled(false);
#ifdef GAMEAI_USE_SPACE_PARTITIONING
			pPartitionedSpace->AddAgent(*Agents[i]);
			OldPositions[i] = Agents[i]->GetPosition();
#endif		
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Flock: Failed to spawn agent!"));
		}
	}
}

Flock::~Flock()
{
 // TODO: Cleanup any additional data
	for (ASteeringAgent* pAgent : Agents)
	{
		if (pAgent && pAgent->IsValidLowLevel())
		{
			pAgent->Destroy();
		}
	}
	
	Agents.Empty();
#ifndef GAMEAI_USE_SPACE_PARTITIONING
	Neighbors.Empty();
#endif
}

void Flock::Tick(float DeltaTime)
{
 // TODO: update the flock
 // TODO: for every agent:
  // TODO: register the neighbors for this agent (-> fill the memory pool with the neighbors for the currently evaluated agent)
  // TODO: update the agent (-> the steeringbehaviors use the neighbors in the memory pool)
  // TODO: trim the agent to the world
	
	if (pAgentToEvade && pAgentToEvade->IsValidLowLevel())
	{
		pEvadeBehavior->SetTarget(FSteeringParams{pAgentToEvade->GetPosition()});
	}else if (pEvadeBehavior)
	{
		pEvadeBehavior->SetTarget(FSteeringParams{FVector2D(999.f, 999.f)});
	}
		
	for (int i = 0; i < FlockSize; ++i)
	{
		ASteeringAgent* agent = Agents[i];
#ifdef GAMEAI_USE_SPACE_PARTITIONING
		pPartitionedSpace->UpdateAgentCell(*agent, OldPositions[i]);
		OldPositions[i] = agent->GetPosition();		
		pPartitionedSpace->RegisterNeighbors(*agent, NeighborhoodRadius);
#else
		RegisterNeighbors(agent);
#endif

		//agent->Tick(DeltaTime);
	}
}

void Flock::RenderDebug()
{
 // TODO: Render all the agents in the flock
#ifdef GAMEAI_USE_SPACE_PARTITIONING

	if (DebugRenderPartitions)
		pPartitionedSpace->RenderCells();

#endif
if (DebugRenderNeighborhood)
	RenderNeighborhood();
}

void Flock::ImGuiRender(ImVec2 const& WindowPos, ImVec2 const& WindowSize)
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		bool bWindowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Gameplay Programming", &bWindowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("LMB: place target");
		ImGui::Text("RMB: move cam.");
		ImGui::Text("Scrollwheel: zoom cam.");
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Text("Flocking");
		ImGui::Spacing();

  // TODO: implement ImGUI checkboxes for debug rendering here
		if (ImGui::Checkbox("Debug Render Steering", &DebugRenderSteering))
		{
			for (ASteeringAgent* pAgent : Agents)
			{
				if (pAgent) pAgent->SetDebugRenderingEnabled(DebugRenderSteering);
			}
		}
		ImGui::Checkbox("Debug Render Neighborhood", &DebugRenderNeighborhood);
		ImGui::Checkbox("Debug Render Partitions", &DebugRenderPartitions);

		ImGui::Spacing();		
		
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

  // TODO: implement ImGUI sliders for steering behavior weights here
		if (pBlendedSteering)
		{
			auto& weightedBehaviors = pBlendedSteering->GetWeightedBehaviorsRef();

			if (weightedBehaviors.size() >= 5)
			{
				// Directly reference the weights
				ImGui::SliderFloat("Separation", &weightedBehaviors[0].Weight, 0.f, 1.f, "%.2f");
				ImGui::SliderFloat("Cohesion", &weightedBehaviors[1].Weight, 0.f, 1.f, "%.2f");
				ImGui::SliderFloat("Alignment", &weightedBehaviors[2].Weight, 0.f, 1.f, "%.2f");
				ImGui::SliderFloat("Wander", &weightedBehaviors[3].Weight, 0.f, 1.f, "%.2f");
				ImGui::SliderFloat("Seek", &weightedBehaviors[4].Weight, 0.f, 1.f, "%.2f");
			}
		}
		//End
		ImGui::End();
	}
#pragma endregion
#endif
}

void Flock::RenderNeighborhood()
{
 // TODO: Debugrender the neighbors for the first agent in the flock
	if (!DebugRenderNeighborhood || Agents.Num() == 0) return;
    
	// just the first agent because I still wanna see
	ASteeringAgent* currentAgent = Agents[0];
	Agents[0]->SetDebugRenderingEnabled(true);
	
	FVector center3D = FVector(currentAgent->GetPosition(), 0.f);
	
	DrawDebugCircle(pWorld, center3D, NeighborhoodRadius, 32, FColor::Green, false, -1.f, 0, 2.f, FVector(1,0,0), FVector(0,1,0), false);
	
	for (ASteeringAgent* other : Agents)
	{
		if (currentAgent == other) continue;

		float distSq = FVector2D::DistSquared(currentAgent->GetPosition(), other->GetPosition());
		if (distSq < (NeighborhoodRadius * NeighborhoodRadius))
		{
			DrawDebugLine(pWorld, center3D, FVector(other->GetPosition(), 0.f), FColor::Green, false, -1.f, 0, 1.5f);
		}
	}
}

#ifndef GAMEAI_USE_SPACE_PARTITIONING
void Flock::RegisterNeighbors(ASteeringAgent* const pAgent)
{
 // TODO: Implement
	NrOfNeighbors = 0; 
	FVector2D agentPos = pAgent->GetPosition();

	for (ASteeringAgent* other : Agents)
	{
		if (other == pAgent)
			continue;

		float dist = FVector2D::Distance(
			pAgent->GetPosition(),
			other->GetPosition());

		if (dist < NeighborhoodRadius)
		{
			Neighbors[NrOfNeighbors++] = other;
		}
	}
}
#endif

FVector2D Flock::GetAverageNeighborPos() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;
	
	FVector2D avgPosition = FVector2D::ZeroVector;

 // TODO: Implement
	for (int i = 0; i < NrOfNeighbors; ++i) 
	{
		avgPosition += GetNeighbors()[i]->GetPosition(); 
	}
    
	return avgPosition / static_cast<float>(GetNrOfNeighbors()); 
}

FVector2D Flock::GetAverageNeighborVelocity() const
{
	if (NrOfNeighbors == 0) return FVector2D::ZeroVector;
	FVector2D avgVelocity = FVector2D::ZeroVector;

 // TODO: Implement
	for (int i = 0; i < NrOfNeighbors; ++i) 
	{
		avgVelocity += GetNeighbors()[i]->GetLinearVelocity();
	}

	return avgVelocity / static_cast<float>(GetNrOfNeighbors());
}

void Flock::SetTarget_Seek(FSteeringParams const& Target)
{
 // TODO: Implement
	if (pSeekBehavior)
	{
		pSeekBehavior->SetTarget(Target);
	}
}

