// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify_AttackCheck.h"
#include "SCharacter.h"

void UAnimNotify_AttackCheck::Notify(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation)
{
	Super::Notify(MeshComp, Animation);
	ASCharacter *player = Cast<ASCharacter>(MeshComp->GetOwner());
	if (player)
		player->Attack();
}

FString UAnimNotify_AttackCheck::GetNotifyName_Implementation() const
{
	return L"AttackCheck";
}
