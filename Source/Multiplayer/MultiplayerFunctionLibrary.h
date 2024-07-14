#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "MultiplayerFunctionLibrary.generated.h"

struct FMultiplayerSessionInfo;
struct FMultiplayerSessionSearchResult;

UCLASS()
class MULTIPLAYER_API UMultiplayerFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// Returns the unique net ID of the local player using the first player controller.
	UFUNCTION(BlueprintPure, meta = (WorldContext = "WorldContextObject"))
	static FUniqueNetIdRepl GetLocalPlayerNetId(const UObject* WorldContextObject);
	
	// Returns true if the world is running on a client build. Defaults false if the world is invalid.
	UFUNCTION(BlueprintPure)
	static bool IsClient(const UWorld* World);

	// Returns true if the world is running on a server build. Defaults true if the world is invalid.
	UFUNCTION(BlueprintPure)
	static bool IsServer(const UWorld* World);

	// Returns the given unique net ID in string form.
	UFUNCTION(BlueprintPure)
	static FString NetIdToString(const FUniqueNetIdRepl& NetId);

	// Converts a string into a unique net ID.
	UFUNCTION(BlueprintPure)
	static FUniqueNetIdRepl StringToNetId(const FString& NetIdString);

	// Converts a session search result to a session info.
	UFUNCTION(BlueprintPure)
	static FMultiplayerSessionInfo SessionSearchResultToInfo(const FMultiplayerSessionSearchResult& SessionSearchResult);
	
	// Logs the current net mode.
	UFUNCTION(BlueprintCallable)
	static void LogNetMode(const UWorld* World);
};
