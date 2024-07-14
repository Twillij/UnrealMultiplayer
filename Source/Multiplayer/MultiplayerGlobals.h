#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerGlobals.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMultiplayer, Display, All)

// A wrapper for FOnlineSessionSettings that is intended for blueprint use
USTRUCT(BlueprintType)
struct FMultiplayerSessionSettings
{
	GENERATED_BODY()

	FMultiplayerSessionSettings() {}
	FMultiplayerSessionSettings(const FOnlineSessionSettings& InSessionSettings);
	
	// The name of the session
	UPROPERTY(BlueprintReadWrite)
	FName SessionName = NAME_None;

	// The number of publicly available connections advertised
	UPROPERTY(BlueprintReadWrite)
	int32 NumPublicConnections = 2;

	// Overrides some of FOnlineSessionSettings with own settings
	FOnlineSessionSettings operator+(const FOnlineSessionSettings& OnlineSessionSettings) const;
};

// A wrapper for FOnlineSession that is intended for blueprint use
USTRUCT(BlueprintType)
struct FMultiplayerSessionInfo
{
	GENERATED_BODY()

	FMultiplayerSessionInfo() {}
	FMultiplayerSessionInfo(const FOnlineSession& Session);
	FMultiplayerSessionInfo(const FOnlineSessionSearchResult& SessionSearchResult) : FMultiplayerSessionInfo(SessionSearchResult.Session) {}

	// The settings used to create the session
	UPROPERTY(BlueprintReadWrite)
	FMultiplayerSessionSettings SessionSettings;

	// The unique net ID of the session itself
	UPROPERTY(BlueprintReadWrite)
	FUniqueNetIdRepl SessionId;
	
	// The unique net ID of the player hosting the session
	UPROPERTY(BlueprintReadWrite)
	FUniqueNetIdRepl HostPlayerId;

	// The number of users currently in this session
	UPROPERTY(BlueprintReadWrite)
	int32 NumCurrentUsers = 0;
	
	FName GetSessionName() const { return SessionSettings.SessionName; }
};

// A wrapper for FOnlineSessionSearchResult that is intended for blueprint use
USTRUCT(BlueprintType)
struct FMultiplayerSessionSearchResult
{
	GENERATED_BODY()

	FMultiplayerSessionSearchResult() {}
	FMultiplayerSessionSearchResult(const FOnlineSessionSearchResult& SessionSearchResult) : Result(SessionSearchResult) {}
	
	FOnlineSessionSearchResult Result;
};
