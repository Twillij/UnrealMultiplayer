#include "MultiplayerSubsystem.h"
#include "MultiplayerFunctionLibrary.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Online/OnlineSessionNames.h"

UMultiplayerSubsystem::UMultiplayerSubsystem()
{
	// Set the default session settings
	DefaultSessionSettings.NumPublicConnections = 2;
	DefaultSessionSettings.bIsDedicated = false;
	DefaultSessionSettings.bAllowInvites = true;
	DefaultSessionSettings.bAllowJoinInProgress = true;
	DefaultSessionSettings.bAllowJoinViaPresence = true;
	DefaultSessionSettings.bShouldAdvertise = true;
	DefaultSessionSettings.bUseLobbiesIfAvailable = true;
	DefaultSessionSettings.bUsesPresence = true;

	// Set the default session search settings
	DefaultSessionSearch.MaxSearchResults = 64;
	DefaultSessionSearch.QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
}

TArray<FMultiplayerSessionSearchResult> UMultiplayerSubsystem::GetLastSessionSearchResults() const
{
	TArray<FMultiplayerSessionSearchResult> Results;
	
	if (!LastSessionSearch)
		return Results;
	
	for (int i = 0; i < LastSessionSearch->SearchResults.Num(); ++i)
	{
		Results.Add(LastSessionSearch->SearchResults[i]);
	}
	return Results;
}

FMultiplayerSessionSearchResult UMultiplayerSubsystem::GetLastSessionSearchResultByHostId(const FUniqueNetIdRepl InHostId) const
{
	if (LastSessionSearch)
	{
		TArray<FOnlineSessionSearchResult> Results = LastSessionSearch->SearchResults;
	
		for (int i = 0; i < Results.Num(); ++i)
		{
			if (Results[i].Session.OwningUserId == InHostId)
			{
				return Results[i];
			}
		}
	}
	return FMultiplayerSessionSearchResult();
}

FMultiplayerSessionSearchResult UMultiplayerSubsystem::GetLastSessionSearchResultBySessionId(const FUniqueNetIdRepl InSessionId) const
{
	if (LastSessionSearch)
	{
		TArray<FOnlineSessionSearchResult> Results = LastSessionSearch->SearchResults;
	
		for (int i = 0; i < Results.Num(); ++i)
		{
			if (const TSharedPtr<FOnlineSessionInfo> SessionInfo = Results[i].Session.SessionInfo)
			{
				if (SessionInfo->GetSessionId() == InSessionId)
				{
					return Results[i];
				}
			}
		}
	}
	return FMultiplayerSessionSearchResult();
}

void UMultiplayerSubsystem::CreateSession(const FMultiplayerSessionSettings SessionSettings)
{
	if (!SessionInterface)
		return;
	
	// Get the player's unique net ID
	const FUniqueNetIdRepl PlayerNetId = UMultiplayerFunctionLibrary::GetLocalPlayerNetId(GetWorld());

	// Bind the delegate to the session interface and store the handle
	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	// Create a new session settings by combining the default and custom settings.
	const FOnlineSessionSettings NewSessionSettings = SessionSettings + DefaultSessionSettings;

	// Save the session info
	LastSessionInfo.HostPlayerId = PlayerNetId;
	LastSessionInfo.SessionSettings = SessionSettings;
	
	// Try to create a session using the specified params
	if (!SessionInterface->CreateSession(*PlayerNetId, SessionSettings.SessionName, NewSessionSettings))
	{
		OnCreateSessionCompleted(SessionSettings.SessionName, false);
	}
}

void UMultiplayerSubsystem::DestroySession(const FName SessionName)
{
	if (!SessionInterface)
		return;

	// Check if a session with that name exists on the server or if that session has any players remaining
	const FNamedOnlineSession* NamedSession = SessionInterface->GetNamedSession(LastSessionInfo.GetSessionName());
	if (!NamedSession || NamedSession->RegisteredPlayers.Num() > 0)
	{
		return;
	}
	
	// Bind the delegate to the session interface and store the handle
	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);

	// Try to destroy a session using the specified params
	if (!SessionInterface->DestroySession(SessionName))
	{
		OnDestroySessionCompleted(SessionName, false);
	}
}

void UMultiplayerSubsystem::FindSessions()
{
	if (!SessionInterface)
		return;
	
	// Get the player's unique net ID
	const FUniqueNetIdRepl PlayerNetId = UMultiplayerFunctionLibrary::GetLocalPlayerNetId(GetWorld());
	
	// Create and store a new session search
	LastSessionSearch = MakeShareable(new FOnlineSessionSearch(DefaultSessionSearch));

	// Bind the delegate to the session interface and store the handle
	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);
	
	// Try to find sessions using the specified params
	if (!SessionInterface->FindSessions(*PlayerNetId, LastSessionSearch.ToSharedRef()))
	{
		OnFindSessionsCompleted(false);
	}
}

void UMultiplayerSubsystem::JoinSession(const FMultiplayerSessionSearchResult& SessionSearchResult)
{
	if (!SessionInterface)
		return;

	// Save the session info and get the name
	LastSessionInfo = SessionSearchResult.Result;
	const FName SessionName = LastSessionInfo.GetSessionName();
	
	// Get the player's unique net ID
	const FUniqueNetIdRepl PlayerNetId = UMultiplayerFunctionLibrary::GetLocalPlayerNetId(GetWorld());

	// Bind the delegate to the session interface and store the handle
	JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
	
	// Try to join a session using the specified params
	if (!SessionInterface->JoinSession(*PlayerNetId, SessionName, SessionSearchResult.Result))
	{
		OnJoinSessionCompleted(SessionName, EOnJoinSessionCompleteResult::Type::UnknownError);
	}
}

void UMultiplayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (!OnlineSubsystem)
	{
		UE_LOG(LogMultiplayer, Error, TEXT("Failed to connect to a valid OSS."))
		return;
	}

	// Store the name of the subsystem
	OnlineSubsystemName = OnlineSubsystem->GetSubsystemName();
	UE_LOG(LogMultiplayer, Display, TEXT("Connected to OSS: %s"), *OnlineSubsystemName.ToString());

	// Store the session interface pointer
	SessionInterface = OnlineSubsystem->GetSessionInterface();
	if (!SessionInterface)
	{
		UE_LOG(LogMultiplayer, Error, TEXT("Failed to initialize OSS interface."))
		return;
	}

	// Assign the delegates
	CreateSessionCompleteDelegate = FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleted);
	DestroySessionCompleteDelegate = FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompleted);
	FindSessionsCompleteDelegate = FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompleted);
	JoinSessionCompleteDelegate = FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompleted);
	
	// Determine whether this is a LAN match or not
	DefaultSessionSettings.bIsLANMatch = OnlineSubsystemName == "NULL";
	DefaultSessionSearch.bIsLanQuery = OnlineSubsystemName == "NULL";
}

void UMultiplayerSubsystem::Deinitialize()
{
	Super::Deinitialize();
	DestroySession(LastSessionInfo.GetSessionName());
}

void UMultiplayerSubsystem::OnCreateSessionCompleted(const FName SessionName, const bool bWasSuccessful)
{
	if (!SessionInterface)
		return;
	
	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

	const FString LogString = FString::Printf(TEXT("Creating session \"%s\": %hs"), *SessionName.ToString(), bWasSuccessful ? "SUCCESS" : "FAILED");
	UE_LOG(LogMultiplayer, Display, TEXT("%s"), *LogString)
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, LogString);

	PostCreateSessionDelegate.Broadcast(LastSessionInfo);
}

void UMultiplayerSubsystem::OnDestroySessionCompleted(const FName SessionName, const bool bWasSuccessful)
{
	if (!SessionInterface)
		return;

	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	
	const FString LogString = FString::Printf(TEXT("Destroying session \"%s\": %hs"), *SessionName.ToString(), bWasSuccessful ? "SUCCESS" : "FAILED");
	UE_LOG(LogMultiplayer, Type::Warning, TEXT("%s"), *LogString);
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, LogString);

	PostDestroySessionDelegate.Broadcast(SessionName);
}

void UMultiplayerSubsystem::OnFindSessionsCompleted(const bool bWasSuccessful)
{
	if (!SessionInterface)
		return;
	
	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	
	if (!bWasSuccessful || !LastSessionSearch)
	{
		const FString FailLogString = "Finding session: FAILED";
		UE_LOG(LogMultiplayer, Display, TEXT("%s"), *FailLogString)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, FailLogString);
		return;
	}
	
	TArray<FOnlineSessionSearchResult> Results = LastSessionSearch->SearchResults;

	const FString CountLogString = FString::Printf(TEXT("Finding session: %i result"), Results.Num());
	UE_LOG(LogMultiplayer, Display, TEXT("%s"), *CountLogString)
	GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, CountLogString);
	
	for (int i = 0; i < Results.Num(); ++i)
	{
		FMultiplayerSessionInfo SessionInfo = Results[i];
		const FString ResultLogString = FString::Printf(TEXT("Found session %i: %s"), i + 1, *SessionInfo.GetSessionName().ToString());
		UE_LOG(LogMultiplayer, Display, TEXT("%s"), *ResultLogString)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, ResultLogString);
	}

	PostFindSessionDelegate.Broadcast(GetLastSessionSearchResults());
}

void UMultiplayerSubsystem::OnJoinSessionCompleted(const FName SessionName, const EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface)
		return;
	
	SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

	FString ConnectInfo;
	
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		// Get the connect string, which is usually the target's IP address.
		SessionInterface->GetResolvedConnectString(SessionName, ConnectInfo);
		
		const FString LogString = FString::Printf(TEXT("Joined session \"%s\" at %s"), *SessionName.ToString(), *ConnectInfo);
		UE_LOG(LogMultiplayer, Display, TEXT("%s"), *LogString)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, LogString);
	}
	else
	{
		const uint8 ErrorCode = static_cast<uint8>(Result);
		ConnectInfo = FString::Printf(TEXT("Error %i"), ErrorCode);
		const FString LogString = FString::Printf(TEXT("Failed to join session \"%s\": %s"), *SessionName.ToString(), *ConnectInfo);
		UE_LOG(LogMultiplayer, Warning, TEXT("%s"), *LogString)
		GEngine->AddOnScreenDebugMessage(-1, 5, FColor::White, LogString);
	}

	PostJoinSessionDelegate.Broadcast(LastSessionInfo, ConnectInfo);
}
