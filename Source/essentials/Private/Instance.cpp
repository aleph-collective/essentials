// Fill out your copyright notice in the Description page of Project Settings.


#include "Instance.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

#include "Interfaces/OnlineIdentityInterface.h"
#include "Interfaces/OnlineExternalUIInterface.h"
#include "Interfaces/OnlineFriendsInterface.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

const FName Session_N = FName("CoreXSession");
const FString LobbyName = FString("CoreXLobby");
const FString LobbyMap = FString("sp_testing_01?listen");

UInstance::UInstance()
{
	bLoginStatus = false;
	bSessionStatus = false;
}

void UInstance::Init()
{
	Super::Init();
	OnlineSubsystem = IOnlineSubsystem::Get();

	if (GameType == "Multiplayer" || GameType == "Both") {
		Login();
	}
}

void UInstance::Login()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (OnlineSubsystem)
		{
			if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
			{
				FOnlineAccountCredentials Credentials;
				Credentials.Id = FString("localhost:6300");
				Credentials.Token = FString("Context_1");
				Credentials.Type = FString("developer");

				Identity->OnLoginCompleteDelegates->AddUObject(this, &UInstance::OnLoginComplete);
				Identity->Login(0, Credentials);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot login while the GameType is set to: Singleplayer"));
	}
}

void UInstance::OnLoginComplete(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error)
{
	if (bWasSuccessful == true)
	{
		UE_LOG(LogTemp, Warning, TEXT("Client successfully logged in."));
		bLoginStatus = bWasSuccessful;
		if (IOnlineIdentityPtr Identity = OnlineSubsystem->GetIdentityInterface())
		{
			UserDisplayName = Identity->GetUserAccount(UserId)->GetDisplayName();
			Identity->ClearOnLoginCompleteDelegates(0, this);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Client failed to login."))
	}
}

void UInstance::CreateSession()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				FOnlineSessionSettings SessionSettings;
				SessionSettings.bIsDedicated = false;
				SessionSettings.bShouldAdvertise = true;
				SessionSettings.bIsLANMatch = false;
				SessionSettings.NumPublicConnections = 5;
				SessionSettings.bAllowJoinInProgress = true;
				SessionSettings.bAllowJoinViaPresence = true;
				SessionSettings.bUsesPresence = true;
				SessionSettings.bUseLobbiesIfAvailable = true;
				SessionSettings.bUseLobbiesVoiceChatIfAvailable = true;
				SessionSettings.Set(SEARCH_KEYWORDS, LobbyName, EOnlineDataAdvertisementType::ViaOnlineService);

				SessionPtr->OnCreateSessionCompleteDelegates.AddUObject(this, &UInstance::OnCreateSessionComplete);
				SessionPtr->CreateSession(0, Session_N, SessionSettings);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot create a session while the GameType is set to: Singleplayer"));
	}
}

void UInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	bSessionStatus = bWasSuccessful;
	UE_LOG(LogTemp, Warning, TEXT("Session created with success. Client connected with success. Code: %d"), bSessionStatus);
	if (OnlineSubsystem)
	{
		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnCreateSessionCompleteDelegates(this);
			GetWorld()->ServerTravel(LobbyMap, false);
		}
	}
}

void UInstance::DestroySession()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SessionPtr->OnDestroySessionCompleteDelegates.AddUObject(this, &UInstance::OnDestroySessionComplete);
				SessionPtr->DestroySession(Session_N);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot destroy a session while the GameType is set to: Singleplayer"));
	}
}

void UInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
	bSessionStatus = bWasSuccessful;
	UE_LOG(LogTemp, Warning, TEXT("Session destroyed with success. Client disconnected with success. Code: %d"), bSessionStatus);
	if (OnlineSubsystem)
	{
		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnDestroySessionCompleteDelegates(this);
		}
	}
}

void UInstance::FindSession()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{
				SearchSettings = MakeShareable(new FOnlineSessionSearch());
				SearchSettings->MaxSearchResults = 5000;
				SearchSettings->QuerySettings.Set(SEARCH_KEYWORDS, LobbyName, EOnlineComparisonOp::Equals);
				SearchSettings->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);

				SessionPtr->OnFindSessionsCompleteDelegates.AddUObject(this, &UInstance::OnFindSessionComplete);
				SessionPtr->FindSessions(0, SearchSettings.ToSharedRef());
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot find a session while the GameType is set to: Singleplayer"));
	}
}

void UInstance::OnFindSessionComplete(bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("Session(s) found with success. Code: %d"), bWasSuccessful);
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Warning, TEXT("Lobbie(s) found: %d"), SearchSettings->SearchResults.Num());

		if (OnlineSubsystem)
		{
			if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
			{

				if (SearchSettings->SearchResults.Num())
				{
					SessionPtr->OnJoinSessionCompleteDelegates.AddUObject(this, &UInstance::OnJoinSessionComplete);
					SessionPtr->JoinSession(0, Session_N, SearchSettings->SearchResults[0]);
				}
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Session failed to be found. Code: %d"), bWasSuccessful);
	}

	if (OnlineSubsystem)
	{
		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			SessionPtr->ClearOnFindSessionsCompleteDelegates(this);
		}
	}
}

void UInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (OnlineSubsystem)
	{
		if (IOnlineSessionPtr SessionPtr = OnlineSubsystem->GetSessionInterface())
		{
			FString ConInfo = FString();
			SessionPtr->GetResolvedConnectString(Session_N, ConInfo);
			if (!ConInfo.IsEmpty())
			{
				if (APlayerController* PController = UGameplayStatics::GetPlayerController(GetWorld(), 0))
				{
					PController->ClientTravel(ConInfo, ETravelType::TRAVEL_Absolute);
				}
			}
		}
	}
}

void UInstance::GetAllFriends()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface())
			{
				FriendsPtr->ReadFriendsList(0, FString(""), FOnReadFriendsListComplete::CreateUObject(this, &UInstance::OnGetAllFriendsComplete));
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot get the friend list while the GameType is set to: Singleplayer"));
	}
}

void UInstance::OnGetAllFriendsComplete(int32 LocalUserNumber, bool bWasSuccessful, const FString& ListName, const FString& ErrorStr)
{
	UE_LOG(LogTemp, Warning, TEXT("Friends retrieved with success. Code: %d"), bWasSuccessful);
	if (bWasSuccessful && OnlineSubsystem)
	{
		if (IOnlineFriendsPtr FriendsPtr = OnlineSubsystem->GetFriendsInterface())
		{
			TArray<TSharedRef<FOnlineFriend>> FriendsList;
			if (FriendsPtr->GetFriendsList(0, ListName, FriendsList))
			{
				for (TSharedRef<FOnlineFriend> Friend : FriendsList)
				{
					FriendDisplayName = Friend.Get().GetRealName();
					UE_LOG(LogTemp, Warning, TEXT("%s"), *FriendDisplayName);
				}
			}
			else {
				UE_LOG(LogTemp, Warning, TEXT("Failed to retrieve friends."));
			}
		}
	}
}

void UInstance::ShowInviteUI()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowInviteUI(0);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot invite people while the GameType is set to: Singleplayer"));
	}
}

void UInstance::ShowFriendsUI()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowFriendsUI(0);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot get the friend list while the GameType is set to: Singleplayer"));
	}
}

void UInstance::ShowAchievementsUI()
{
	if (GameType == "Multiplayer" || GameType == "Both") {
		if (bLoginStatus && OnlineSubsystem)
		{
			if (IOnlineExternalUIPtr UIPtr = OnlineSubsystem->GetExternalUIInterface())
			{
				UIPtr->ShowAchievementsUI(0);
			}
		}
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("You cannot get the achievements list while the GameType is set to: Singleplayer"));
	}
}
