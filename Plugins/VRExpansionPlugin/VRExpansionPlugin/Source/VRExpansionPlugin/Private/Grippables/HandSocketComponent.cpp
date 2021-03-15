// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

#include "Grippables/HandSocketComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/UnrealNetwork.h"

  //=============================================================================
UHandSocketComponent::UHandSocketComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bReplicateMovement = false;
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	//this->bReplicates = true;

	bRepGameplayTags = true;

#if WITH_EDITORONLY_DATA
	bTickedPose = false;
#endif


	HandRelativePlacement = FTransform::Identity;
	OverrideDistance = 0.0f;
	SlotPrefix = FName("VRGripP");
	HandTargetAnimation = nullptr;
	bShowVisualizationMesh = true;
}

UAnimSequence* UHandSocketComponent::GetTargetAnimation()
{
	return HandTargetAnimation;
}

FTransform UHandSocketComponent::GetHandRelativePlacement()
{
	return HandRelativePlacement;
}

void UHandSocketComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	AActor* MyOwner = GetOwner();
	if (bShowVisualizationMesh && (MyOwner != nullptr) && !IsRunningCommandlet())
	{
		if (HandVisualizerComponent == nullptr)
		{
			HandVisualizerComponent = NewObject<USkeletalMeshComponent>(MyOwner, NAME_None, RF_Transactional | RF_TextExportTransient);
			if (HandVisualizerComponent)
			{
				HandVisualizerComponent->SetupAttachment(this);
				HandVisualizerComponent->SetIsVisualizationComponent(true);
				HandVisualizerComponent->SetCollisionProfileName(UCollisionProfile::NoCollision_ProfileName);
				HandVisualizerComponent->CastShadow = false;
				HandVisualizerComponent->CreationMethod = CreationMethod;
				//HandVisualizerComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
				HandVisualizerComponent->SetComponentTickEnabled(false);
				HandVisualizerComponent->SetHiddenInGame(true);
				HandVisualizerComponent->RegisterComponentWithWorld(GetWorld());

				if (VisualizationMesh)
				{
					HandVisualizerComponent->SetSkeletalMesh(VisualizationMesh);
					if (HandPreviewMaterial)
					{
						HandVisualizerComponent->SetMaterial(0, HandPreviewMaterial);
					}
				}

				HandVisualizerComponent->SetRelativeTransform(HandRelativePlacement);

				if (HandTargetAnimation)
				{
					HandVisualizerComponent->SetAnimationMode(EAnimationMode::AnimationSingleNode);
					HandVisualizerComponent->PlayAnimation(HandTargetAnimation, false);
					HandVisualizerComponent->AnimationData.AnimToPlay = HandTargetAnimation;

					if (HandVisualizerComponent && !bTickedPose)
					{
						// Tick Pose first
						if (HandVisualizerComponent->ShouldTickPose() && HandVisualizerComponent->IsRegistered())
						{
							bTickedPose = true;
							HandVisualizerComponent->TickPose(1.0f, false);
							if (HandVisualizerComponent->MasterPoseComponent.IsValid())
							{
								HandVisualizerComponent->UpdateSlaveComponent();
							}
							else
							{
								HandVisualizerComponent->RefreshBoneTransforms(&HandVisualizerComponent->PrimaryComponentTick);
							}
						}
					}
				}
			}
		}
	}

#endif	// WITH_EDITORONLY_DATA

	Super::OnRegister();
}

#if WITH_EDITORONLY_DATA
void UHandSocketComponent::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UHandSocketComponent* This = CastChecked<UHandSocketComponent>(InThis);
	Collector.AddReferencedObject(This->HandVisualizerComponent);

	Super::AddReferencedObjects(InThis, Collector);
}

void UHandSocketComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);

	if (HandVisualizerComponent)
	{
		HandVisualizerComponent->DestroyComponent();
	}
}

#endif

void UHandSocketComponent::GetLifetimeReplicatedProps(TArray< class FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(UHandSocketComponent, bRepGameplayTags);
	DOREPLIFETIME(UHandSocketComponent, bReplicateMovement);
	DOREPLIFETIME_CONDITION(UHandSocketComponent, GameplayTags, COND_Custom);
}

void UHandSocketComponent::PreReplication(IRepChangedPropertyTracker & ChangedPropertyTracker)
{
	Super::PreReplication(ChangedPropertyTracker);

	// Don't replicate if set to not do it
	DOREPLIFETIME_ACTIVE_OVERRIDE(UHandSocketComponent, GameplayTags, bRepGameplayTags);

	DOREPLIFETIME_ACTIVE_OVERRIDE_PRIVATE_PROPERTY(USceneComponent, RelativeLocation, bReplicateMovement);
	DOREPLIFETIME_ACTIVE_OVERRIDE_PRIVATE_PROPERTY(USceneComponent, RelativeRotation, bReplicateMovement);
	DOREPLIFETIME_ACTIVE_OVERRIDE_PRIVATE_PROPERTY(USceneComponent, RelativeScale3D, bReplicateMovement);
}

//=============================================================================
UHandSocketComponent::~UHandSocketComponent()
{
}

#if WITH_EDITOR
void UHandSocketComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FProperty* PropertyThatChanged = PropertyChangedEvent.Property;

	if (PropertyThatChanged != nullptr)
	{
#if WITH_EDITORONLY_DATA
		if (PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UHandSocketComponent, HandTargetAnimation) ||
			PropertyThatChanged->GetFName() == GET_MEMBER_NAME_CHECKED(UHandSocketComponent, VisualizationMesh)
			)
		{
			if (HandVisualizerComponent)
			{
				HandVisualizerComponent->SetSkeletalMesh(VisualizationMesh);
				if (HandPreviewMaterial)
				{
					HandVisualizerComponent->SetMaterial(0, HandPreviewMaterial);
				}

				// make sure the animation skeleton matches the current skeletalmesh
				if (HandTargetAnimation != nullptr && HandVisualizerComponent->SkeletalMesh && HandTargetAnimation->GetSkeleton() == HandVisualizerComponent->SkeletalMesh->Skeleton)
				{
					HandVisualizerComponent->AnimationData.AnimToPlay = HandTargetAnimation;
					HandVisualizerComponent->PlayAnimation(HandTargetAnimation, false);
				}
			}
		}
#endif
	}
}
#endif

