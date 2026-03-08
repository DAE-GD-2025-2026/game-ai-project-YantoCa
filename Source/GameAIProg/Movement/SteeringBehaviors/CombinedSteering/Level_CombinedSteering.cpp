#include "Level_CombinedSteering.h"

#include "imgui.h"
#include <vector>


// Sets default values
ALevel_CombinedSteering::ALevel_CombinedSteering()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ALevel_CombinedSteering::BeginPlay()
{
	Super::BeginPlay();

	SeekAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector(-200.f, 0.f, 90.f), FRotator::ZeroRotator);
	WanderAgent = GetWorld()->SpawnActor<ASteeringAgent>(SteeringAgentClass, FVector(200.f, 0.f, 90.f), FRotator::ZeroRotator);
	
	if (!IsValid(SeekAgent) || !IsValid(WanderAgent))
		return;
	
	SeekBehaviour = MakeUnique<Seek>();
	EvadeBehaviour = MakeUnique<Evade>();
	WanderBehaviour = MakeUnique<Wander>();

	std::vector<BlendedSteering::WeightedBehavior> weightedBehaviors{
			{SeekBehaviour.Get(), 0.7f},
			{EvadeBehaviour.Get(), 1.0f}
	};

	BlendedBehaviour = MakeUnique<BlendedSteering>(weightedBehaviors);

	SeekAgent->SetSteeringBehavior(BlendedBehaviour.Get());
	WanderAgent->SetSteeringBehavior(WanderBehaviour.Get());

	SeekAgent->SetDebugRenderingEnabled(CanDebugRender);
	WanderAgent->SetDebugRenderingEnabled(CanDebugRender);
}

void ALevel_CombinedSteering::BeginDestroy()
{
	Super::BeginDestroy();

}

// Called every frame
void ALevel_CombinedSteering::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
#pragma region UI
	//UI
	{
		//Setup
		bool windowActive = true;
		ImGui::SetNextWindowPos(WindowPos);
		ImGui::SetNextWindowSize(WindowSize);
		ImGui::Begin("Game AI", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
	
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
		ImGui::Spacing();
	
		ImGui::Text("Flocking");
		ImGui::Spacing();
		ImGui::Spacing();
	
		if (ImGui::Checkbox("Debug Rendering", &CanDebugRender))
		{
   // TODO: Handle the debug rendering of your agents here :)
			
			
			
			
			
			
			
			
			
			
			
			
			
			
			//
		}
		ImGui::Checkbox("Trim World", &TrimWorld->bShouldTrimWorld);
		if (TrimWorld->bShouldTrimWorld)
		{
			ImGuiHelpers::ImGuiSliderFloatWithSetter("Trim Size",
				TrimWorld->GetTrimWorldSize(), 1000.f, 3000.f,
				[this](float InVal) { TrimWorld->SetTrimWorldSize(InVal); });
		}
		
		ImGui::Spacing();
		ImGui::Spacing();
		ImGui::Spacing();
	
		ImGui::Text("Behavior Weights");
		ImGui::Spacing();

		if (BlendedBehaviour)
		{
			auto& weightedBehaviors = BlendedBehaviour->GetWeightedBehaviorsRef();
			if (weightedBehaviors.size() >= 2)
			{
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
					weightedBehaviors[0].Weight, 0.f, 1.f,
					[this](float InVal) { BlendedBehaviour->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
				ImGuiHelpers::ImGuiSliderFloatWithSetter("Evade",
					weightedBehaviors[1].Weight, 0.f, 1.f,
					[this](float InVal) { BlendedBehaviour->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
			}
		}
				
		// ImGuiHelpers::ImGuiSliderFloatWithSetter("Seek",
		// 	pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight, 0.f, 1.f,
		// 	[this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[0].Weight = InVal; }, "%.2f");
		//
		// ImGuiHelpers::ImGuiSliderFloatWithSetter("Wander",
		// pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight, 0.f, 1.f,
		// [this](float InVal) { pBlendedSteering->GetWeightedBehaviorsRef()[1].Weight = InVal; }, "%.2f");
	
		//End
		ImGui::End();
	}
#pragma endregion
	
	// Combined Steering Update
 // TODO: implement handling mouse click input for seek
 // TODO: implement Make sure to also evade the wanderer

	if (!IsValid(SeekAgent) || !IsValid(WanderAgent) || !SeekBehaviour || !EvadeBehaviour)
		return;

	if (UseMouseTarget)
	{
		SeekBehaviour->SetTarget(MouseTarget);
	}

	FTargetData WandererTarget;
	WandererTarget.Position = WanderAgent->GetPosition();
	WandererTarget.Orientation = WanderAgent->GetRotation();
	WandererTarget.LinearVelocity = WanderAgent->GetLinearVelocity();
	WandererTarget.AngularVelocity = WanderAgent->GetAngularVelocity();
	EvadeBehaviour->SetTarget(WandererTarget);
}
