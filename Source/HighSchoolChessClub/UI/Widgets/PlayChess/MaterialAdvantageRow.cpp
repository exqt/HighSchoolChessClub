#include "UI/Widgets/PlayChess/MaterialAdvantageRow.h"

#include "ChessGameState.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WrapBox.h"

namespace
{
	const EChessCorePieceType DisplayPieceTypes[] =
	{
		EChessCorePieceType::Queen,
		EChessCorePieceType::Rook,
		EChessCorePieceType::Bishop,
		EChessCorePieceType::Knight,
		EChessCorePieceType::Pawn
	};
}

void UMaterialAdvantageRow::SetPlayerColor(const EChessCorePieceColor InPlayerColor)
{
	PlayerColor = InPlayerColor;
}

void UMaterialAdvantageRow::RefreshFromGameState(UChessGameState* GameState)
{
	TArray<FChessCorePiece> Pieces;
	TArray<FChessCorePiece> CapturedPieces;
	GameState->GetPieces(Pieces);
	GameState->GetCapturedPieces(CapturedPieces);

	const EChessCorePieceColor CapturedColor = GetOpponentColor(PlayerColor);
	TMap<EChessCorePieceType, int32> CapturedPieceCounts;
	int32 WhiteMaterial = 0;
	int32 BlackMaterial = 0;

	for (const FChessCorePiece& Piece : Pieces)
	{
		if (Piece.Color == EChessCorePieceColor::White)
		{
			WhiteMaterial += GetPieceValue(Piece.Type);
		}
		else
		{
			BlackMaterial += GetPieceValue(Piece.Type);
		}
	}

	for (const FChessCorePiece& CapturedPiece : CapturedPieces)
	{
		if (CapturedPiece.Color == CapturedColor)
		{
			++CapturedPieceCounts.FindOrAdd(CapturedPiece.Type);
		}
	}

	RefreshCapturedPieces(CapturedColor, CapturedPieceCounts);

	const int32 PlayerMaterial = PlayerColor == EChessCorePieceColor::White
		? WhiteMaterial
		: BlackMaterial;
	const int32 OpponentMaterial = PlayerColor == EChessCorePieceColor::White
		? BlackMaterial
		: WhiteMaterial;
	const int32 MaterialAdvantage = FMath::Max(0, PlayerMaterial - OpponentMaterial);
	MaterialAdvantageText->SetText(MaterialAdvantage > 0
		? FText::FromString(FString::Printf(TEXT("+%d"), MaterialAdvantage))
		: FText::GetEmpty());
}

void UMaterialAdvantageRow::RefreshCapturedPieces(
	const EChessCorePieceColor CapturedColor,
	const TMap<EChessCorePieceType, int32>& CapturedPieceCounts)
{
	for (const EChessCorePieceType Type : DisplayPieceTypes)
	{
		UWrapBox* PieceBox = GetPieceBox(Type);
		PieceBox->ClearChildren();

		for (int32 Index = 0; Index < CapturedPieceCounts.FindRef(Type); ++Index)
		{
			UImage* PieceImage = NewObject<UImage>(this);
			PieceImage->SetBrushFromTexture(GetPieceImage(CapturedColor, Type), true);
			PieceBox->AddChildToWrapBox(PieceImage);
			PieceImage->SetDesiredSizeOverride(PieceImageSize);
		}
	}
}

UWrapBox* UMaterialAdvantageRow::GetPieceBox(const EChessCorePieceType Type) const
{
	switch (Type)
	{
	case EChessCorePieceType::Queen: return Queen;
	case EChessCorePieceType::Rook: return Rook;
	case EChessCorePieceType::Bishop: return Bishop;
	case EChessCorePieceType::Knight: return Knight;
	case EChessCorePieceType::Pawn: return Pawn;
	default: return nullptr;
	}
}

UTexture2D* UMaterialAdvantageRow::GetPieceImage(
	const EChessCorePieceColor Color,
	const EChessCorePieceType Type) const
{
	const TMap<EChessCorePieceType, TSoftObjectPtr<UTexture2D>>& PieceImages =
		Color == EChessCorePieceColor::White ? WhitePieceImages : BlackPieceImages;

	return PieceImages.FindChecked(Type).LoadSynchronous();
}

int32 UMaterialAdvantageRow::GetPieceValue(const EChessCorePieceType Type)
{
	switch (Type)
	{
	case EChessCorePieceType::Queen: return 9;
	case EChessCorePieceType::Rook: return 5;
	case EChessCorePieceType::Bishop:
	case EChessCorePieceType::Knight: return 3;
	case EChessCorePieceType::Pawn: return 1;
	default: return 0;
	}
}

EChessCorePieceColor UMaterialAdvantageRow::GetOpponentColor(const EChessCorePieceColor Color)
{
	return Color == EChessCorePieceColor::White
		? EChessCorePieceColor::Black
		: EChessCorePieceColor::White;
}
