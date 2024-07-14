#include "MultiplayerGlobals.h"

DEFINE_LOG_CATEGORY(LogMultiplayer);

FMultiplayerSessionSettings::FMultiplayerSessionSettings(const FOnlineSessionSettings& InSessionSettings)
{
	FString NameString;
	InSessionSettings.Get(FName("SESSION_NAME"), NameString);
	SessionName = FName(NameString);
	NumPublicConnections = InSessionSettings.NumPublicConnections;
}

FOnlineSessionSettings FMultiplayerSessionSettings::operator+(const FOnlineSessionSettings& OnlineSessionSettings) const
{
	FOnlineSessionSettings Result = OnlineSessionSettings;
	Result.Set(FName("SESSION_NAME"), SessionName.ToString(), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	Result.NumPublicConnections = NumPublicConnections;
	return Result;
}

FMultiplayerSessionInfo::FMultiplayerSessionInfo(const FOnlineSession& Session)
{
	SessionSettings = Session.SessionSettings;
	SessionId = Session.SessionInfo ? Session.SessionInfo->GetSessionId() : SessionId;
	HostPlayerId = Session.OwningUserId;
	NumCurrentUsers = Session.SessionSettings.NumPublicConnections - Session.NumOpenPublicConnections;
}
