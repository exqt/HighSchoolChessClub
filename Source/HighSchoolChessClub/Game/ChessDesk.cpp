#include "Game/ChessDesk.h"

#include "ChessGameState.h"
#include "ChessPiece.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AChessDesk::AChessDesk()
{
	PrimaryActorTick.bCanEverTick = false;

	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	BoardOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("Board Origin"));
	BoardOrigin->SetupAttachment(SceneRoot);

	CursorMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cursor Mesh"));
	CursorMesh->SetupAttachment(BoardOrigin);
	CursorMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CursorMesh->SetGenerateOverlapEvents(false);
}

void AChessDesk::BeginPlay()
{
	Super::BeginPlay();
	ChessState = NewObject<UChessGameState>(this);
	CursorSquare.X = FMath::Clamp(CursorSquare.X, 0, 7);
	CursorSquare.Y = FMath::Clamp(CursorSquare.Y, 0, 7);
	RefreshCursorTransform();
	SetupInitialPosition();
}

bool AChessDesk::MoveCursor(const FIntPoint Delta, const EChessPlayerPosition PlayerPosition)
{
	if (!CanPlayerControl(PlayerPosition))
	{
		return false;
	}

	return SetCursorSquare(CursorSquare + Delta);
}

bool AChessDesk::SetCursorSquare(FIntPoint NewSquare)
{
	NewSquare.X = FMath::Clamp(NewSquare.X, 0, 7);
	NewSquare.Y = FMath::Clamp(NewSquare.Y, 0, 7);

	if (NewSquare == CursorSquare)
	{
		return false;
	}

	CursorSquare = NewSquare;
	RefreshCursorTransform();
	return true;
}

FVector AChessDesk::GetCursorWorldLocation() const
{
	if (!IsValid(BoardOrigin))
	{
		return GetActorLocation();
	}

	const FVector BoardLocation(
		static_cast<float>(CursorSquare.X) * SquareSize,
		-static_cast<float>(CursorSquare.Y) * SquareSize,
		0);
	return BoardOrigin->GetComponentTransform().TransformPosition(BoardLocation);
}

bool AChessDesk::SelectCurrentSquare(const EChessPlayerPosition PlayerPosition)
{
	if (!CanPlayerControl(PlayerPosition))
	{
		return false;
	}

	const EChessCorePieceColor PlayerColor = GetPlayerColor(PlayerPosition);
	if (PlayerColor == EChessCorePieceColor::None || ChessState->GetSideToMove() != PlayerColor)
	{
		return false;
	}

	if (!bHasSelectedSquare)
	{
		return SelectPieceAtCursor(PlayerColor);
	}

	if (CursorSquare == SelectedSquare)
	{
		CancelSelection();
		return true;
	}

	const FChessCoreMove* MoveToApply = nullptr;
	for (const FChessCoreMove& Move : SelectedLegalMoves)
	{
		if (Move.To.File != CursorSquare.X || Move.To.Rank != CursorSquare.Y)
		{
			continue;
		}

		if (!MoveToApply || Move.Promotion == EChessCorePieceType::Queen)
		{
			MoveToApply = &Move;
		}
	}

	if (MoveToApply)
	{
		FChessCorePiece MovingPiece;
		if (!GetPieceAtSquare(SelectedSquare, MovingPiece))
		{
			return false;
		}

		FChessCoreMove AppliedMove;
		if (!ChessState->TryMakeMove(*MoveToApply, AppliedMove))
		{
			return false;
		}

		CancelSelection();
		if (!ApplyMoveToPieceActors(AppliedMove, MovingPiece))
		{
			RebuildPieceActorsFromState();
		}
		RefreshCursorVisibility();
		OnMoveApplied(AppliedMove);
		return true;
	}

	FChessCorePiece Piece;
	if (GetPieceAtSquare(CursorSquare, Piece) && Piece.Color == PlayerColor)
	{
		CancelSelection();
		return SelectPieceAtCursor(PlayerColor);
	}

	return false;
}

void AChessDesk::CancelSelection()
{
	if (!bHasSelectedSquare)
	{
		return;
	}

	if (AChessPiece* SelectedPieceActor = PieceActorsBySquare.FindRef(SelectedSquare))
	{
		SelectedPieceActor->LowerPiece();
	}

	bHasSelectedSquare = false;
	SelectedSquare = FIntPoint::ZeroValue;
	SelectedLegalMoves.Reset();
	OnSelectionCleared();
}

TArray<FIntPoint> AChessDesk::GetLegalDestinationSquares() const
{
	TArray<FIntPoint> Destinations;
	Destinations.Reserve(SelectedLegalMoves.Num());

	for (const FChessCoreMove& Move : SelectedLegalMoves)
	{
		const FIntPoint Destination(Move.To.File, Move.To.Rank);
		Destinations.AddUnique(Destination);
	}

	return Destinations;
}

EChessCorePieceColor AChessDesk::GetSideToMove() const
{
	return ChessState ? ChessState->GetSideToMove() : EChessCorePieceColor::None;
}

EChessCorePieceColor AChessDesk::GetPlayerColor(const EChessPlayerPosition PlayerPosition) const
{
	if (PlayerPosition == EChessPlayerPosition::PlayerA) return PlayerAColor;
	return PlayerAColor == EChessCorePieceColor::White ? EChessCorePieceColor::Black : EChessCorePieceColor::White;
}

void AChessDesk::BeginPlayerControl(const EChessPlayerPosition PlayerPosition)
{
	ActivePlayerPosition = PlayerPosition;
	bHasActivePlayer = true;
	RefreshCursorVisibility();
}

void AChessDesk::EndPlayerControl(const EChessPlayerPosition PlayerPosition)
{
	if (!bHasActivePlayer || ActivePlayerPosition != PlayerPosition)
	{
		return;
	}

	bHasActivePlayer = false;
	CancelSelection();
	RefreshCursorVisibility();
}

bool AChessDesk::CanPlayerControl(const EChessPlayerPosition PlayerPosition) const
{
	return bHasActivePlayer
		&& ActivePlayerPosition == PlayerPosition
		&& ChessState
		&& ChessState->GetSideToMove() == GetPlayerColor(PlayerPosition);
}

void AChessDesk::SetupInitialPosition()
{
	if (!ChessState)
	{
		ChessState = NewObject<UChessGameState>(this);
	}

	ChessState->ResetToStartPosition();
	CancelSelection();
	RebuildPieceActorsFromState();
	RefreshCursorVisibility();
}

void AChessDesk::RebuildPieceActorsFromState()
{
	for (const TPair<FIntPoint, TObjectPtr<AChessPiece>>& Entry : PieceActorsBySquare)
	{
		if (Entry.Value)
		{
			Entry.Value->Destroy();
		}
	}
	PieceActorsBySquare.Reset();

	TArray<FChessCorePiece> Pieces;
	ChessState->GetPieces(Pieces);

	for (const FChessCorePiece& Piece : Pieces)
	{
		SpawnPieceActor(Piece);
	}
}

AChessPiece* AChessDesk::SpawnPieceActor(const FChessCorePiece& Piece)
{
	const TSoftClassPtr<AChessPiece>* SoftPieceClass = ChessPieceClasses.Find(Piece.Type);
	if (!SoftPieceClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("Chess piece class is not configured for type %d."), static_cast<int32>(Piece.Type));
		return nullptr;
	}

	UClass* PieceClass = SoftPieceClass->LoadSynchronous();
	if (!PieceClass)
	{
		return nullptr;
	}

	const FIntPoint Square(Piece.Square.File, Piece.Square.Rank);
	const FVector BoardLocation(
		static_cast<float>(Square.X) * SquareSize,
		-static_cast<float>(Square.Y) * SquareSize,
		0.0f);
	const FVector WorldLocation = BoardOrigin->GetComponentTransform().TransformPosition(BoardLocation);
	const FTransform SpawnTransform(BoardOrigin->GetComponentQuat(), WorldLocation);

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = this;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AChessPiece* PieceActor = GetWorld()->SpawnActor<AChessPiece>(PieceClass, SpawnTransform, SpawnParameters);
	if (!PieceActor)
	{
		return nullptr;
	}

	PieceActor->AttachToComponent(BoardOrigin, FAttachmentTransformRules::KeepWorldTransform);
	PieceActor->SetPieceColor(Piece.Color);
	PieceActorsBySquare.Add(Square, PieceActor);
	return PieceActor;
}

bool AChessDesk::ApplyMoveToPieceActors(const FChessCoreMove& Move, const FChessCorePiece& MovingPiece)
{
	const FIntPoint FromSquare(Move.From.File, Move.From.Rank);
	const FIntPoint ToSquare(Move.To.File, Move.To.Rank);
	AChessPiece* MovingActor = PieceActorsBySquare.FindRef(FromSquare);
	if (!MovingActor)
	{
		return false;
	}

	if (PieceActorsBySquare.Contains(ToSquare))
	{
		DestroyPieceActorAtSquare(ToSquare);
	}
	else if (MovingPiece.Type == EChessCorePieceType::Pawn && FromSquare.X != ToSquare.X) // En passant
	{
		DestroyPieceActorAtSquare(FIntPoint(ToSquare.X, FromSquare.Y));
	}

	PieceActorsBySquare.Remove(FromSquare);
	MovePieceActorToSquare(MovingActor, ToSquare);
	PieceActorsBySquare.Add(ToSquare, MovingActor);

	if (MovingPiece.Type == EChessCorePieceType::King && FMath::Abs(ToSquare.X - FromSquare.X) == 2)
	{
		const bool bKingSide = ToSquare.X > FromSquare.X;
		const FIntPoint RookFromSquare(bKingSide ? 7 : 0, FromSquare.Y);
		const FIntPoint RookToSquare(bKingSide ? 5 : 3, FromSquare.Y);
		AChessPiece* RookActor = PieceActorsBySquare.FindRef(RookFromSquare);
		if (!RookActor)
		{
			return false;
		}

		PieceActorsBySquare.Remove(RookFromSquare);
		MovePieceActorToSquare(RookActor, RookToSquare);
		PieceActorsBySquare.Add(RookToSquare, RookActor);
	}

	if (Move.Promotion != EChessCorePieceType::None)
	{
		DestroyPieceActorAtSquare(ToSquare);

		FChessCorePiece PromotedPiece;
		if (!GetPieceAtSquare(ToSquare, PromotedPiece) || !SpawnPieceActor(PromotedPiece))
		{
			return false;
		}
	}

	return true;
}

void AChessDesk::MovePieceActorToSquare(AChessPiece* PieceActor, const FIntPoint Square) const
{
	const FVector BoardLocation(
		static_cast<float>(Square.X) * SquareSize,
		-static_cast<float>(Square.Y) * SquareSize,
		0.0f);
	PieceActor->MovePieceTo(BoardOrigin->GetComponentTransform().TransformPosition(BoardLocation));
}

void AChessDesk::DestroyPieceActorAtSquare(const FIntPoint Square)
{
	AChessPiece* PieceActor = PieceActorsBySquare.FindRef(Square);
	if (!PieceActor)
	{
		return;
	}

	PieceActorsBySquare.Remove(Square);
	PieceActor->Destroy();
}

bool AChessDesk::SelectPieceAtCursor(const EChessCorePieceColor PlayerColor)
{
	FChessCorePiece Piece;
	if (!GetPieceAtSquare(CursorSquare, Piece) || Piece.Color != PlayerColor)
	{
		return false;
	}

	TArray<FChessCoreMove> LegalMoves;
	ChessState->GetLegalMoves(LegalMoves);

	SelectedLegalMoves.Reset();
	for (const FChessCoreMove& Move : LegalMoves)
	{
		if (Move.From.File == CursorSquare.X && Move.From.Rank == CursorSquare.Y)
		{
			SelectedLegalMoves.Add(Move);
		}
	}

	if (SelectedLegalMoves.IsEmpty())
	{
		return false;
	}

	bHasSelectedSquare = true;
	SelectedSquare = CursorSquare;
	if (AChessPiece* SelectedPieceActor = PieceActorsBySquare.FindRef(SelectedSquare))
	{
		SelectedPieceActor->LiftPiece();
	}
	OnPieceSelected(SelectedSquare, GetLegalDestinationSquares());
	return true;
}

bool AChessDesk::GetPieceAtSquare(const FIntPoint Square, FChessCorePiece& OutPiece) const
{
	TArray<FChessCorePiece> Pieces;
	if (!ChessState || !ChessState->GetPieces(Pieces))
	{
		return false;
	}

	for (const FChessCorePiece& Piece : Pieces)
	{
		if (Piece.Square.File == Square.X && Piece.Square.Rank == Square.Y)
		{
			OutPiece = Piece;
			return true;
		}
	}

	return false;
}

void AChessDesk::RefreshCursorTransform()
{
	const FVector RelativeLocation(
		static_cast<float>(CursorSquare.X) * SquareSize,
		-static_cast<float>(CursorSquare.Y) * SquareSize,
		0);
	CursorMesh->SetRelativeLocation(RelativeLocation);
}

void AChessDesk::RefreshCursorVisibility()
{
	const bool bShouldBeVisible = bHasActivePlayer && CanPlayerControl(ActivePlayerPosition);
	CursorMesh->SetHiddenInGame(!bShouldBeVisible);
}
