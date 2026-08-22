#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ChessCoreTypes.h"
#include "MaterialAdvantageRow.generated.h"

class UChessGameState;
class UTextBlock;
class UTexture2D;
class UWrapBox;

UCLASS()
class HIGHSCHOOLCHESSCLUB_API UMaterialAdvantageRow : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="MaterialAdvantageRow")
	void SetPlayerColor(EChessCorePieceColor InPlayerColor);

	UFUNCTION(BlueprintCallable, Category="MaterialAdvantageRow")
	void RefreshFromGameState(UChessGameState* GameState);

private:
	void RefreshCapturedPieces(
		EChessCorePieceColor CapturedColor,
		const TMap<EChessCorePieceType, int32>& CapturedPieceCounts);
	UWrapBox* GetPieceBox(EChessCorePieceType Type) const;
	UTexture2D* GetPieceImage(EChessCorePieceColor Color, EChessCorePieceType Type) const;

	static int32 GetPieceValue(EChessCorePieceType Type);
	static EChessCorePieceColor GetOpponentColor(EChessCorePieceColor Color);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MaterialAdvantageRow", meta=(AllowPrivateAccess="true", ExposeOnSpawn="true"))
	EChessCorePieceColor PlayerColor = EChessCorePieceColor::White;

	UPROPERTY(EditDefaultsOnly, Category="MaterialAdvantageRow")
	TMap<EChessCorePieceType, TSoftObjectPtr<UTexture2D>> WhitePieceImages;

	UPROPERTY(EditDefaultsOnly, Category="MaterialAdvantageRow")
	TMap<EChessCorePieceType, TSoftObjectPtr<UTexture2D>> BlackPieceImages;

	UPROPERTY(EditDefaultsOnly, Category="MaterialAdvantageRow")
	FVector2D PieceImageSize = FVector2D(40.0f, 40.0f);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> Queen;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> Rook;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> Bishop;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> Knight;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UWrapBox> Pawn;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> MaterialAdvantageText;
};
