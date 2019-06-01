// Fill out your copyright notice in the Description page of Project Settings.


#include "DetectableActor.h"


// Sets default values for this component's properties
UDetectableActor::UDetectableActor()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	Occluded = 0;
	ClassName = "Person";
	Pose = "Unknown";
	Difficult = 0;
}


// Called when the game starts
void UDetectableActor::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UDetectableActor::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// ...
}

