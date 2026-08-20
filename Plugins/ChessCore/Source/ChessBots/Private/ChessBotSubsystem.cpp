// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessBotSubsystem.h"

#include "Async/Async.h"
#include "ChessRandomBot.h"

namespace
{
	TUniquePtr<IChessBotEngine> CreateBotEngine(const EChessBotType BotType)
	{
		switch (BotType)
		{
		case EChessBotType::Random:
		default:
			return MakeUnique<FChessRandomBot>();
		}
	}

	EChessBotResultStatus ToPublicStatus(const EChessBotSearchStatus Status)
	{
		switch (Status)
		{
		case EChessBotSearchStatus::MoveFound:
			return EChessBotResultStatus::MoveFound;
		case EChessBotSearchStatus::NoLegalMoves:
			return EChessBotResultStatus::NoLegalMoves;
		case EChessBotSearchStatus::Cancelled:
			return EChessBotResultStatus::Cancelled;
		case EChessBotSearchStatus::InvalidPosition:
		default:
			return EChessBotResultStatus::InvalidPosition;
		}
	}
}

void UChessBotSubsystem::Deinitialize()
{
	for (TPair<FGuid, FPendingRequest>& Pair : PendingRequests)
	{
		Pair.Value.CancellationToken->Cancel();
	}
	PendingRequests.Reset();

	Super::Deinitialize();
}

FGuid UChessBotSubsystem::RequestMove(const FString& Fen, const FChessBotSettings& Settings, FOnChessBotMoveReady OnCompleted)
{
	const FGuid RequestId = FGuid::NewGuid();
	const TSharedPtr<FChessBotCancellationToken, ESPMode::ThreadSafe> CancellationToken =
		MakeShared<FChessBotCancellationToken, ESPMode::ThreadSafe>();

	FPendingRequest& PendingRequest = PendingRequests.Add(RequestId);
	PendingRequest.CancellationToken = CancellationToken;
	PendingRequest.OnCompleted = MoveTemp(OnCompleted);

	FChessBotSearchRequest SearchRequest;
	const FTCHARToUTF8 FenUtf8(*Fen);
	SearchRequest.Fen.assign(FenUtf8.Get(), FenUtf8.Length());
	SearchRequest.RandomSeed = Settings.RandomSeed != 0 ? static_cast<uint64>(Settings.RandomSeed) : static_cast<uint64>(GetTypeHash(RequestId));
	SearchRequest.TimeLimitSeconds = Settings.TimeLimitSeconds;

	const EChessBotType BotType = Settings.BotType;
	const TWeakObjectPtr<UChessBotSubsystem> WeakThis(this);

	Async(EAsyncExecution::ThreadPool,
		[WeakThis, RequestId, BotType, SearchRequest = MoveTemp(SearchRequest), CancellationToken]() mutable
		{
			TUniquePtr<IChessBotEngine> BotEngine = CreateBotEngine(BotType);
			FChessBotSearchResult SearchResult = BotEngine->FindMove(SearchRequest, *CancellationToken);

			AsyncTask(ENamedThreads::GameThread,
				[WeakThis, RequestId, SearchResult = MoveTemp(SearchResult)]() mutable
				{
					if (UChessBotSubsystem* Subsystem = WeakThis.Get())
					{
						Subsystem->CompleteRequest(RequestId, MoveTemp(SearchResult));
					}
				});
		});

	return RequestId;
}

bool UChessBotSubsystem::CancelRequest(const FGuid RequestId)
{
	FPendingRequest* PendingRequest = PendingRequests.Find(RequestId);
	if (!PendingRequest)
	{
		return false;
	}

	PendingRequest->CancellationToken->Cancel();
	return true;
}

bool UChessBotSubsystem::IsRequestPending(const FGuid RequestId) const
{
	return PendingRequests.Contains(RequestId);
}

void UChessBotSubsystem::CompleteRequest(FGuid RequestId, FChessBotSearchResult&& SearchResult)
{
	FPendingRequest PendingRequest;
	if (!PendingRequests.RemoveAndCopyValue(RequestId, PendingRequest))
	{
		return;
	}

	if (PendingRequest.CancellationToken->IsCancellationRequested())
	{
		SearchResult.Status = EChessBotSearchStatus::Cancelled;
		SearchResult.UciMove.clear();
	}

	FChessBotResult Result;
	Result.RequestId = RequestId;
	Result.Status = ToPublicStatus(SearchResult.Status);
	Result.UciMove = UTF8_TO_TCHAR(SearchResult.UciMove.c_str());
	Result.SearchedNodes = static_cast<int64>(SearchResult.SearchedNodes);
	Result.ElapsedSeconds = static_cast<float>(SearchResult.ElapsedSeconds);

	if (PendingRequest.OnCompleted.IsBound())
	{
		PendingRequest.OnCompleted.Execute(Result);
	}
}
