#include "Game/ChessPiece.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"

AChessPiece::AChessPiece()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Scene Root"));
	SetRootComponent(SceneRoot);

	PieceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Piece Mesh"));
	PieceMesh->SetupAttachment(SceneRoot);

	MaterialIndices.Add(0);
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
