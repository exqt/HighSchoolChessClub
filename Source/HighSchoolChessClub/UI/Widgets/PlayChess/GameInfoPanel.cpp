#include "UI/Widgets/PlayChess/GameInfoPanel.h"

#include "ChessCoreTypes.h"
#include "Components/UniformGridSlot.h"
#include "InputCoreTypes.h"
#include "Components/GridPanel.h"
#include "Components/GridSlot.h"
#include "Game/ChessMatch.h"
#include "UI/Widgets/PlayChess/MaterialAdvantageRow.h"

void UGameInfoPanel::SetCurrentPlyIndex(const int32 PlyIndex)
{
	CurrentPlyIndex = FMath::Clamp(PlyIndex, 0, LastPlyIndex);
	OnPlyNavigationRequested.Broadcast(CurrentPlyIndex);
}

void UGameInfoPanel::AddPly(FChessCoreMove Ply)
{
	PlyHistory.Add(Ply);
	
    UUserWidget* CellWidget = CreateWidget<UUserWidget>(
        GetOwningPlayer(),
        CellWidgetClass
    );

    UGridSlot* GridSlot = PlyListGrid->AddChildToGrid(CellWidget);
	
	const int Index = PlyHistory.Num() - 1;
	const int X = Index % 2 + 1;
	const int Y = Index / 2 + 1;

    GridSlot->SetColumn(X);
    GridSlot->SetRow(Y);    

    GridSlot->SetColumnSpan(1);
    GridSlot->SetRowSpan(1);
    GridSlot->SetHorizontalAlignment(HAlign_Fill);
    GridSlot->SetVerticalAlignment(VAlign_Fill);

	RefreshMaterialAdvantage();
}

void UGameInfoPanel::SetChessGameState(UChessGameState* InChessGameState)
{
	ChessGameState = InChessGameState;
	RefreshMaterialAdvantage();
}

void UGameInfoPanel::SetChessMatch(AChessMatch* InChessMatch)
{
	ChessGameState = InChessMatch->GetChessState();

	const EChessCorePieceColor HumanColor = InChessMatch->GetHumanPlayerColor();
	const EChessCorePieceColor NPCColor = HumanColor == EChessCorePieceColor::White
		? EChessCorePieceColor::Black
		: EChessCorePieceColor::White;

	HumanMaterialAdvantageRow->SetPlayerColor(HumanColor);
	NPCMaterialAdvantageRow->SetPlayerColor(NPCColor);
	RefreshMaterialAdvantage();
}

void UGameInfoPanel::ResetAll()
{
	PlyHistory.Empty();
	RefreshMaterialAdvantage();
}

void UGameInfoPanel::RefreshMaterialAdvantage() const
{
	if (!ChessGameState)
	{
		return;
	}

	HumanMaterialAdvantageRow->RefreshFromGameState(ChessGameState);
	NPCMaterialAdvantageRow->RefreshFromGameState(ChessGameState);
}

void UGameInfoPanel::HandleFirstPlyClicked()
{
	SetCurrentPlyIndex(0);
}

void UGameInfoPanel::HandlePreviousPlyClicked()
{
	SetCurrentPlyIndex(CurrentPlyIndex - 1);
}

void UGameInfoPanel::HandleNextPlyClicked()
{
	SetCurrentPlyIndex(CurrentPlyIndex + 1);
}

void UGameInfoPanel::HandleLastPlyClicked()
{
	SetCurrentPlyIndex(LastPlyIndex);
}
