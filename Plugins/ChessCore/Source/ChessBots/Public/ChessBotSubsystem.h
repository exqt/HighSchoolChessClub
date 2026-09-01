// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChessBotEngine.h"
#include "ChessBotTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ChessBotSubsystem.generated.h"

UCLASS()
class CHESSBOTS_API UChessBotSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category="Chess Bot", meta=(AutoCreateRefTerm="Settings"))
	FGuid RequestMove(const FString& Fen, const FChessBotSettings& Settings, FOnChessBotMoveReady OnCompleted);

	UFUNCTION(BlueprintCallable, Category="Chess Bot")
	bool CancelRequest(FGuid RequestId);

	UFUNCTION(BlueprintPure, Category="Chess Bot")
	bool IsRequestPending(FGuid RequestId) const;

private:
	struct FBotEngineSession;

	struct FPendingRequest
	{
		TSharedPtr<FChessBotCancellationToken, ESPMode::ThreadSafe> CancellationToken;
		FOnChessBotMoveReady OnCompleted;
	};

	void CompleteRequest(FGuid RequestId, FChessBotSearchResult&& SearchResult);

	TMap<FGuid, FPendingRequest> PendingRequests;
	TMap<TWeakObjectPtr<UObject>, TSharedPtr<FBotEngineSession, ESPMode::ThreadSafe>> EngineSessions;
};
