// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Engine/GameInstance.h"
#include "Instance.generated.h"

/**
 * 
 */
UCLASS()
class ESSENTIALS_API UInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UInstance();
	virtual void Init() override;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Client: General", meta = (GetOptions = "GameTypeArray"))
	FString GameType = "Singleplayer";

	UFUNCTION(BlueprintCallable, Category = "Client: General")
	TArray<FString> GameTypeArray() const
	{
		return
		{
			"Singleplayer",
			"Multiplayer",
			"Both"
		};
	};

public:
	UFUNCTION(Exec, BlueprintCallable, Category = "Client: Login")
	void Login();
	void OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client: Sessions")
	void CreateSession();
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client: Sessions")
	void DestroySession();
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client: Sessions")
	void FindSession();
	void OnFindSessionComplete(bool bWasSuccessful);
	TSharedPtr<FOnlineSessionSearch> SearchSettings;
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client: Friends")
	void GetAllFriends();
	void OnGetAllFriendsComplete(int32 LocalUserNumber, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr);

	UFUNCTION(Exec, BlueprintCallable, Category = "Client: UI")
	void ShowInviteUI();
	UFUNCTION(Exec, BlueprintCallable, Category = "Client: UI")
	void ShowFriendsUI();
	UFUNCTION(Exec, BlueprintCallable, Category = "Client: UI")
	void ShowAchievementsUI();

protected:
	class IOnlineSubsystem* OnlineSubsystem;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client: Info")
	FString UserDisplayName;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client: Info")
	FString FriendDisplayName;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client: Status")
	bool bLoginStatus; // True if logged in to EOS, false if not.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Client: Status")
	bool bSessionStatus; // True if the session is created, false if not

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	bool bToggleCrouch = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	bool bToggleSprint = false;
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Input")
	bool bToggleInteract = true;

public:
	// Aleph-only bools
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Client: Rules")
	bool bAllowDebug; //Allows debugging
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Client: Rules")
	bool bAllowExtraMov = true; //Allows advanced movement
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Client: Rules")
	bool bAllowAbilities = true; // Allows the use of all abilities
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Client: Rules")
	bool bAllowExchange = true; //Allows the use of the Heal ability
	
};
