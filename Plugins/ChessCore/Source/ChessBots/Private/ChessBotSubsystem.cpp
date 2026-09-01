// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessBotSubsystem.h"

#include "Async/Async.h"
#include "ChessMctsBot.h"
#include "ChessRandomBot.h"
#include "Misc/ScopeLock.h"

namespace
{
	TUniquePtr<IChessBotEngine> CreateBotEngine(const EChessBotType BotType)
	{
		switch (BotType)
		{
		case EChessBotType::Mcts:
			return MakeUnique<FChessMctsBot>();
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

struct UChessBotSubsystem::FBotEngineSession
{
	FBotEngineSession(const EChessBotType InBotType, TUniquePtr<IChessBotEngine>&& InEngine)
		: BotType(InBotType)
		, Engine(MoveTemp(InEngine))
	{
	}

	EChessBotType BotType;
	TUniquePtr<IChessBotEngine> Engine;
	FCriticalSection Mutex;
};

void UChessBotSubsystem::Deinitialize()
{
	for (TPair<FGuid, FPendingRequest>& Pair : PendingRequests)
	{
		Pair.Value.CancellationToken->Cancel();
	}
	PendingRequests.Reset();
	EngineSessions.Reset();

	Super::Deinitialize();
}

FGuid UChessBotSubsystem::RequestMove(const FString& Fen, const FChessBotSettings& Settings, FOnChessBotMoveReady OnCompleted)
{
	for (auto Iterator = EngineSessions.CreateIterator(); Iterator; ++Iterator)
	{
		if (!Iterator.Key().IsValid())
		{
			Iterator.RemoveCurrent();
		}
	}

	const FGuid RequestId = FGuid::NewGuid();
	UObject* RequestOwner = OnCompleted.GetUObject();
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
	SearchRequest.MctsSettings.IterationLimit = static_cast<uint64>(FMath::Max(0, Settings.MctsSettings.IterationLimit));
	SearchRequest.MctsSettings.ExplorationConstant = FMath::Max(0.0f, Settings.MctsSettings.ExplorationConstant);
	SearchRequest.MctsSettings.MaterialScoreScale = FMath::Max(0.001f, Settings.MctsSettings.MaterialScoreScale);
	SearchRequest.MctsSettings.PawnValue = FMath::Max(0.0f, Settings.MctsSettings.PawnValue);
	SearchRequest.MctsSettings.KnightValue = FMath::Max(0.0f, Settings.MctsSettings.KnightValue);
	SearchRequest.MctsSettings.BishopValue = FMath::Max(0.0f, Settings.MctsSettings.BishopValue);
	SearchRequest.MctsSettings.RookValue = FMath::Max(0.0f, Settings.MctsSettings.RookValue);
	SearchRequest.MctsSettings.QueenValue = FMath::Max(0.0f, Settings.MctsSettings.QueenValue);
	SearchRequest.MctsSettings.MaterialWeight = FMath::Max(0.0f, Settings.MctsSettings.MaterialWeight);
	SearchRequest.MctsSettings.PieceActivityWeight = FMath::Max(0.0f, Settings.MctsSettings.PieceActivityWeight);
	SearchRequest.MctsSettings.MobilityWeight = FMath::Max(0.0f, Settings.MctsSettings.MobilityWeight);
	SearchRequest.MctsSettings.PawnStructureWeight = FMath::Max(0.0f, Settings.MctsSettings.PawnStructureWeight);
	SearchRequest.MctsSettings.KingSafetyWeight = FMath::Max(0.0f, Settings.MctsSettings.KingSafetyWeight);
	SearchRequest.MctsSettings.ThreatWeight = FMath::Max(0.0f, Settings.MctsSettings.ThreatWeight);
	SearchRequest.MctsSettings.PolicyPriorStrength = FMath::Max(0.0f, Settings.MctsSettings.PolicyPriorStrength);
	SearchRequest.MctsSettings.CheckPolicyWeight = FMath::Max(0.0f, Settings.MctsSettings.CheckPolicyWeight);
	SearchRequest.MctsSettings.CapturePolicyWeight = FMath::Max(0.0f, Settings.MctsSettings.CapturePolicyWeight);
	SearchRequest.MctsSettings.AttackPolicyWeight = FMath::Max(0.0f, Settings.MctsSettings.AttackPolicyWeight);
	SearchRequest.MctsSettings.DefensePolicyWeight = FMath::Max(0.0f, Settings.MctsSettings.DefensePolicyWeight);
	SearchRequest.MctsSettings.bLogSearch = Settings.MctsSettings.bLogSearch;
	SearchRequest.MctsSettings.LogCandidateCount = FMath::Max(0, Settings.MctsSettings.LogCandidateCount);
	SearchRequest.PreferredOpeningEcos.reserve(Settings.PreferredOpenings.Num());
	for (const FString& Eco : Settings.PreferredOpenings)
	{
		const FTCHARToUTF8 EcoUtf8(*Eco);
		SearchRequest.PreferredOpeningEcos.emplace_back(EcoUtf8.Get(), EcoUtf8.Length());
	}

	const EChessBotType BotType = Settings.BotType;
	TSharedPtr<FBotEngineSession, ESPMode::ThreadSafe> EngineSession;
	if (RequestOwner)
	{
		const TWeakObjectPtr<UObject> SessionKey(RequestOwner);
		EngineSession = EngineSessions.FindRef(SessionKey);
		if (!EngineSession || EngineSession->BotType != BotType)
		{
			EngineSession = MakeShared<FBotEngineSession, ESPMode::ThreadSafe>(BotType, CreateBotEngine(BotType));
			EngineSessions.Add(SessionKey, EngineSession);
		}
	}
	else
	{
		EngineSession = MakeShared<FBotEngineSession, ESPMode::ThreadSafe>(BotType, CreateBotEngine(BotType));
	}
	const TWeakObjectPtr<UChessBotSubsystem> WeakThis(this);

	Async(EAsyncExecution::ThreadPool,
		[WeakThis, RequestId, SearchRequest = MoveTemp(SearchRequest), CancellationToken, EngineSession]() mutable
		{
			FChessBotSearchResult SearchResult;
			{
				FScopeLock Lock(&EngineSession->Mutex);
				SearchResult = EngineSession->Engine->FindMove(SearchRequest, *CancellationToken);
			}

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
