// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Login.generated.h"

class UEditableTextBox;
class UButton;

UCLASS()
class BATTLELOYAL_API ALogin : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALogin();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "FunctionTest")
	void OnClickLogin(FText Email, FText Password) const;

};
