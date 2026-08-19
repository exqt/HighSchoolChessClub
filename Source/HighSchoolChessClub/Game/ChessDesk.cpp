#include "Game/ChessDesk.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

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

void AChessDesk::RefreshCursorTransform()
{
	const FVector RelativeLocation(
		static_cast<float>(CursorSquare.X) * SquareSize,
		-static_cast<float>(CursorSquare.Y) * SquareSize,
		0);
	CursorMesh->SetRelativeLocation(RelativeLocation);
}
