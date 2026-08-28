#include "CCGameSettingRegistry.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "DataSource/GameSettingDataSourceDynamic.h"
#include "GameFramework/GameUserSettings.h"
#include "Player/CCLocalPlayer.h"

#define LOCTEXT_NAMESPACE "CCSettings"

UGameSettingCollection* UCCGameSettingRegistry::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
{
	UGameSettingCollection* Screen = NewObject<UGameSettingCollection>();
	Screen->SetDevName(TEXT("VideoCollection"));
	Screen->SetDisplayName(LOCTEXT("VideoCollection_Name", "Video"));
	Screen->Initialize(InLocalPlayer);

	UGameSettingCollection* Display = NewObject<UGameSettingCollection>();
	Display->SetDevName(TEXT("DisplayCollection"));
	Display->SetDisplayName(LOCTEXT("DisplayCollection_Name", "Display"));
	Screen->AddSetting(Display);

	UGameSettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameSettingValueDiscreteDynamic_Enum>();
	Setting->SetDevName(TEXT("WindowMode"));
	Setting->SetDisplayName(LOCTEXT("WindowMode_Name", "Window Mode"));
	Setting->SetDescriptionRichText(LOCTEXT("WindowMode_Description", "Choose whether the game runs in fullscreen or in a window."));
	Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFullscreenMode));
	Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFullscreenMode));
	Setting->SetDefaultValue(UGameUserSettings::GetDefaultWindowMode());
	Setting->AddEnumOption(EWindowMode::Fullscreen, LOCTEXT("WindowModeFullscreen", "Fullscreen"));
	Setting->AddEnumOption(EWindowMode::WindowedFullscreen, LOCTEXT("WindowModeWindowedFullscreen", "Borderless"));
	Setting->AddEnumOption(EWindowMode::Windowed, LOCTEXT("WindowModeWindowed", "Windowed"));
	Display->AddSetting(Setting);

	return Screen;
}

#undef LOCTEXT_NAMESPACE
