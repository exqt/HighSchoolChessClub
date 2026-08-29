// Copyright Epic Games, Inc. All Rights Reserved.

#include "ChessOpeningTree.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#include <algorithm>
#include <cctype>

#ifdef check
#pragma push_macro("check")
#undef check
#define CHESSBOTS_RESTORE_CHECK_MACRO 1
#endif

THIRD_PARTY_INCLUDES_START
#include "chess.hpp"
THIRD_PARTY_INCLUDES_END

#ifdef CHESSBOTS_RESTORE_CHECK_MACRO
#pragma pop_macro("check")
#undef CHESSBOTS_RESTORE_CHECK_MACRO
#endif

namespace
{
	std::string NormalizeEco(std::string Eco)
	{
		const auto IsWhitespace = [](const unsigned char Character) { return std::isspace(Character) != 0; };
		Eco.erase(Eco.begin(), std::find_if(Eco.begin(), Eco.end(), [&IsWhitespace](const unsigned char Character) { return !IsWhitespace(Character); }));
		Eco.erase(std::find_if(Eco.rbegin(), Eco.rend(), [&IsWhitespace](const unsigned char Character) { return !IsWhitespace(Character); }).base(), Eco.end());
		std::transform(Eco.begin(), Eco.end(), Eco.begin(), [](const unsigned char Character) { return static_cast<char>(std::toupper(Character)); });
		return Eco;
	}
}

const FChessOpeningTree& FChessOpeningTree::Get()
{
	static const FChessOpeningTree OpeningTree;
	return OpeningTree;
}

FChessOpeningTree::FChessOpeningTree()
{
	Nodes.emplace_back();
	chess::Board StartBoard;
	NodesByPosition[StartBoard.hash()].push_back(0);

	const TSharedPtr<IPlugin> ChessCorePlugin = IPluginManager::Get().FindPlugin(TEXT("ChessCore"));
	if (!ChessCorePlugin.IsValid())
	{
		return;
	}

	FString Contents;
	const FString ResourcePath = FPaths::Combine(ChessCorePlugin->GetBaseDir(), TEXT("Resources/chess-openings.tsv"));
	const FString SourcePath = FPaths::Combine(ChessCorePlugin->GetBaseDir(), TEXT("Source/ThirdParty/chess-openings.tsv"));
	if (!FFileHelper::LoadFileToString(Contents, *ResourcePath) && !FFileHelper::LoadFileToString(Contents, *SourcePath))
	{
		return;
	}

	TArray<FString> Lines;
	Contents.ParseIntoArrayLines(Lines, true);
	for (int32 LineIndex = 1; LineIndex < Lines.Num(); ++LineIndex)
	{
		TArray<FString> Fields;
		Lines[LineIndex].ParseIntoArray(Fields, TEXT("\t"), false);
		if (Fields.Num() < 4)
		{
			continue;
		}

		FString EcoField = Fields[0].TrimStartAndEnd();
		EcoField.ToUpperInline();
		TArray<FString> UciFields;
		Fields[3].ParseIntoArrayWS(UciFields);
		if (EcoField.IsEmpty() || UciFields.IsEmpty())
		{
			continue;
		}

		const FTCHARToUTF8 EcoUtf8(*EcoField);
		std::vector<std::string> Moves;
		Moves.reserve(UciFields.Num());
		for (const FString& UciField : UciFields)
		{
			const FTCHARToUTF8 UciUtf8(*UciField);
			Moves.emplace_back(UciUtf8.Get(), UciUtf8.Length());
		}
		InsertLine(std::string(EcoUtf8.Get(), EcoUtf8.Length()), Moves);
	}
}

void FChessOpeningTree::InsertLine(const std::string& Eco, const std::vector<std::string>& Moves)
{
	chess::Board Board;
	size_t NodeIndex = 0;
	for (const std::string& UciMove : Moves)
	{
		const chess::Move Move = chess::uci::uciToMove(Board, UciMove);
		chess::Movelist LegalMoves;
		chess::movegen::legalmoves(LegalMoves, Board);
		if (Move == chess::Move::NO_MOVE || std::find(LegalMoves.begin(), LegalMoves.end(), Move) == LegalMoves.end())
		{
			return;
		}

		auto ChildIterator = Nodes[NodeIndex].Children.find(UciMove);
		if (ChildIterator == Nodes[NodeIndex].Children.end())
		{
			const size_t ChildIndex = Nodes.size();
			Nodes.emplace_back();
			Nodes[NodeIndex].Children.emplace(UciMove, ChildIndex);
			ChildIterator = Nodes[NodeIndex].Children.find(UciMove);

			Board.makeMove(Move);
			NodesByPosition[Board.hash()].push_back(ChildIndex);
		}
		else
		{
			Board.makeMove(Move);
		}

		NodeIndex = ChildIterator->second;
		Nodes[NodeIndex].Ecos.insert(Eco);
	}
}

bool FChessOpeningTree::TrySelectMove(const chess::Board& Board, const std::vector<std::string>& PreferredEcos, std::mt19937_64& RandomGenerator, std::string& OutUciMove) const
{
	std::unordered_set<std::string> NormalizedEcos;
	for (const std::string& Eco : PreferredEcos)
	{
		const std::string NormalizedEco = NormalizeEco(Eco);
		if (!NormalizedEco.empty())
		{
			NormalizedEcos.insert(NormalizedEco);
		}
	}
	if (NormalizedEcos.empty())
	{
		return false;
	}

	const auto PositionIterator = NodesByPosition.find(Board.hash());
	if (PositionIterator == NodesByPosition.end())
	{
		return false;
	}

	chess::Movelist LegalMoves;
	chess::movegen::legalmoves(LegalMoves, Board);
	std::unordered_set<std::string> LegalUciMoves;
	for (const chess::Move Move : LegalMoves)
	{
		LegalUciMoves.insert(chess::uci::moveToUci(Move, Board.chess960()));
	}

	std::vector<std::string> Candidates;
	for (const size_t NodeIndex : PositionIterator->second)
	{
		for (const auto& [UciMove, ChildIndex] : Nodes[NodeIndex].Children)
		{
			const FNode& Child = Nodes[ChildIndex];
			const bool bMatchesPreferredEco = std::any_of(NormalizedEcos.begin(), NormalizedEcos.end(), [&Child](const std::string& Eco) { return Child.Ecos.contains(Eco); });
			if (bMatchesPreferredEco && LegalUciMoves.contains(UciMove))
			{
				Candidates.push_back(UciMove);
			}
		}
	}

	std::sort(Candidates.begin(), Candidates.end());
	Candidates.erase(std::unique(Candidates.begin(), Candidates.end()), Candidates.end());
	if (Candidates.empty())
	{
		return false;
	}

	std::uniform_int_distribution<size_t> Distribution(0, Candidates.size() - 1);
	OutUciMove = Candidates[Distribution(RandomGenerator)];
	return true;
}
