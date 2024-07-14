#pragma once

#include "CoreMinimal.h"
#include "MultiplayerGlobals.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MultiplayerSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPostCreateSessionDelegate, const FMultiplayerSessionInfo&, SessionInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPostDestroySessionDelegate, const FName&, SessionName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPostFindSessionDelegate, const TArray<FMultiplayerSessionSearchResult>&, SessionSearchResults);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPostJoinSessionDelegate, const FMultiplayerSessionInfo&, SessionInfo, const FString&, ConnectInfo);

UCLASS(Config=Game)
class MULTIPLAYER_API UMultiplayerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSubsystem();
	
	FOnlineSessionSettings DefaultSessionSettings;
	FOnlineSessionSearch DefaultSessionSearch;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	FName OnlineSubsystemName = NAME_None;
	
	// Info on the last session that this subsystem has either attempted to create or join.
	UPROPERTY(BlueprintReadOnly)
	FMultiplayerSessionInfo LastSessionInfo;
	
	IOnlineSessionPtr SessionInterface = nullptr;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FDelegateHandle CreateSessionCompleteDelegateHandle;

	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FDelegateHandle JoinSessionCompleteDelegateHandle;

public:
	UPROPERTY(BlueprintAssignable)
	FPostCreateSessionDelegate PostCreateSessionDelegate;

	UPROPERTY(BlueprintAssignable)
	FPostDestroySessionDelegate PostDestroySessionDelegate;
	
	UPROPERTY(BlueprintAssignable)
	FPostFindSessionDelegate PostFindSessionDelegate;

	UPROPERTY(BlueprintAssignable)
	FPostJoinSessionDelegate PostJoinSessionDelegate;
	
	// Returns all the search results from the last time FindSession() was called.
	UFUNCTION(BlueprintPure)
	TArray<FMultiplayerSessionSearchResult> GetLastSessionSearchResults() const;

	// Returns the search result from the last time FindSession() was called that has the corresponding host player ID.
	UFUNCTION(BlueprintPure)
	FMultiplayerSessionSearchResult GetLastSessionSearchResultByHostId(const FUniqueNetIdRepl InHostId) const;

	// Returns the search result from the last time FindSession() was called that has the corresponding session ID.
	UFUNCTION(BlueprintPure)
	FMultiplayerSessionSearchResult GetLastSessionSearchResultBySessionId(const FUniqueNetIdRepl InSessionId) const;
	
	// Creates a session based on the given session info params.
	UFUNCTION(BlueprintCallable)
	void CreateSession(const FMultiplayerSessionSettings SessionSettings);

	// Safely destroys an existing session of the given name.
	UFUNCTION(BlueprintCallable)
	void DestroySession(const FName SessionName);

	// Finds all the publicly available sessions.
	UFUNCTION(BlueprintCallable)
	void FindSessions();

	// Joins a session from the given name and search result.
	UFUNCTION(BlueprintCallable)
	void JoinSession(const FMultiplayerSessionSearchResult& SessionSearchResult);
	
protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void OnCreateSessionCompleted(const FName SessionName, const bool bWasSuccessful);
	void OnDestroySessionCompleted(const FName SessionName, const bool bWasSuccessful);
	void OnFindSessionsCompleted(const bool bWasSuccessful);
	void OnJoinSessionCompleted(const FName SessionName, const EOnJoinSessionCompleteResult::Type Result);
};
