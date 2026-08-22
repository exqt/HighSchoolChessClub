#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameInfoPanel.generated.h"

class AChessMatch;
class UGridPanel;
class UChessGameState;
class UMaterialAdvantageRow;
struct FChessCoreMove;
class UButton;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlyNavigationRequested, int32, PlyIndex);

UCLASS(Abstract, Blueprintable)
class HIGHSCHOOLCHESSCLUB_API UGameInfoPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	/**
	 * 이번 보드 히스토리로 요청하는 함수 ply기준
	 * 초기 상태는 0 백의 첫수:1 흑의 첫수: 2 
	 */
	UFUNCTION(BlueprintCallable, Category="GameInfoPanel")
	void SetCurrentPlyIndex(int32 PlyIndex);

	UPROPERTY(BlueprintAssignable, Category="GameInfoPanel")
	FOnPlyNavigationRequested OnPlyNavigationRequested;

	UFUNCTION(BlueprintCallable, Category="GameInfoPanel")
	void AddPly(FChessCoreMove Ply);

	UFUNCTION(BlueprintCallable, Category="GameInfoPanel")
	void SetChessGameState(UChessGameState* InChessGameState);

	UFUNCTION(BlueprintCallable, Category="GameInfoPanel")
	void SetChessMatch(AChessMatch* InChessMatch);
	
	UFUNCTION(BlueprintCallable, Category="GameInfoPanel")
	void ResetAll();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="GameInfoPanel")
	TSubclassOf<UUserWidget> CellWidgetClass;

private:
	void RefreshMaterialAdvantage() const;

	TArray<FChessCoreMove> PlyHistory;

	UPROPERTY(Transient)
	TObjectPtr<UChessGameState> ChessGameState;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="GameInfoPanel", meta=(AllowPrivateAccess="true"))
	int32 CurrentPlyIndex = INDEX_NONE;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="GameInfoPanel", meta=(AllowPrivateAccess="true"))
	int32 LastPlyIndex = INDEX_NONE;

	UFUNCTION()
	void HandleFirstPlyClicked();

	UFUNCTION()
	void HandlePreviousPlyClicked();

	UFUNCTION()
	void HandleNextPlyClicked();

	UFUNCTION()
	void HandleLastPlyClicked();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UGridPanel> PlyListGrid;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> FirstPlyButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> PreviousPlyButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> NextPlyButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> LastPlyButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMaterialAdvantageRow> HumanMaterialAdvantageRow;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UMaterialAdvantageRow> NPCMaterialAdvantageRow;
};
