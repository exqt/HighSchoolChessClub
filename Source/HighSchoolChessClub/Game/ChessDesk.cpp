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
	CursorSquare.X = FMath::Clamp(CursorSquare.X, 0, 7);
	CursorSquare.Y = FMath::Clamp(CursorSquare.Y, 0, 7);
	RefreshCursorTransform();
	SetupInitialPosition();
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

void AChessDesk::SetupInitialPosition()
{
	for (AChessPiece* PieceActor : PieceActors)
	{
		if (PieceActor)
		{
			PieceActor->Destroy();
		}
	}
	PieceActors.Reset();

	const UChessGameState* State = NewObject<UChessGameState>(this);
	TArray<FChessCorePiece> Pieces;
	State->GetPieces(Pieces);

	UWorld* World = GetWorld();
	
	for (const FChessCorePiece& Piece : Pieces)
	{
		const TSoftClassPtr<AChessPiece>* SoftPieceClass = ChessPieceClasses.Find(Piece.Type);
		if (!SoftPieceClass)
		{
			UE_LOG(LogTemp, Warning, TEXT("Chess piece class is not configured for type %d."), static_cast<int32>(Piece.Type));
			continue;
		}

		UClass* PieceClass = SoftPieceClass->LoadSynchronous();
		if (!PieceClass)
		{
			continue;
		}

		const FVector BoardLocation(
			static_cast<float>(Piece.Square.File) * SquareSize,
			-static_cast<float>(Piece.Square.Rank) * SquareSize,
			0.0f);
		const FVector WorldLocation = BoardOrigin->GetComponentTransform().TransformPosition(BoardLocation);
		const FTransform SpawnTransform(BoardOrigin->GetComponentQuat(), WorldLocation);

		FActorSpawnParameters SpawnParameters;
		SpawnParameters.Owner = this;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AChessPiece* PieceActor = World->SpawnActor<AChessPiece>(PieceClass, SpawnTransform, SpawnParameters);
		if (!PieceActor)
		{
			continue;
		}

		PieceActor->AttachToComponent(BoardOrigin, FAttachmentTransformRules::KeepWorldTransform);
		PieceActor->SetPieceColor(Piece.Color);
		PieceActors.Add(PieceActor);
	}
}

void AChessDesk::RefreshCursorTransform()
{
	const FVector RelativeLocation(
		static_cast<float>(CursorSquare.X) * SquareSize,
		-static_cast<float>(CursorSquare.Y) * SquareSize,
		0);
	CursorMesh->SetRelativeLocation(RelativeLocation);
}
