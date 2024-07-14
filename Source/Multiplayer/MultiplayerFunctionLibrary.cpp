#include "MultiplayerFunctionLibrary.h"
#include "MultiplayerGlobals.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/OnlineReplStructs.h"
#include "Online/CoreOnline.h"

FUniqueNetIdRepl UMultiplayerFunctionLibrary::GetLocalPlayerNetId(const UObject* WorldContextObject)
{
	FUniqueNetIdRepl PlayerNetId;
	
	if (const APlayerController* PlayerController = UGameplayStatics::GetPlayerControllerFromID(WorldContextObject, 0))
	{
		if (const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer())
		{
			PlayerNetId = LocalPlayer->GetPreferredUniqueNetId();
		}
	}
	
	return PlayerNetId;
}

bool UMultiplayerFunctionLibrary::IsClient(const UWorld* World)
{
	return World ? World->GetNetMode() >= NM_Client : false;
}

bool UMultiplayerFunctionLibrary::IsServer(const UWorld* World)
{
	return World ? World->GetNetMode() <= NM_ListenServer : true;
}

FString UMultiplayerFunctionLibrary::NetIdToString(const FUniqueNetIdRepl& NetId)
{
	return NetId.IsValid() ? NetId->ToString() : FString();
}

FUniqueNetIdRepl UMultiplayerFunctionLibrary::StringToNetId(const FString& NetIdString)
{
	FUniqueNetIdRepl Result;
	Result.FromJson(NetIdString);
	return Result;
}

FMultiplayerSessionInfo UMultiplayerFunctionLibrary::SessionSearchResultToInfo(const FMultiplayerSessionSearchResult& SessionSearchResult)
{
	return FMultiplayerSessionInfo(SessionSearchResult.Result);
}

void UMultiplayerFunctionLibrary::LogNetMode(const UWorld* World)
{
	const FString LogString = World ? ToString(World->GetNetMode()) : "Invalid";
	UE_LOG(LogMultiplayer, Warning, TEXT("NetMode: %s"), *LogString);
}
