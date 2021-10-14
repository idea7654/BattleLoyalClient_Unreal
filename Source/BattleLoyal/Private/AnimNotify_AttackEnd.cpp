// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "AnimNotify_AttackEnd.h"

void UAnimNotify_AttackEnd::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::Notify(MeshComp, Animation);
	ASCharacter *player = Cast<ASCharacter>(MeshComp->GetOwner());
	if(player)
		player->AttackEnd();
}

FString UAnimNotify_AttackEnd::GetNotifyName_Implementation() const
{
	return L"AttackEnd";
}
