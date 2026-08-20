#include "Game/ChessPiece.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Game/PositionTweenComponent.h"
#include "Materials/MaterialInterface.h"

AChessPiece::AChessPiece()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Piece Mesh"));
	PieceMesh->SetupAttachment(SceneRoot);

	PositionTween = CreateDefaultSubobject<UPositionTweenComponent>(TEXT("Position Tween"));

	MaterialIndices.Add(0);
}

void AChessPiece::BeginPlay()
{
	Super::BeginPlay();
	RestingWorldPosition = GetActorLocation();
}

void AChessPiece::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	ApplyPieceColor();
}

void AChessPiece::SetPieceColor(const EChessCorePieceColor NewColor)
{
	PieceColor = NewColor;
	ApplyPieceColor();
}

void AChessPiece::LiftPiece()
{
	if (bIsLifted)
	{
		return;
	}

	bIsLifted = true;
	PositionTween->TweenToPosition(
		RestingWorldPosition + GetActorUpVector() * SelectionLiftHeight,
		SelectionTweenDuration);
}

void AChessPiece::LowerPiece()
{
	if (!bIsLifted)
	{
		return;
	}

	bIsLifted = false;
	PositionTween->TweenToPosition(RestingWorldPosition, SelectionTweenDuration);
}

void AChessPiece::MovePieceTo(const FVector TargetWorldPosition)
{
	RestingWorldPosition = TargetWorldPosition;
	bIsLifted = false;
	PositionTween->TweenToPosition(RestingWorldPosition, MoveTweenDuration);
}

void AChessPiece::ApplyPieceColor()
{
	UMaterialInterface* Material = nullptr;
	if (PieceColor == EChessCorePieceColor::White)
	{
		Material = WhiteMaterial;
	}
	else if (PieceColor == EChessCorePieceColor::Black)
	{
		Material = BlackMaterial;
	}

	if (!Material)
	{
		return;
	}

	const int32 MaterialCount = PieceMesh->GetNumMaterials();
	for (const int32 MaterialIndex : MaterialIndices)
	{
		if (MaterialIndex >= 0 && MaterialIndex < MaterialCount)
		{
			PieceMesh->SetMaterial(MaterialIndex, Material);
		}
	}
}
