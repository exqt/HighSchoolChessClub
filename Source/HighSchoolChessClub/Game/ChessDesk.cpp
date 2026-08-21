#include "Game/ChessDesk.h"

#include "Game/ChessPiece.h"
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
	CursorSquare.X = FMath::Clamp(CursorSquare.X, 0, 7);
	CursorSquare.Y = FMath::Clamp(CursorSquare.Y, 0, 7);
	RefreshCursorTransform();
	SetCursorVisible(false);
}

bool AChessDesk::MoveCursor(const FIntPoint Delta)
{
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
	const FVector BoardLocation(
		static_cast<float>(CursorSquare.X) * SquareSize,
		-static_cast<float>(CursorSquare.Y) * SquareSize,
		CursorHeight);
	return BoardOrigin->GetComponentTransform().TransformPosition(BoardLocation);
}

void AChessDesk::SetCursorVisible(const bool bVisible)
{
	CursorMesh->SetHiddenInGame(!bVisible);
}

bool AChessDesk::ProjectRayToSquare(
	const FVector& RayOrigin,
	const FVector& RayDirection,
	FIntPoint& OutSquare) const
{
	const FVector BoardNormal = BoardOrigin->GetUpVector();
	const float Denominator = FVector::DotProduct(RayDirection, BoardNormal);
	if (FMath::IsNearlyZero(Denominator))
	{
		return false;
	}

	const float Distance = FVector::DotProduct(
		BoardOrigin->GetComponentLocation() - RayOrigin,
		BoardNormal) / Denominator;
	if (Distance < 0.0f)
	{
		return false;
	}

	return WorldLocationToSquare(RayOrigin + RayDirection * Distance, OutSquare);
}

bool AChessDesk::WorldLocationToSquare(const FVector& WorldLocation, FIntPoint& OutSquare) const
{
	const FVector LocalLocation = BoardOrigin->GetComponentTransform().InverseTransformPosition(WorldLocation);
	OutSquare.X = FMath::RoundToInt(LocalLocation.X / SquareSize);
	OutSquare.Y = FMath::RoundToInt(-LocalLocation.Y / SquareSize);
	return OutSquare.X >= 0 && OutSquare.X < 8 && OutSquare.Y >= 0 && OutSquare.Y < 8;
}

void AChessDesk::ShowPieceSelection(const FIntPoint Square, const TArray<FIntPoint>& LegalDestinations)
{
	if (AChessPiece* PieceActor = PieceActorsBySquare.FindRef(Square))
	{
		PieceActor->LiftPiece();
	}
	OnPieceSelected(Square, LegalDestinations);
}

void AChessDesk::ClearPieceSelection(const FIntPoint Square)
{
	if (AChessPiece* PieceActor = PieceActorsBySquare.FindRef(Square))
	{
		PieceActor->LowerPiece();
	}
	OnSelectionCleared();
}

void AChessDesk::RebuildPieceActors(const TArray<FChessCorePiece>& Pieces)
{
	for (const TPair<FIntPoint, TObjectPtr<AChessPiece>>& Entry : PieceActorsBySquare)
	{
		if (Entry.Value)
		{
			Entry.Value->Destroy();
		}
	}
	PieceActorsBySquare.Reset();

	for (const FChessCorePiece& Piece : Pieces)
	{
		SpawnPieceActor(Piece);
	}
}

bool AChessDesk::ApplyMoveToPieceActors(
	const FChessCoreMove& Move,
	const FChessCorePiece& MovingPiece,
	const FChessCorePiece& PieceAfterMove)
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
	else if (MovingPiece.Type == EChessCorePieceType::Pawn && FromSquare.X != ToSquare.X)
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
		return SpawnPieceActor(PieceAfterMove) != nullptr;
	}
	return true;
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

void AChessDesk::RefreshCursorTransform()
{
	const FVector RelativeLocation(
		static_cast<float>(CursorSquare.X) * SquareSize,
		-static_cast<float>(CursorSquare.Y) * SquareSize,
		CursorHeight);
	CursorMesh->SetRelativeLocation(RelativeLocation);
}
