#include "guis/GuiMenu.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiCollectionSystemsOptions.h"
#include "guis/GuiDetectDevice.h"
#include "guis/GuiGeneralScreensaverOptions.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiScraperStart.h"
#include "guis/GuiPackageInstaller.h" //351elec
#include "guis/GuiDecorationOptions.h" //351elec
#include "guis/GuiHashStart.h"
#include "guis/GuiThemeInstaller.h" //batocera
#include "guis/GuiBezelInstaller.h" //batocera
#include "guis/GuiBatoceraStore.h" //batocera
#include "guis/GuiSettings.h"
#include "guis/GuiRetroAchievements.h" //batocera
#include "guis/GuiGamelistOptions.h"
#include "guis/GuiImageViewer.h"
#include "guis/GuiMoonlight.h"
#include "views/UIModeController.h"
#include "views/ViewController.h"
#include "CollectionSystemManager.h"
#include "EmulationStation.h"
#include "Scripting.h"
#include "SystemData.h"
#include "VolumeControl.h"
#include "BrightnessControl.h"
#include <SDL_events.h>
#include <algorithm>
#include "platform.h"

#include "SystemConf.h"
#include "ApiSystem.h"
#include "InputManager.h"
#include "AudioManager.h"
#include <LibretroRatio.h>
#include "guis/GuiUpdate.h"
#include "guis/GuiInstallStart.h"
#include "guis/GuiTextEditPopupKeyboard.h"
#include "guis/GuiBackupStart.h"
#include "guis/GuiTextEditPopup.h"
#include "guis/GuiWifi.h"
#include "guis/GuiBluetooth.h"
#include "scrapers/ThreadedScraper.h"
#include "FileSorts.h"
#include "ThreadedHasher.h"
#include "ThreadedBluetooth.h"
#include "views/gamelist/IGameListView.h"
#include "components/MultiLineMenuEntry.h"
#include "components/BatteryIndicatorComponent.h"
#include "GuiLoading.h"
#include "guis/GuiBios.h"
#include "guis/GuiKeyMappingEditor.h"
#include "Gamelist.h"
#include "TextToSpeech.h"

#if WIN32
#include "Win32ApiSystem.h"
#endif

#define fake_gettext_fade _("fade")
#define fake_gettext_slide _("slide")
#define fake_gettext_instant _("instant")
#define fake_gettext_fadeslide _("fade & slide")

#define fake_gettext_system       _("System")
#define fake_gettext_architecture _("Architecture")
#define fake_gettext_diskformat   _("Disk format")
#define fake_gettext_temperature  _("Temperature")
#define fake_gettext_avail_memory _("Available memory")
#define fake_gettext_battery      _("Battery")
#define fake_gettext_model        _("Model")
#define fake_gettext_cpu_model    _("Cpu model")
#define fake_gettext_cpu_number   _("Cpu number")
#define fake_gettext_cpu_frequency _("Cpu max frequency")
#define fake_gettext_cpu_feature  _("Cpu feature")

#define fake_gettext_scanlines		_("SCANLINES")
#define fake_gettext_retro			_("RETRO")
#define fake_gettext_enhanced		_("ENHANCED")
#define fake_gettext_curvature		_("CURVATURE")
#define fake_gettext_zfast			_("ZFAST")
#define fake_gettext_flatten_glow	_("FLATTEN-GLOW")
#define fake_gettext_rgascaling		_("RGA SCALING")

#define fake_gettext_glvendor		_("VENDOR")
#define fake_gettext_glvrenderer	_("RENDERER")
#define fake_gettext_glversion		_("VERSION")
#define fake_gettext_glslversion	_("SHADERS")

#define gettext_controllers_settings				_("CONTROLLER SETTINGS")
#define gettext_controllers_and_bluetooth_settings  _("CONTROLLER & BLUETOOTH SETTINGS")

// Windows build does not have bluetooth support, so affect the label for Windows
#if WIN32
#define controllers_settings_label		gettext_controllers_settings
#else
#define controllers_settings_label		gettext_controllers_and_bluetooth_settings
#endif

std::string GetEnv( const std::string & var ) {
     const char * val = std::getenv( var.c_str() );
     if ( val == nullptr ) { 
         return "";
     }
     else {
         return val;
     }
}

GuiMenu::GuiMenu(Window *window, bool animate) : GuiComponent(window), mMenu(window, _("MAIN MENU").c_str()), mVersion(window)
{
	// MAIN MENU
	bool isFullUI = UIModeController::getInstance()->isUIModeFull();
	bool isKidUI = UIModeController::getInstance()->isUIModeKid();

	if (isFullUI)
	{
#if !defined(WIN32) || defined(_DEBUG)
		addEntry(_("GAME SETTINGS").c_str(), true, [this] { openGamesSettings_batocera(); }, "iconGames");
		addEntry(_("GAME COLLECTION SETTINGS").c_str(), true, [this] { openCollectionSystemSettings(); }, "iconAdvanced");

		if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::RETROACHIVEMENTS) &&
		    SystemConf::getInstance()->getBool("global.retroachievements") &&
		    Settings::getInstance()->getBool("RetroachievementsMenuitem") &&
		    SystemConf::getInstance()->get("global.retroachievements.username") != "") {
			addEntry(_("RETROACHIEVEMENTS").c_str(), true, [this] {
			if (!checkNetwork()) {
				return;
			}
			GuiRetroAchievements::show(mWindow); }, "iconRetroachievements");
		}

		addEntry(_("SYSTEM SETTINGS").c_str(), true, [this] { openSystemSettings_batocera(); }, "iconSystem");
		addEntry(_("UI SETTINGS").c_str(), true, [this] { openUISettings(); }, "iconUI");
		addEntry(controllers_settings_label.c_str(), true, [this] { openControllersSettings_batocera(); }, "iconControllers");
		addEntry(_("SOUND SETTINGS").c_str(), true, [this] { openSoundSettings(); }, "iconSound");

		if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::WIFI)) {
			addEntry(_("NETWORK SETTINGS").c_str(), true, [this] { openNetworkSettings_batocera(); }, "iconNetwork");
#if defined(AMD64) || defined(RK3326) || defined(RK3566) || defined(RK3566_X55) || defined(RK3588) || defined(RK3588_ACE) ||defined(RK3399)
		  addEntry(_("MOONLIGHT GAME STREAMING").c_str(), true, [this] { GuiMoonlight::show(mWindow); }, "iconGames");
#endif
		}
#else
		if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::GAMESETTINGS))
			addEntry(_("GAME SETTINGS").c_str(), true, [this] { openGamesSettings_batocera(); }, "iconGames");

		addEntry(_("UI SETTINGS").c_str(), true, [this] { openUISettings(); }, "iconUI");

		if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::GAMESETTINGS))
			addEntry(controllers_settings_label.c_str(), true, [this] { openControllersSettings_batocera(); }, "iconControllers");
		else
			addEntry(_("CONFIGURE INPUT"), true, [this] { openConfigInput(); }, "iconControllers");

		addEntry(_("SOUND SETTINGS").c_str(), true, [this] { openSoundSettings(); }, "iconSound");
		addEntry(_("GAME COLLECTION SETTINGS").c_str(), true, [this] { openCollectionSystemSettings(); }, "iconAdvanced");

		if (!ApiSystem::getInstance()->isScriptingSupported(ApiSystem::GAMESETTINGS))
		{
			for (auto system : SystemData::sSystemVector)
			{
				if (system->isCollection() || system->getEmulators().size() == 0 || (system->getEmulators().size() == 1 && system->getEmulators().begin()->cores.size() <= 1))
					continue;

				addEntry(_("EMULATOR SETTINGS"), true, [this] { openEmulatorSettings(); }, "iconGames");
				break;
			}
		}
#endif

		addEntry(_("SCRAPER").c_str(), true, [this] { openScraperSettings(); }, "iconScraper");
//		addEntry(_("UPDATES & DOWNLOADS"), true, [this] { openUpdatesSettings(); }, "iconUpdates");
	}
	else
	{


		addEntry(_("INFORMATION").c_str(), true, [this] { openSystemInformations_batocera(); }, "iconSystem");
		addEntry(_("UNLOCK UI MODE").c_str(), true, [this] { exitKidMode(); }, "iconAdvanced");
	}

	if (!isKidUI) {
		addEntry(_("QUIT").c_str(), true, [this] { openQuitMenu_batocera(); }, "iconQuit");
	}

	addChild(&mMenu);
	addVersionInfo(); 
	setSize(mMenu.getSize());

	if (animate)
	{
		if (Renderer::isSmallScreen())
			animateTo(Vector2f((Renderer::getScreenWidth() - getSize().x()) / 2, (Renderer::getScreenHeight() - getSize().y()) / 2));
		else
			animateTo(Vector2f((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f));
	}
	else
	{
		if (Renderer::isSmallScreen())
			setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, (Renderer::getScreenHeight() - mSize.y()) / 2);
		else
			setPosition((Renderer::getScreenWidth() - mSize.x()) / 2, Renderer::getScreenHeight() * 0.15f);
	}
}

void GuiMenu::openResetOptions(Window* mWindow, std::string configName)
{

	GuiSettings* resetOptions = new GuiSettings(mWindow, _("SYSTEM MANAGEMENT AND RESET").c_str());

    resetOptions->addGroup(_("DATA MANAGEMENT"));
    resetOptions->addEntry(_("BACKUP CONFIGURATIONS"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING THIS WILL RESTART EMULATIONSTATION!\n\nAFTER THE SCRIPT IS DONE REMEMBER TO COPY THE FILE /storage/roms/backup/JELOS_BACKUP.zip TO SOME PLACE SAFE OR IT WILL BE DELETED ON NEXT REBOOT!\n\nBACKUP CURRENT CONFIG AND RESTART?"), _("YES"),
				[] {
				runSystemCommand("/usr/bin/run \"/usr/bin/backuptool backup\"", "", nullptr);
				}, _("NO"), nullptr));
     });

    resetOptions->addEntry(_("RESTORE FROM BACKUP"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING THIS WILL REBOOT YOUR DEVICE!\n\nYOUR EXISTING CONFIGURATION WILL BE OVERWRITTEN!\n\nRESTORE FROM BACKUP AND RESTART?"), _("YES"),
				[] {
				runSystemCommand("/usr/bin/run \"/usr/bin/backuptool restore\"", "", nullptr);
				}, _("NO"), nullptr));
     });

    resetOptions->addEntry(_("DELETE EMPTY GAME DIRECTORIES"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING THIS WILL REBOOT YOUR DEVICE!\n\nDELETE EMPTY GAME DIRECTORIES??"
), _("YES"),
                                [] {
                                runSystemCommand("/usr/bin/run \"/usr/bin/cleanup_overlay\"", "", nullptr);
                                }, _("NO"), nullptr));
     });

    resetOptions->addEntry(_("CLEAN GAMELISTS & REMOVE UNUSED MEDIA"), true, [mWindow] {
	mWindow->pushGui(new GuiMsgBox(mWindow, _("ARE YOU SURE?"), _("YES"), [&]
	{
		int idx = 0;
		for (auto system : SystemData::sSystemVector)
		{
			cleanupGamelist(system);
			idx++;
		}
	}, _("NO"), nullptr));
      });

    resetOptions->addGroup(_("EMULATOR MANAGEMENT"));
    resetOptions->addEntry(_("RESET RETROARCH CONFIG TO DEFAULT"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: RETROARCH CONFIG WILL RESET TO DEFAULT\n\nPER-CORE CONFIGURATIONS WILL NOT BE AFFECTED AND NO BACKUP WILL BE CREATED!\n\nRESET RETROARCH CONFIG TO DEFAULT?"), _("YES"),
				[] {
				runSystemCommand("/usr/bin/run \"/usr/bin/factoryreset retroarch\"", "", nullptr);
				}, _("NO"), nullptr));
     });

    resetOptions->addEntry(_("RESET OVERLAYS (CORES, CHEATS, JOYPADS, ETC)"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: ALL CUSTOM RETROARCH OVERLAYS WILL BE REMOVED\n\nCUSTOM CORES, JOYSTICKS, CHEATS, ETC. NO BACKUP WILL BE CREATED!\n\nRESET RETROARCH OVERLAYS TO DEFAULT?"), _("YES"),
                                [] {
                                runSystemCommand("/usr/bin/run \"/usr/bin/factoryreset overlays\"", "", nullptr);
                                }, _("NO"), nullptr));
     });

    resetOptions->addEntry(_("FULLY RESET RETROARCH"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: RETROARCH AND ALL USER SAVED CONFIGURATIONS WILL RESET TO DEFAULT\n\nPER-CORE CONFIGURATIONS WILL BE REMOVED AND NO BACKUP WILL BE CREATED!\n\nRESET RETROARCH?"), _("YES"),
                                [] {
                                runSystemCommand("/usr/bin/run \"/usr/bin/factoryreset retroarch-full && /usr/bin/factoryreset overlays\"", "", nullptr);
                                }, _("NO"), nullptr));
     });

//Only show on devices that currently support Mednafen
#if defined(AMD64) || defined(RK3326) || defined(RK3399) || defined(RK3588)  || defined(RK3588_ACE)
    resetOptions->addEntry(_("RESET MEDNAFEN CONFIG TO DEFAULT"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: MEDNAFEN CONFIG WILL RESET TO DEFAULT\n\nNO BACKUP WILL BE CREATED!\n\nRESET MEDNAFEN CONFIG TO DEFAULT?"), _("YES"),
				[] {
				runSystemCommand("/usr/bin/run \"/usr/bin/factoryreset mednafen\"", "", nullptr);
				}, _("NO"), nullptr));
     });
#endif

    resetOptions->addGroup(_("SYSTEM MANAGEMENT"));

    resetOptions->addEntry(_("AUDIO RESET"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: AUDIO SETTINGS WILL BE RESET TO DEFAULTS AND THE SYSTEM WILL REBOOT!\n\nRESET AUDIO AND RESTART?"), _("YES"),
                                [] {
                                runSystemCommand("/usr/bin/run \"/usr/bin/factoryreset audio\"", "", nullptr);
                                }, _("NO"), nullptr));
    });

    resetOptions->addEntry(_("FACTORY RESET"), true, [mWindow] {
    mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: YOUR DATA AND ALL OTHER CONFIGURATIONS WILL BE RESET TO DEFAULTS!\n\nIF YOU WANT TO KEEP YOUR SETTINGS MAKE A BACKUP AND SAVE IT ON AN EXTERNAL DRIVE BEFORE RUNING THIS OPTION!\n\nEJECT YOUR GAME CARD BEFORE PROCEEDING!\n\nRESET SYSTEM AND RESTART?"), _("YES"),
				[] {
				runSystemCommand("/usr/bin/run \"/usr/bin/factoryreset ALL\"", "", nullptr);
				}, _("NO"), nullptr));
     });

mWindow->pushGui(resetOptions);
}


void GuiMenu::openScraperSettings()
{
	// scrape now
	ComponentListRow row;
	auto openScrapeNow = [this]
	{
		if (!checkNetwork())
			return;

		if (ThreadedScraper::isRunning())
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, _("SCRAPER IS RUNNING. DO YOU WANT TO STOP IT?"), _("YES"), [this]
			{
				ThreadedScraper::stop();
			}, _("NO"), nullptr));

			return;
		}

		mWindow->pushGui(new GuiScraperStart(mWindow));
	};

	auto s = new GuiSettings(mWindow,
		_("SCRAPER"),
		_("NEXT"), [openScrapeNow](GuiSettings* settings)
	{
		settings->save();
		openScrapeNow();
	});

	std::string scraper = Settings::getInstance()->getString("Scraper");

	// scrape from
	auto scraper_list = std::make_shared< OptionListComponent< std::string > >(mWindow, _("SCRAPING DATABASE"), false);
	std::vector<std::string> scrapers = Scraper::getScraperList();

	// Select either the first entry of the one read from the settings, just in case the scraper from settings has vanished.
	for(auto it = scrapers.cbegin(); it != scrapers.cend(); it++)
		scraper_list->add(*it, *it, *it == scraper);

	s->addWithLabel(_("SCRAPING DATABASE"), scraper_list); 
	s->addSaveFunc([scraper_list] { Settings::getInstance()->setString("Scraper", scraper_list->getSelected()); });

	if (!scraper_list->hasSelection())
	{
		scraper_list->selectFirstItem();
		scraper = scraper_list->getSelected();
	}

	if (scraper == "ScreenScraper")
	{
		// Image source : <image> tag
		std::string imageSourceName = Settings::getInstance()->getString("ScrapperImageSrc");
		auto imageSource = std::make_shared< OptionListComponent<std::string> >(mWindow, _("IMAGE SOURCE"), false);
		imageSource->add(_("SCREENSHOT"), "ss", imageSourceName == "ss");
		imageSource->add(_("TITLE SCREENSHOT"), "sstitle", imageSourceName == "sstitle");
		imageSource->add(_("MIX V1"), "mixrbv1", imageSourceName == "mixrbv1");
		imageSource->add(_("MIX V2"), "mixrbv2", imageSourceName == "mixrbv2");
		imageSource->add(_("BOX 2D"), "box-2D", imageSourceName == "box-2D");
		imageSource->add(_("BOX 3D"), "box-3D", imageSourceName == "box-3D");
		imageSource->add(_("FAN ART"), "fanart", imageSourceName == "fanart");
		imageSource->add(_("NONE"), "", imageSourceName.empty());

		if (!imageSource->hasSelection())
			imageSource->selectFirstItem();

		s->addWithLabel(_("IMAGE SOURCE"), imageSource);
		s->addSaveFunc([imageSource] { Settings::getInstance()->setString("ScrapperImageSrc", imageSource->getSelected()); });

		// Box source : <thumbnail> tag
		std::string thumbSourceName = Settings::getInstance()->getString("ScrapperThumbSrc");
		auto thumbSource = std::make_shared< OptionListComponent<std::string> >(mWindow, _("BOX SOURCE"), false);
		thumbSource->add(_("NONE"), "", thumbSourceName.empty());
		thumbSource->add(_("BOX 2D"), "box-2D", thumbSourceName == "box-2D");
		thumbSource->add(_("BOX 3D"), "box-3D", thumbSourceName == "box-3D");

		if (!thumbSource->hasSelection())
			thumbSource->selectFirstItem();

		s->addWithLabel(_("BOX SOURCE"), thumbSource);
		s->addSaveFunc([thumbSource] { Settings::getInstance()->setString("ScrapperThumbSrc", thumbSource->getSelected()); });

		imageSource->setSelectedChangedCallback([this, thumbSource](std::string value)
		{
			if (value == "box-2D")
				thumbSource->remove(_("BOX 2D"));
			else
				thumbSource->add(_("BOX 2D"), "box-2D", false);

			if (value == "box-3D")
				thumbSource->remove(_("BOX 3D"));
			else
				thumbSource->add(_("BOX 3D"), "box-3D", false);
		});

		// Logo source : <marquee> tag
		std::string logoSourceName = Settings::getInstance()->getString("ScrapperLogoSrc");
		auto logoSource = std::make_shared< OptionListComponent<std::string> >(mWindow, _("LOGO SOURCE"), false);
		logoSource->add(_("NONE"), "", logoSourceName.empty());
		logoSource->add(_("WHEEL"), "wheel", logoSourceName == "wheel");
		logoSource->add(_("MARQUEE"), "marquee", logoSourceName == "marquee");

		if (!logoSource->hasSelection())
			logoSource->selectFirstItem();

		s->addWithLabel(_("LOGO SOURCE"), logoSource);
		s->addSaveFunc([logoSource] { Settings::getInstance()->setString("ScrapperLogoSrc", logoSource->getSelected()); });

		// scrape ratings
		auto scrape_ratings = std::make_shared<SwitchComponent>(mWindow);
		scrape_ratings->setState(Settings::getInstance()->getBool("ScrapeRatings"));
		s->addWithLabel(_("SCRAPE RATINGS"), scrape_ratings); 
		s->addSaveFunc([scrape_ratings] { Settings::getInstance()->setBool("ScrapeRatings", scrape_ratings->getState()); });

		// scrape video
		auto scrape_video = std::make_shared<SwitchComponent>(mWindow);
		scrape_video->setState(Settings::getInstance()->getBool("ScrapeVideos"));
		s->addWithLabel(_("SCRAPE VIDEOS"), scrape_video);
		s->addSaveFunc([scrape_video] { Settings::getInstance()->setBool("ScrapeVideos", scrape_video->getState()); });

		// SCRAPE FANART
		auto scrape_fanart = std::make_shared<SwitchComponent>(mWindow);
		scrape_fanart->setState(Settings::getInstance()->getBool("ScrapeFanart"));
		s->addWithLabel(_("SCRAPE FANART"), scrape_fanart);
		s->addSaveFunc([scrape_fanart] { Settings::getInstance()->setBool("ScrapeFanart", scrape_fanart->getState()); });

		// SCRAPE BOX BACKSIDE
		auto scrape_boxBack = std::make_shared<SwitchComponent>(mWindow);
		scrape_boxBack->setState(Settings::getInstance()->getBool("ScrapeBoxBack"));
		s->addWithLabel(_("SCRAPE BOX BACKSIDE"), scrape_boxBack);
		s->addSaveFunc([scrape_boxBack] { Settings::getInstance()->setBool("ScrapeBoxBack", scrape_boxBack->getState()); });

		// SCRAPE MAP
		auto scrape_map = std::make_shared<SwitchComponent>(mWindow);
		scrape_map->setState(Settings::getInstance()->getBool("ScrapeMap"));
		s->addWithLabel(_("SCRAPE MAP"), scrape_map);
		s->addSaveFunc([scrape_map] { Settings::getInstance()->setBool("ScrapeMap", scrape_map->getState()); });

		// SCRAPE TITLESHOT
		auto scrape_titleshot = std::make_shared<SwitchComponent>(mWindow);
		scrape_titleshot->setState(Settings::getInstance()->getBool("ScrapeTitleShot"));
		s->addWithLabel(_("SCRAPE TITLESHOT"), scrape_titleshot);
		s->addSaveFunc([scrape_titleshot] { Settings::getInstance()->setBool("ScrapeTitleShot", scrape_titleshot->getState()); });

		// SCRAPE CARTRIDGE
		auto scrape_cartridge = std::make_shared<SwitchComponent>(mWindow);
		scrape_cartridge->setState(Settings::getInstance()->getBool("ScrapeCartridge"));
		s->addWithLabel(_("SCRAPE CARTRIDGE"), scrape_cartridge);
		s->addSaveFunc([scrape_cartridge] { Settings::getInstance()->setBool("ScrapeCartridge", scrape_cartridge->getState()); });

		// SCRAPE MANUAL
		auto scrape_manual = std::make_shared<SwitchComponent>(mWindow);
		scrape_manual->setState(Settings::getInstance()->getBool("ScrapeManual"));
		s->addWithLabel(_("SCRAPE MANUAL"), scrape_manual);
		s->addSaveFunc([scrape_manual] { Settings::getInstance()->setBool("ScrapeManual", scrape_manual->getState()); });

		// SCRAPE PAD TO KEYBOARD
		//auto scrapePadToKey = std::make_shared<SwitchComponent>(mWindow);
		//scrapePadToKey->setState(Settings::getInstance()->getBool("ScrapePadToKey"));
		//s->addWithLabel(_("SCRAPE PADTOKEY SETTINGS"), scrapePadToKey);
		//s->addSaveFunc([scrapePadToKey] { Settings::getInstance()->setBool("ScrapePadToKey", scrapePadToKey->getState()); });

		// Account
		s->addInputTextRow(_("USERNAME"), "ScreenScraperUser", false, true);
		s->addInputTextRow(_("PASSWORD"), "ScreenScraperPass", true, true);
	}
	else
	{
		std::string imageSourceName = Settings::getInstance()->getString("ScrapperImageSrc");
		auto imageSource = std::make_shared< OptionListComponent<std::string> >(mWindow, _("IMAGE SOURCE"), false);

		// Image source : <image> tag

		imageSource->add(_("SCREENSHOT"), "ss", imageSourceName == "ss");
		imageSource->add(_("TITLE SCREENSHOT"), "sstitle", imageSourceName == "sstitle");
		imageSource->add(_("BOX 2D"), "box-2D", imageSourceName == "box-2D");
		imageSource->add(_("FAN ART"), "fanart", imageSourceName == "fanart");

		if (!imageSource->hasSelection())
			imageSource->selectFirstItem();

		s->addWithLabel(_("IMAGE SOURCE"), imageSource);
		s->addSaveFunc([imageSource] { Settings::getInstance()->setString("ScrapperImageSrc", imageSource->getSelected()); });

		// Box source : <thumbnail> tag
		std::string thumbSourceName = Settings::getInstance()->getString("ScrapperThumbSrc");
		auto thumbSource = std::make_shared< OptionListComponent<std::string> >(mWindow, _("BOX SOURCE"), false);
		thumbSource->add(_("NONE"), "", thumbSourceName.empty());
		thumbSource->add(_("BOX 2D"), "box-2D", thumbSourceName == "box-2D");

		if (scraper == "HfsDB")
			thumbSource->add(_("BOX 3D"), "box-3D", thumbSourceName == "box-3D");

		if (!thumbSource->hasSelection())
			thumbSource->selectFirstItem();

		s->addWithLabel(_("BOX SOURCE"), thumbSource);
		s->addSaveFunc([thumbSource] { Settings::getInstance()->setString("ScrapperThumbSrc", thumbSource->getSelected()); });

		imageSource->setSelectedChangedCallback([this, thumbSource](std::string value)
		{
			if (value == "box-2D")
				thumbSource->remove(_("BOX 2D"));
			else
				thumbSource->add(_("BOX 2D"), "box-2D", false);
		});

		// Logo source : <marquee> tag
		std::string logoSourceName = Settings::getInstance()->getString("ScrapperLogoSrc");
		auto logoSource = std::make_shared< OptionListComponent<std::string> >(mWindow, _("LOGO SOURCE"), false);
		logoSource->add(_("NONE"), "", logoSourceName.empty());
		logoSource->add(_("WHEEL"), "wheel", logoSourceName == "wheel");

		if (scraper == "HfsDB")
			logoSource->add(_("MARQUEE"), "marquee", logoSourceName == "marquee");

		if (!logoSource->hasSelection())
			logoSource->selectFirstItem();

		s->addWithLabel(_("LOGO SOURCE"), logoSource);
		s->addSaveFunc([logoSource] { Settings::getInstance()->setString("ScrapperLogoSrc", logoSource->getSelected()); });

		if (scraper == "TheGamesDB")
		{
			// SCRAPE FANART
			auto scrape_fanart = std::make_shared<SwitchComponent>(mWindow);
			scrape_fanart->setState(Settings::getInstance()->getBool("ScrapeFanart"));
			s->addWithLabel(_("SCRAPE FANART"), scrape_fanart);
			s->addSaveFunc([scrape_fanart] { Settings::getInstance()->setBool("ScrapeFanart", scrape_fanart->getState()); });

			// SCRAPE BOX BACKSIDE
			auto scrape_boxBack = std::make_shared<SwitchComponent>(mWindow);
			scrape_boxBack->setState(Settings::getInstance()->getBool("ScrapeBoxBack"));
			s->addWithLabel(_("SCRAPE BOX BACKSIDE"), scrape_boxBack);
			s->addSaveFunc([scrape_boxBack] { Settings::getInstance()->setBool("ScrapeBoxBack", scrape_boxBack->getState()); });
		}
		else if (scraper == "HfsDB")
		{
			// SCRAPE FANART
			auto scrape_fanart = std::make_shared<SwitchComponent>(mWindow);
			scrape_fanart->setState(Settings::getInstance()->getBool("ScrapeFanart"));
			s->addWithLabel(_("SCRAPE FANART"), scrape_fanart);
			s->addSaveFunc([scrape_fanart] { Settings::getInstance()->setBool("ScrapeFanart", scrape_fanart->getState()); });

			// scrape video
			auto scrape_video = std::make_shared<SwitchComponent>(mWindow);
			scrape_video->setState(Settings::getInstance()->getBool("ScrapeVideos"));
			s->addWithLabel(_("SCRAPE VIDEOS"), scrape_video);
			s->addSaveFunc([scrape_video] { Settings::getInstance()->setBool("ScrapeVideos", scrape_video->getState()); });

			// SCRAPE BOX BACKSIDE
			auto scrape_boxBack = std::make_shared<SwitchComponent>(mWindow);
			scrape_boxBack->setState(Settings::getInstance()->getBool("ScrapeBoxBack"));
			s->addWithLabel(_("SCRAPE BOX BACKSIDE"), scrape_boxBack);
			s->addSaveFunc([scrape_boxBack] { Settings::getInstance()->setBool("ScrapeBoxBack", scrape_boxBack->getState()); });

			// SCRAPE MANUAL
			auto scrape_manual = std::make_shared<SwitchComponent>(mWindow);
			scrape_manual->setState(Settings::getInstance()->getBool("ScrapeManual"));
			s->addWithLabel(_("SCRAPE MANUAL"), scrape_manual);
			s->addSaveFunc([scrape_manual] { Settings::getInstance()->setBool("ScrapeManual", scrape_manual->getState()); });
		}
		else
		{		// scrape video
			auto scrape_video = std::make_shared<SwitchComponent>(mWindow);
			scrape_video->setState(Settings::getInstance()->getBool("ScrapeVideos"));
			s->addWithLabel(_("SCRAPE VIDEOS"), scrape_video);
			s->addSaveFunc([scrape_video] { Settings::getInstance()->setBool("ScrapeVideos", scrape_video->getState()); });
		}
	}

	scraper_list->setSelectedChangedCallback([this, s, scraper, scraper_list](std::string value)
	{
		if (value != scraper)
		{
			Settings::getInstance()->setString("Scraper", value);
			delete s;
			openScraperSettings();
		}
	});

	mWindow->pushGui(s);
}

void GuiMenu::openConfigInput()
{
	Window* window = mWindow;
	window->pushGui(new GuiMsgBox(window, _("ARE YOU SURE YOU WANT TO CONFIGURE THE INPUT?"),
		_("YES"), [window] { window->pushGui(new GuiDetectDevice(window, false, nullptr)); },
		_("NO"), nullptr)
	);
}

void GuiMenu::addVersionInfo()
{
	std::string  buildDate = (Settings::getInstance()->getBool("Debug") ? std::string( "   (" + Utils::String::toUpper(PROGRAM_BUILT_STRING) + ")") : (""));

	auto theme = ThemeData::getMenuTheme();

	mVersion.setFont(theme->Footer.font);
	mVersion.setColor(theme->Footer.color);

	mVersion.setLineSpacing(0);

	if (!ApiSystem::getInstance()->getVersion().empty())
	{
		mVersion.setText(ApiSystem::getInstance()->getApplicationName());
	}

	mVersion.setHorizontalAlignment(ALIGN_CENTER);
	mVersion.setVerticalAlignment(ALIGN_CENTER);
	addChild(&mVersion);
}

void GuiMenu::openScreensaverOptions()
{
	mWindow->pushGui(new GuiGeneralScreensaverOptions(mWindow));
}
void GuiMenu::openCollectionSystemSettings()
{
	if (ThreadedScraper::isRunning() || ThreadedHasher::isRunning())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow, _("THIS FUNCTION IS DISABLED WHILE THE SCRAPER IS RUNNING")));
		return;
	}

	mWindow->pushGui(new GuiCollectionSystemsOptions(mWindow));
}

void GuiMenu::onSizeChanged()
{
	float h = mMenu.getButtonGridHeight();

	mVersion.setSize(mSize.x(), h);
	mVersion.setPosition(0, mSize.y() - h); //  mVersion.getSize().y()
}

void GuiMenu::addEntry(std::string name, bool add_arrow, const std::function<void()>& func, const std::string iconName)
{
	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	// populate the list
	ComponentListRow row;

	if (!iconName.empty())
	{
		std::string iconPath = theme->getMenuIcon(iconName);
		if (!iconPath.empty())
		{
			// icon
			auto icon = std::make_shared<ImageComponent>(mWindow, true);
			icon->setImage(iconPath);
			icon->setColorShift(theme->Text.color);
			icon->setResize(0, theme->Text.font->getLetterHeight() * 1.25f);
			row.addElement(icon, false);

			// spacer between icon and text
			auto spacer = std::make_shared<GuiComponent>(mWindow);
			spacer->setSize(10, 0);
			row.addElement(spacer, false);
		}
	}

	auto text = std::make_shared<TextComponent>(mWindow, name, font, color);
	row.addElement(text, true);

	if (EsLocale::isRTL())
		text->setHorizontalAlignment(Alignment::ALIGN_RIGHT);

	if (add_arrow)
	{
		std::shared_ptr<ImageComponent> bracket = makeArrow(mWindow);

		if (EsLocale::isRTL())
			bracket->setFlipX(true);

		row.addElement(bracket, false);
	}

	row.makeAcceptInputHandler(func);
	mMenu.addRow(row);
}

bool GuiMenu::input(InputConfig* config, Input input)
{
	if(GuiComponent::input(config, input))
		return true;

	if((config->isMappedTo(BUTTON_BACK, input) || config->isMappedTo("start", input)) && input.value != 0)
	{
		delete this;
		return true;
	}

	return false;
}

std::vector<HelpPrompt> GuiMenu::getHelpPrompts()
{
	std::vector<HelpPrompt> prompts;
	prompts.push_back(HelpPrompt("up/down", _("CHOOSE"))); 
	prompts.push_back(HelpPrompt(BUTTON_OK, _("SELECT"))); 
	prompts.push_back(HelpPrompt("start", _("CLOSE"))); 
	return prompts;
}

class ExitKidModeMsgBox : public GuiSettings
{
	public: ExitKidModeMsgBox(Window* window, const std::string& title, const std::string& text) : GuiSettings(window, title) { addEntry(text); }

	bool input(InputConfig* config, Input input) override
	{
		if (UIModeController::getInstance()->listen(config, input))
		{
			mWindow->pushGui(new GuiMsgBox(mWindow, _("THE UI MODE IS NOW UNLOCKED"),
				_("OK"), [this]
				{
					Window* window = mWindow;
					while (window->peekGui() && window->peekGui() != ViewController::get())
						delete window->peekGui();
				}));


			return true;
		}

		return GuiComponent::input(config, input);
	}
};

void GuiMenu::exitKidMode()
{
	mWindow->pushGui(new ExitKidModeMsgBox(mWindow, _("UNLOCK UI MODE"), _("ENTER THE CODE TO UNLOCK THE CURRENT UI MODE")));
}

void GuiMenu::openSystemInformations_batocera()
{
	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	Window *window = mWindow;
	bool isFullUI = UIModeController::getInstance()->isUIModeFull();
	GuiSettings *informationsGui = new GuiSettings(window, _("INFORMATION").c_str());

	// various informations
	std::vector<std::string> infos = ApiSystem::getInstance()->getSystemInformations();
	for (auto it = infos.begin(); it != infos.end(); it++) {
		std::vector<std::string> tokens = Utils::String::split(*it, ':');

		if (tokens.size() >= 2) {
			// concatenat the ending words
			std::string vname = "";
			for (unsigned int i = 1; i < tokens.size(); i++) {
				if (i > 1) vname += " ";
				vname += tokens.at(i);
			}

			auto space = std::make_shared<TextComponent>(window,
				vname,
				font,
				color);
			informationsGui->addWithLabel(_(tokens.at(0).c_str()), space);
		}
	}

	informationsGui->addGroup(_("VIDEO DRIVER"));
	for (auto info : Renderer::getDriverInformation())
	{
		auto glversion = std::make_shared<TextComponent>(window, info.second, font, color);
		informationsGui->addWithLabel(_(info.first.c_str()), glversion);
	}

	window->pushGui(informationsGui);
}

void GuiMenu::openChangeLog()
{
	runSystemCommand("system-upgrade changelog", "", nullptr);
	runSystemCommand("show_changelog", "", nullptr);
}

void GuiMenu::openDecorationConfiguration(Window *mWindow, std::string configName, std::vector<DecorationSetInfo> sets)
{
	//Using a shared pointer to ensure the memory doesn't cause issues in the other class
	std::map<std::string, std::string> decorationSetNameToPath;
	for (auto set : sets)
	{
		decorationSetNameToPath.insert(std::make_pair(set.name, set.path));
	}

	auto decorationOptions = new GuiDecorationOptions(mWindow, configName, decorationSetNameToPath);
	mWindow->pushGui(decorationOptions);
}

bool GuiMenu::checkNetwork()
{
	if (ApiSystem::getInstance()->getIpAdress() == "NOT CONNECTED")
	{
		mWindow->pushGui(new GuiMsgBox(mWindow, _("YOU ARE NOT CONNECTED TO A NETWORK"), _("OK"), nullptr));
		return false;
	}

	return true;
}

void GuiMenu::openSystemSettings_batocera()
{
	Window *window = mWindow;

	auto s = new GuiSettings(mWindow, _("SYSTEM SETTINGS").c_str());
	bool isFullUI = UIModeController::getInstance()->isUIModeFull();

	s->addGroup(_("SYSTEM"));

	// System informations
	s->addEntry(_("INFORMATION"), true, [this] { openSystemInformations_batocera(); });

	auto emuelec_timezones = std::make_shared<OptionListComponent<std::string> >(mWindow, _("TIMEZONE"), false);
	std::string currentTimezone = SystemConf::getInstance()->get("system.timezone");
	if (currentTimezone.empty())
		currentTimezone = std::string(getShOutput(R"(/usr/bin/timeinfo current_timezone)"));
	std::string a;
	for(std::stringstream ss(getShOutput(R"(/usr/bin/timeinfo timezones)")); getline(ss, a, ','); ) {
		emuelec_timezones->add(a, a, currentTimezone == a); // emuelec
	}
	s->addWithLabel(_("TIMEZONE"), emuelec_timezones);
	s->addSaveFunc([emuelec_timezones] {
		if (emuelec_timezones->changed()) {
			std::string selectedTimezone = emuelec_timezones->getSelected();
			runSystemCommand("ln -sf /usr/share/zoneinfo/" + selectedTimezone + " $(readlink /etc/localtime)", "", nullptr);
		}
		SystemConf::getInstance()->set("system.timezone", emuelec_timezones->getSelected());
	});

	// Clock time format (14:42 or 2:42 pm)
	auto tmFormat = std::make_shared<SwitchComponent>(mWindow);
	tmFormat->setState(Settings::getInstance()->getBool("ClockMode12"));
	s->addWithLabel(_("SHOW CLOCK IN 12-HOUR FORMAT"), tmFormat);
	tmFormat->setOnChangedCallback([tmFormat] { Settings::getInstance()->setBool("ClockMode12", tmFormat->getState()); });

        s->addGroup(_("AUTHENTICATION"));
        bool rotateRootPassEnabled = SystemConf::getInstance()->getBool("rotate.root.password");
        auto rotate_root_pass = std::make_shared<SwitchComponent>(mWindow);
        rotate_root_pass->setState(rotateRootPassEnabled);
        s->addWithLabel(_("ROTATE ROOT PASSWORD"), rotate_root_pass);

        rotate_root_pass->setOnChangedCallback([this, rotate_root_pass]
        {
                SystemConf::getInstance()->setBool("rotate.root.password", rotate_root_pass->getState());
                SystemConf::getInstance()->saveSystemConf();
        });

        auto root_password = std::make_shared<TextComponent>(mWindow, SystemConf::getInstance()->get("root.password"), ThemeData::getMenuTheme()->Text.font, ThemeData::getMenuTheme()->Text.color);
        s->addInputTextRow(_("ROOT PASSWORD"), "root.password", false);

        s->addSaveFunc([this, root_password] {
                SystemConf::getInstance()->saveSystemConf();
                const std::string rootpass = SystemConf::getInstance()->get("root.password");
                runSystemCommand("setrootpass " + rootpass, "", nullptr);
        });

	s->addGroup(_("DISPLAY"));


	if (BrightnessControl::getInstance()->isAvailable())
	{
		// brightness
		int brightness = BrightnessControl::getInstance()->getBrightness();
		float brightness_count = BrightnessControl::getInstance()->getNumBrightness() - 1;
		std::string brightness_count_str = "/" + std::to_string((int)brightness_count) + "  ";
		auto brightnessComponent = std::make_shared<SliderComponent>(mWindow, 0.f, brightness_count, 1.f, brightness_count_str);
		brightnessComponent->setValue(brightness);
		brightnessComponent->setOnValueChanged([](const float &newVal) {
			BrightnessControl::getInstance()->setBrightness((int)Math::round(newVal));
                        SystemConf::getInstance()->set("system.brightness", std::to_string((int)newVal));
		});

		s->addWithLabel(_("BRIGHTNESS"), brightnessComponent);

	}
	if (GetEnv("DEVICE_PWR_LED_CONTROL") == "true") {

		s->addGroup(_("DEVICE LEDS"));
		// Disable Power LED
		auto pwr_led_disabled = std::make_shared<SwitchComponent>(mWindow);
		bool pwrleddisabled = SystemConf::getInstance()->get("powerled.disabled") == "1";
		pwr_led_disabled->setState(SystemConf::getInstance()->getBool("powerled.disabled"));
		s->addWithLabel(_("DISABLE POWER LED"), pwr_led_disabled);
		pwr_led_disabled->setOnChangedCallback([pwr_led_disabled] {
			bool pwrleddisabled = pwr_led_disabled->getState();
				SystemConf::getInstance()->set("powerled.disabled", pwrleddisabled ? "1" : "0");
				SystemConf::getInstance()->saveSystemConf();
		});
	}
	
        if (GetEnv("DEVICE_LED_CONTROL") == "true"){
		s->addGroup(_("DEVICE LEDS"));
		// Provides LED management
		auto optionsColors = std::make_shared<OptionListComponent<std::string> >(mWindow, _("LED COLOR"), false);

		std::vector<std::string> availableColors = ApiSystem::getInstance()->getAvailableColors();
		std::string selectedColors = SystemConf::getInstance()->get("led.color");
		if (selectedColors.empty())
			selectedColors = "default";

		bool lfound = false;
		for (auto it = availableColors.begin(); it != availableColors.end(); it++)
		{
			optionsColors->add((*it), (*it), selectedColors == (*it));
			if (selectedColors == (*it))
			        lfound = true;
		}
		if (!lfound)
			optionsColors->add(selectedColors, selectedColors, true);

		s->addWithLabel(_("LED COLOR"), optionsColors);

		s->addSaveFunc([this, optionsColors, selectedColors]
		{
			if (optionsColors->changed()) {
				SystemConf::getInstance()->set("led.color", optionsColors->getSelected());
				runSystemCommand("/usr/bin/sh -lc \"/usr/bin/ledcontrol " + optionsColors->getSelected() + "\"" , "", nullptr);
				SystemConf::getInstance()->saveSystemConf();
			}
		});
	}
	if (GetEnv("DEVICE_LED_BRIGHTNESS") == "true"){
	        // Sets LED brightness
	        auto optionsLEDBrightness = std::make_shared<OptionListComponent<std::string> >(mWindow, _("LED BRIGHTNESS"), false);
	        std::string selectedLEDBrightness = SystemConf::getInstance()->get("led.brightness");
	        if (selectedLEDBrightness.empty())
	                selectedLEDBrightness = "max";

	        optionsLEDBrightness->add(_("MAX"),"max", selectedLEDBrightness == "max");
	        optionsLEDBrightness->add(_("MID"),"mid", selectedLEDBrightness == "mid");
	        optionsLEDBrightness->add(_("MIN"),"min", selectedLEDBrightness == "min");
	        s->addWithLabel(_("LED BRIGHTNESS"), optionsLEDBrightness);
	
	        s->addSaveFunc([this, optionsLEDBrightness, selectedLEDBrightness]
	        {
	                if (optionsLEDBrightness->changed()) {
	                        SystemConf::getInstance()->set("led.brightness", optionsLEDBrightness->getSelected());
	                        SystemConf::getInstance()->saveSystemConf();
	                        runSystemCommand("/usr/bin/ledcontrol brightness " + optionsLEDBrightness->getSelected(), "", nullptr);
	                }
	        });
	}

        if (GetEnv("DEVICE_DTB_SWITCH") == "true"){
        s->addGroup(_("HARDWARE /DEVICE"));
        // Switch device dtb between the R33S & R36S
        auto device_switch = std::make_shared<SwitchComponent>(mWindow);
        bool deviceswitchEnabled = SystemConf::getInstance()->get("system.device-dtb-r36s") == "1";
        device_switch->setState(deviceswitchEnabled);
        s->addWithLabel(_("DEVICE IS R36S / R35S?"), device_switch);
        s->addSaveFunc([this,device_switch] {

                if (device_switch->changed()) {
                std::string msg = _("The system will restart\n and user settings will be reset")+"\n";
                msg += _("Do you want to continue?");

                        mWindow->pushGui(new GuiMsgBox(mWindow,msg, _("YES"),
                                [this,device_switch] {

                                bool dswitchenabled = device_switch->getState();
                                SystemConf::getInstance()->set("system.device-dtb-r36s", dswitchenabled ? "1" : "0");
                                SystemConf::getInstance()->saveSystemConf();
                                if (device_switch->getState() == false) {
                                        runSystemCommand("/usr/bin/device-switch R33S", "", nullptr);
                                } else {
                                        runSystemCommand("/usr/bin/device-switch R36S", "", nullptr);
                                }
                        }, "NO",nullptr));
                }
        });
        }

	s->addGroup(_("HARDWARE / STORAGE"));
	if (GetEnv("DEVICE_MMC_EJECT") != "false") {

		// Provides a mechanism to disable use of the second device
		bool MountGamesEnabled = SystemConf::getInstance()->getBool("system.automount");
		auto mount_games = std::make_shared<SwitchComponent>(mWindow);
		mount_games->setState(MountGamesEnabled);
		s->addWithLabel(_("AUTODETECT GAMES CARD"), mount_games);
		mount_games->setOnChangedCallback([this, s, mount_games] {
			SystemConf::getInstance()->setBool("system.automount", mount_games->getState());
			SystemConf::getInstance()->saveSystemConf();
			runSystemCommand("/usr/bin/systemctl restart jelos-automount", "", nullptr);
		});

		if (Utils::FileSystem::exists("/storage/.ms_supported") && MountGamesEnabled) 
		{
	                auto overlayState = std::make_shared<SwitchComponent>(mWindow);
	                bool overlayStateEnabled = SystemConf::getInstance()->getBool("system.merged.storage");
	                overlayState->setState(overlayStateEnabled);
	                s->addWithLabel(_("ENABLE MERGED STORAGE"), overlayState);
	                overlayState->setOnChangedCallback([this, s, overlayState] {
	                        bool overlayStateEnabled = overlayState->getState();
				SystemConf::getInstance()->setBool("system.merged.storage", overlayState->getState());
				SystemConf::getInstance()->saveSystemConf();
	                        runSystemCommand("/usr/bin/systemctl restart jelos-automount", "", nullptr);
			});

			auto optionsMSDevice = std::make_shared<OptionListComponent<std::string> >(mWindow, _("MERGED STORAGE PRIMARY CARD"), false);
			std::string selectedMSDevice = SystemConf::getInstance()->get("system.merged.device");
			if (selectedMSDevice.empty())
				selectedMSDevice = "default";

			optionsMSDevice->add(_("DEFAULT"),"default", selectedMSDevice == "default");
			optionsMSDevice->add(_("EXTERNAL"),"external", selectedMSDevice == "external");
			optionsMSDevice->add(_("INTERNAL"),"internal", selectedMSDevice == "internal");
			s->addWithLabel(_("MERGED STORAGE PRIMARY CARD"), optionsMSDevice);

			s->addSaveFunc([this, optionsMSDevice, selectedMSDevice]
			{

				if (optionsMSDevice->changed()) {
					mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: CHANGING THE PRIMARY CARD CAN CAUSE ACCESS TO GAMES TO BE LOST, REQUIRING MANUAL INTERVENTION TO CORRECT. CONTINUE?"), _("YES"), [this, optionsMSDevice, selectedMSDevice]
					{
						SystemConf::getInstance()->set("system.merged.device", optionsMSDevice->getSelected());
						SystemConf::getInstance()->saveSystemConf();
						runSystemCommand("/usr/bin/systemctl restart jelos-automount " + optionsMSDevice->getSelected(), "", nullptr);
					}, _("NO"), nullptr));
				}
			});
		}

		s->addEntry(_("EJECT MICROSD CARD"), false, [window] {
			if (Utils::FileSystem::exists("/storage/.ms_supported"))
			{
				runSystemCommand("/usr/bin/umount -f /storage/roms; /usr/bin/umount -f /storage/games-external", "", nullptr);
			} else {
				runSystemCommand("/usr/bin/umount -f /storage/roms", "", nullptr);
			}
			window->pushGui(new GuiMsgBox(window, _("You may now remove the card.")));
		});
	}

	s->addGroup(_("HARDWARE / PERFORMANCE"));

#if defined(AMD64)
        // Allow offlining all but n threads
	auto optionsThreads = std::make_shared<OptionListComponent<std::string> >(mWindow, _("AVAILABLE THREADS"), false);

	std::vector<std::string> availableThreads = ApiSystem::getInstance()->getAvailableThreads();
	std::string selectedThreads = SystemConf::getInstance()->get("system.threads");
	if (selectedThreads.empty())
		selectedThreads = "default";

	bool wfound = false;
	for (auto it = availableThreads.begin(); it != availableThreads.end(); it++)
	{
		optionsThreads->add((*it), (*it), selectedThreads == (*it));
		if (selectedThreads == (*it))
			wfound = true;
	}
	if (!wfound)
		optionsThreads->add(selectedThreads, selectedThreads, true);

	s->addWithLabel(_("AVAILABLE THREADS"), optionsThreads);

	s->addSaveFunc([this, optionsThreads, selectedThreads]
	{
		if (optionsThreads->changed()) {
			SystemConf::getInstance()->set("system.threads", optionsThreads->getSelected());
			runSystemCommand("/usr/bin/sh -lc \". /etc/profile.d/099-freqfunctions; onlinethreads " + optionsThreads->getSelected() + " 0" + "\"" , "", nullptr);
			SystemConf::getInstance()->saveSystemConf();
		}
	});

#endif
	if (GetEnv("DEVICE_HAS_FAN") == "true") {
	  // Provides cooling profile switching
	  auto optionsFanProfile = std::make_shared<OptionListComponent<std::string> >(mWindow, _("COOLING PROFILE"), false);
	  std::string selectedFanProfile = SystemConf::getInstance()->get("cooling.profile");
	  if (selectedFanProfile.empty())
		selectedFanProfile = "auto";

	  optionsFanProfile->add(_("AUTO"),"auto", selectedFanProfile == "auto");
	  optionsFanProfile->add(_("QUIET"),"quiet", selectedFanProfile == "quiet");
	  optionsFanProfile->add(_("MODERATE"),"moderate", selectedFanProfile == "moderate");
	  optionsFanProfile->add(_("AGGRESSIVE"),"aggressive", selectedFanProfile == "aggressive");
	  optionsFanProfile->add(_("CUSTOM"),"custom", selectedFanProfile == "custom");

	  s->addWithLabel(_("COOLING PROFILE"), optionsFanProfile);

	  s->addSaveFunc([this, optionsFanProfile, selectedFanProfile]
	  {
	    if (optionsFanProfile->changed()) {
	      SystemConf::getInstance()->set("cooling.profile", optionsFanProfile->getSelected());
	      SystemConf::getInstance()->saveSystemConf();
	      runSystemCommand("systemctl restart fancontrol", "", nullptr);
	    }
	  });
	}

// Prep for additional device support.
#if defined(AMD64)
        std::vector<std::string> cpuVendor = ApiSystem::getInstance()->getCPUVendor();
	std::vector<std::string> tdpRange = ApiSystem::getInstance()->getTdpRange();
	auto it = cpuVendor.begin();

        if (*it == "AuthenticAMD") {
		// Provides overclock profile switching
		auto deviceTDP = getenv("DEVICE_BASE_TDP");
		auto optionsOCProfile = std::make_shared<OptionListComponent<std::string> >(mWindow, _("CPU TDP Max"), false);
		std::string selectedOCProfile = SystemConf::getInstance()->get("system.overclock");
		if (selectedOCProfile.empty() || selectedOCProfile == deviceTDP)
			selectedOCProfile = "default";

		bool xfound = false;
		for (auto it = tdpRange.begin(); it != tdpRange.end(); it++)
		{
			optionsOCProfile->add((*it), (*it), selectedOCProfile == (*it));
			if (selectedOCProfile == (*it))
				xfound = true;
		}

		if (!xfound)
			optionsOCProfile->add(selectedOCProfile, selectedOCProfile, true);

	 	s->addWithLabel(_("CPU TDP Max"), optionsOCProfile);

		s->addSaveFunc([this, optionsOCProfile, selectedOCProfile]
		{
			if (optionsOCProfile->changed()) {
				mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: OVERCLOCKING YOUR DEVICE MAY RESULT IN STABILITY PROBLEMS OR CAUSE HARDWARE DAMAGE!\n\nUSING THE QUIET COOLING PROFILE WHILE USING CERTAIN OVERCLOCKS MAY CAUSE PANIC REBOOTS!\n\nJELOS IS NOT RESPONSIBLE FOR ANY DAMAGE THAT MAY OCCUR USING THESE SETTINGS!\n\nCLICK YES THAT YOU AGREE, OR NO TO CANCEL."), _("YES"),
	                                [this,optionsOCProfile] {
						SystemConf::getInstance()->set("system.overclock", optionsOCProfile->getSelected());
						SystemConf::getInstance()->saveSystemConf();
						runSystemCommand("/usr/bin/overclock " + optionsOCProfile->getSelected(), "", nullptr);
	                                }, _("NO"), nullptr));
			}
		});
	}

        if (Utils::FileSystem::exists("/sys/devices/system/cpu/cpufreq/policy0/energy_performance_preference")) {
                // Provides EPP Profile switching
                auto optionsEPP = std::make_shared<OptionListComponent<std::string> >(mWindow, _("Energy Performance Preference"), false);
                std::string selectedEPP = SystemConf::getInstance()->get("system.power.epp");
                if (selectedEPP.empty())
                        selectedEPP = "default";

                optionsEPP->add(_("DEFAULT"), "default", selectedEPP == "default");

                optionsEPP->add(_("Performance"),"performance", selectedEPP == "performance");
                optionsEPP->add(_("Balance Performance"),"balance_performance", selectedEPP == "balance_performance");
                optionsEPP->add(_("Balance Power Saving"),"balance_power", selectedEPP == "balance_power");
                optionsEPP->add(_("Power Saving"),"power", selectedEPP == "power");

                s->addWithLabel(_("Energy Performance Preference"), optionsEPP);

                s->addSaveFunc([this, optionsEPP, selectedEPP]
                {
                        if (optionsEPP->changed()) {
                                SystemConf::getInstance()->set("system.power.epp", optionsEPP->getSelected());
                                SystemConf::getInstance()->saveSystemConf();
                                runSystemCommand("/usr/bin/set_epp " + optionsEPP->getSelected(), "", nullptr);
                        }
                });
        }
#endif
        // Default Scaling governor
        auto optionsGovernors = std::make_shared<OptionListComponent<std::string> >(mWindow, _("DEFAULT SCALING GOVERNOR"), false);

        std::vector<std::string> availableGovernors = ApiSystem::getInstance()->getAvailableGovernors();
        std::string selectedGovernors = SystemConf::getInstance()->get("system.cpugovernor");
        if (selectedGovernors.empty())
                selectedGovernors = "default";

        bool cfound = false;
        for (auto it = availableGovernors.begin(); it != availableGovernors.end(); it++)
        {
		optionsGovernors->add((*it), (*it), selectedGovernors == (*it));
		if (selectedGovernors == (*it))
			cfound = true;
        }
        if (!cfound)
                optionsGovernors->add(selectedGovernors, selectedGovernors, true);

        s->addWithLabel(_("DEFAULT SCALING GOVERNOR"), optionsGovernors);


        s->addSaveFunc([selectedGovernors, optionsGovernors]
        {
          if (optionsGovernors->changed()) {
            SystemConf::getInstance()->set("system.cpugovernor", optionsGovernors->getSelected());
            SystemConf::getInstance()->saveSystemConf();
          }
          runSystemCommand("/usr/bin/sh -lc \". /etc/profile.d/099-freqfunctions; "+ optionsGovernors->getSelected() + "\"", "", nullptr);
        });

	// GPU performance mode with enhanced power savings
	auto gpuPerformance = std::make_shared<OptionListComponent<std::string> >(mWindow, _("GPU PERFORMANCE PROFILE"), false);
	std::string gpu_performance = SystemConf::getInstance()->get("system.gpuperf");
	if (gpu_performance.empty())
		gpu_performance = "auto";

	gpuPerformance->add(_("Balanced"), "auto", gpu_performance == "auto");
	gpuPerformance->add(_("Battery Focus"), "low", gpu_performance == "low");
#if defined(AMD64)
	gpuPerformance->add(_("Performance Focus"), "profile_standard", gpu_performance == "profile_standard");
#endif
	gpuPerformance->add(_("Best Performance"), "profile_peak", gpu_performance == "profile_peak");

	s->addWithLabel(_("GPU PERFORMANCE PROFILE"), gpuPerformance);
	s->addSaveFunc([this, gpuPerformance, gpu_performance]
	{
		if (gpuPerformance->changed()) {
			SystemConf::getInstance()->set("system.gpuperf", gpuPerformance->getSelected());
			SystemConf::getInstance()->saveSystemConf();
			runSystemCommand("/usr/bin/sh -lc \". /etc/profile.d/030-powerfunctions; gpu_performance_level "+ gpuPerformance->getSelected() + "\"", "", nullptr);
		}
	});

        if (GetEnv("DEVICE_TURBO_MODE") == "true"){
                // Add option to enable turbo mode overclocking
                auto turbo_mode = std::make_shared<SwitchComponent>(mWindow);
                bool internalmoduleEnabled = SystemConf::getInstance()->get("enable.turbo-mode") == "1";
                turbo_mode->setState(internalmoduleEnabled);
                s->addWithLabel(_("ENABLE CPU OVERCLOCK"), turbo_mode);
                turbo_mode->setOnChangedCallback([turbo_mode] {
                if (turbo_mode->getState() == false) {
                        runSystemCommand("/usr/bin/turbomode disable", "", nullptr);
                } else {
                        runSystemCommand("/usr/bin/turbomode enable", "", nullptr);
                }
                bool turbomode = turbo_mode->getState();
                        SystemConf::getInstance()->set("enable.turbo-mode", turbomode ? "1" : "0");
                        SystemConf::getInstance()->saveSystemConf();
                });
        }

        const std::string gpuDriverScript = "/usr/bin/gpudriver";
        if (Utils::FileSystem::exists(gpuDriverScript)) {
                auto optionsGpuDriver = std::make_shared<OptionListComponent<std::string> >(mWindow, _("GPU DRIVER"), false);
                std::string selectedGpuDriver = SystemConf::getInstance()->get("gpu.driver");
                if (selectedGpuDriver.empty()) {
                        selectedGpuDriver = std::string(getShOutput(R"(/usr/bin/gpudriver)"));
                }

                std::string a;
                for(std::stringstream ss(getShOutput(R"(/usr/bin/gpudriver --options)")); getline(ss, a, ' '); ) {
                        optionsGpuDriver->add(a, a, a == selectedGpuDriver);
                }

                s->addWithLabel(_("GPU DRIVER"), optionsGpuDriver);

                s->addSaveFunc([this, gpuDriverScript, optionsGpuDriver] {
                                SystemConf::getInstance()->set("gpu.driver", optionsGpuDriver->getSelected());
                                runSystemCommand(gpuDriverScript + " " + optionsGpuDriver->getSelected(), "", nullptr);
                });
        }


	s->addGroup(_("HARDWARE / POWER SAVING"));
        // Automatically enable or disable enhanced power saving mode
        auto enh_powersave = std::make_shared<SwitchComponent>(mWindow);
        bool enhpowersaveEnabled = SystemConf::getInstance()->get("system.powersave") == "1";
        enh_powersave->setState(enhpowersaveEnabled);
        s->addWithLabel(_("ENHANCED POWER SAVING"), enh_powersave);
        s->addSaveFunc([enh_powersave] {
                bool enhpowersaveEnabled = enh_powersave->getState();
                SystemConf::getInstance()->set("system.powersave", enhpowersaveEnabled ? "1" : "0");
                SystemConf::getInstance()->saveSystemConf();
        });

        if (SystemConf::getInstance()->getBool("system.powersave", true)) {
        	// Options for enhanced power savings mode
        	auto enh_cpupowersave = std::make_shared<SwitchComponent>(mWindow);
        	bool enhcpupowersaveEnabled = SystemConf::getInstance()->get("system.power.cpu") == "1";
        	enh_cpupowersave->setState(enhcpupowersaveEnabled);
        	s->addWithLabel(_("CPU POWER SAVING"), enh_cpupowersave);
        	enh_cpupowersave->setOnChangedCallback([enh_cpupowersave] {
        	        bool enhcpupowersaveEnabled = enh_cpupowersave->getState();
               	 SystemConf::getInstance()->set("system.power.cpu", enhcpupowersaveEnabled ? "1" : "0");
               	 SystemConf::getInstance()->saveSystemConf();
        	});

	        auto enh_audiopowersave = std::make_shared<SwitchComponent>(mWindow);
	        bool enhaudiopowersaveEnabled = SystemConf::getInstance()->get("system.power.audio") == "1";
	        enh_audiopowersave->setState(enhaudiopowersaveEnabled);
	        s->addWithLabel(_("AUDIO POWER SAVING"), enh_audiopowersave);
	        enh_audiopowersave->setOnChangedCallback([enh_audiopowersave] {
	                bool enhaudiopowersaveEnabled = enh_audiopowersave->getState();
	                SystemConf::getInstance()->set("system.power.audio", enhaudiopowersaveEnabled ? "1" : "0");
	                SystemConf::getInstance()->saveSystemConf();
	        });

		// Automatically enable or disable WIFI power saving mode
		auto wifi_powersave = std::make_shared<SwitchComponent>(mWindow);
		bool wifipowersaveEnabled = SystemConf::getInstance()->get("system.power.wifi") == "1";
		wifi_powersave->setState(wifipowersaveEnabled);
		s->addWithLabel(_("WIFI POWER SAVING"), wifi_powersave);
		wifi_powersave->setOnChangedCallback([wifi_powersave] {
			bool wifipowersaveEnabled = wifi_powersave->getState();
			SystemConf::getInstance()->set("system.power.wifi", wifipowersaveEnabled ? "1" : "0");
			SystemConf::getInstance()->saveSystemConf();
			runSystemCommand("/usr/bin/wifictl setpowersave", "", nullptr);
		});

		auto warn = std::make_shared<TextComponent>(mWindow, "Below options can affect stability.", ThemeData::getMenuTheme()->Text.font, ThemeData::getMenuTheme()->Text.color);
		s->addWithLabel(_("WARNING"), warn);
	        auto enh_pciepowersave = std::make_shared<SwitchComponent>(mWindow);
	        bool enhpciepowersaveEnabled = SystemConf::getInstance()->get("system.power.pcie") == "1";
	        enh_pciepowersave->setState(enhpciepowersaveEnabled);
	        s->addWithLabel(_("PCIE ACTIVE STATE POWER MANAGEMENT"), enh_pciepowersave);
	        enh_pciepowersave->setOnChangedCallback([enh_pciepowersave] {
	                bool enhpciepowersaveEnabled = enh_pciepowersave->getState();
	                SystemConf::getInstance()->set("system.power.pcie", enhpciepowersaveEnabled ? "1" : "0");
	                SystemConf::getInstance()->saveSystemConf();
	        });

	        auto wakeevents = std::make_shared<SwitchComponent>(mWindow);
	        bool wakeeventsEnabled = SystemConf::getInstance()->get("system.power.wakeevents") == "1";
	        wakeevents->setState(wakeeventsEnabled);
	        s->addWithLabel(_("ENABLE WAKE EVENTS"), wakeevents);
	        wakeevents->setOnChangedCallback([wakeevents] {
	                bool wakeeventsEnabled = wakeevents->getState();
	                SystemConf::getInstance()->set("system.power.wakeevents", wakeeventsEnabled ? "1" : "0");
	                SystemConf::getInstance()->saveSystemConf();
	        });

	        auto rtpm = std::make_shared<SwitchComponent>(mWindow);
	        bool rtpmEnabled = SystemConf::getInstance()->get("system.power.rtpm") == "1";
	        rtpm->setState(rtpmEnabled);
	        s->addWithLabel(_("RUNTIME POWER MANAGEMENT"), rtpm);
	        rtpm->setOnChangedCallback([rtpm] {
	                bool rtpmEnabled = rtpm->getState();
	                SystemConf::getInstance()->set("system.power.rtpm", rtpmEnabled ? "1" : "0");
	                SystemConf::getInstance()->saveSystemConf();
	        });
	}

// Do not show on S922X devices yet.
#if defined(AMD64) || defined(RK3326) || defined(RK3566) || defined(RK3566_X55) || defined(RK3588) || defined(RK3588_ACE) || defined(RK3399)
        // Allow user control over how the device sleeps
        s->addGroup(_("HARDWARE / SUSPEND"));
        auto optionsSleep = std::make_shared<OptionListComponent<std::string> >(mWindow, _("DEVICE SUSPEND MODE"), false);

        std::vector<std::string> availableSleepModes = ApiSystem::getInstance()->getSleepModes();
        std::string selectedSleep = SystemConf::getInstance()->get("system.suspendmode");
        if (selectedSleep.empty())
                selectedSleep = "default";

        bool xfound = false;
        for (auto it = availableSleepModes.begin(); it != availableSleepModes.end(); it++)
        {
		optionsSleep->add((*it), (*it), selectedSleep == (*it));
		if (selectedSleep == (*it))
			xfound = true;
        }

        if (!xfound)
                optionsSleep->add(selectedSleep, selectedSleep, true);

        s->addWithLabel(_("DEVICE SUSPEND MODE"), optionsSleep);

        s->addSaveFunc([this, optionsSleep, selectedSleep]
        {
                if (optionsSleep->changed()) {
                        SystemConf::getInstance()->set("system.suspendmode", optionsSleep->getSelected());
                        runSystemCommand("/usr/bin/suspendmode " + optionsSleep->getSelected(), "", nullptr);
                        SystemConf::getInstance()->saveSystemConf();
                }
        });
#endif

#ifdef RK3399
	// Add option to disable RG552 wifi gpio
        auto internal_wifi = std::make_shared<SwitchComponent>(mWindow);
        bool internalmoduleEnabled = SystemConf::getInstance()->get("internal.wifi") == "1";
        internal_wifi->setState(internalmoduleEnabled);
        s->addWithLabel(_("ENABLE INTERNAL WIFI"), internal_wifi);
        internal_wifi->setOnChangedCallback([internal_wifi] {
        if (internal_wifi->getState() == false) {
                runSystemCommand("/usr/bin/internalwifi disable", "", nullptr);
        } else {
                runSystemCommand("/usr/bin/internalwifi enable", "", nullptr);
        }
        bool internalwifi = internal_wifi->getState();
                SystemConf::getInstance()->set("internal.wifi", internalwifi ? "1" : "0");
                SystemConf::getInstance()->saveSystemConf();
        });
#endif

#ifdef _ENABLEUPDATES
	s->addGroup(_("SYSTEM UPDATE"));

        // Enable updates
        auto updates_enabled = std::make_shared<SwitchComponent>(mWindow);
        updates_enabled->setState(SystemConf::getInstance()->getBool("updates.enabled"));
        updates_enabled->setOnChangedCallback([updates_enabled] {
                bool updatesenabled = updates_enabled->getState();
		SystemConf::getInstance()->set("updates.enabled", updatesenabled ? "1" : "0");
	});

        auto optionsUpdates = std::make_shared<OptionListComponent<std::string> >(mWindow, _("UPDATE BRANCH"), false);

        std::string selectedBranch = SystemConf::getInstance()->get("updates.branch");
        if (selectedBranch.empty())
                selectedBranch = "stable";

        optionsUpdates->add(_("RELEASE"), "stable", selectedBranch == "stable");
//        optionsUpdates->add(_("DEVELOPMENT"), "dev", selectedBranch == "dev");

        s->addWithLabel(_("UPDATE BRANCH"), optionsUpdates);

        s->addSaveFunc([this, optionsUpdates, selectedBranch]
        {
          if (optionsUpdates->changed()) {
            SystemConf::getInstance()->set("updates.branch", optionsUpdates->getSelected());
            SystemConf::getInstance()->saveSystemConf();
          }
        });

        bool ForceUpdateEnabled = SystemConf::getInstance()->getBool("updates.force");
        auto force_update = std::make_shared<SwitchComponent>(mWindow);
        force_update->setState(ForceUpdateEnabled);
        s->addWithLabel(_("FORCE NEXT UPDATE"), force_update);
        force_update->setOnChangedCallback([force_update] {
                bool forceupdate = force_update->getState();
                SystemConf::getInstance()->set("updates.force", forceupdate ? "1" : "0");
        });

        s->addWithLabel(_("CHECK FOR UPDATES"), updates_enabled);
        s->addSaveFunc([updates_enabled]
        {
                SystemConf::getInstance()->setBool("updates.enabled", updates_enabled->getState());
        });

        s->addEntry(_("CHANGE LOG"), true, [this] { openChangeLog(); });

                // Start update
        s->addEntry(GuiUpdate::state == GuiUpdateState::State::UPDATE_READY ? _("APPLY UPDATE") : _("START UPDATE"), true, [this]
        {
                if (GuiUpdate::state == GuiUpdateState::State::UPDATE_READY)
                        quitES(QuitMode::RESTART);
                else if (GuiUpdate::state == GuiUpdateState::State::UPDATER_RUNNING)
                        mWindow->pushGui(new GuiMsgBox(mWindow, _("UPDATER IS ALREADY RUNNING")));
                else
                {
                        if (!checkNetwork())
                                return;

                        mWindow->pushGui(new GuiUpdate(mWindow));
                }
        });
#endif

	if (isFullUI){
		s->addEntry(_("SYSTEM MANAGEMENT AND RESET"), true, [this] { openResetOptions(mWindow, "global"); });
	}

	auto pthis = this;
	s->onFinalize([s, pthis, window]
	{
		if (s->getVariable("reloadGuiMenu"))
		{
			delete pthis;
			window->pushGui(new GuiMenu(window, false));
		}
	});

	mWindow->pushGui(s);
}

void GuiMenu::openSavestatesConfiguration(Window* mWindow, std::string configName)
{
	GuiSettings* guiSaves = new GuiSettings(mWindow, _("SAVES CONFIGUREATION").c_str());

	// autosave/load
	auto autosave_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("AUTO SAVE/LOAD ON GAME LAUNCH"));
	autosave_enabled->addRange({ { _("OFF"), "default" },{ _("ON") , "1" },{ _("SHOW SAVE STATES") , "2" },{ _("SHOW SAVE STATES IF NOT EMPTY") , "3" } }, SystemConf::getInstance()->get("global.autosave"));
	guiSaves->addWithLabel(_("AUTO SAVE/LOAD ON GAME LAUNCH"), autosave_enabled);
	guiSaves->addSaveFunc([autosave_enabled] { SystemConf::getInstance()->set("global.autosave", autosave_enabled->getSelected()); });

	// Incremental savestates
	auto incrementalSaveStates = std::make_shared<SwitchComponent>(mWindow);
	incrementalSaveStates->setState(SystemConf::getInstance()->get("global.incrementalsavestates") == "1");
	guiSaves->addWithLabel(_("INCREMENTAL SAVESTATES"), incrementalSaveStates);
	guiSaves->addSaveFunc([incrementalSaveStates] { SystemConf::getInstance()->set("global.incrementalsavestates", incrementalSaveStates->getState() ? "1" : "0"); });
	
	// Maximum incremental savestates
	auto maxIncrementalSaves = std::make_shared<OptionListComponent<std::string>>(mWindow,"MAX INCREMENTAL SAVESTATES");
	maxIncrementalSaves->addRange({ { _("UNLIMITED"), "default" },{ _("2") , "2" },{ _("3") , "3" },{ _("5") , "5" },{ _("10") , "10" },{ _("20") , "20" },{ _("50") , "50" },{ _("100") , "100" },{ _("200") , "200" },{ _("500") , "500" },{ _("1000") , "1000" } }, SystemConf::getInstance()->get("global.maxincrementalsaves"));
	guiSaves->addWithLabel(_("MAX SAVESTATES"), maxIncrementalSaves);
	guiSaves->addSaveFunc([maxIncrementalSaves] { SystemConf::getInstance()->set("global.maxincrementalsaves", maxIncrementalSaves->getSelected()); });

	// Automated Cloud Backup
	auto cloudBackup = std::make_shared<SwitchComponent>(mWindow);
	cloudBackup->setState(SystemConf::getInstance()->get("cloud.backup") == "1");
	guiSaves->addWithLabel(_("BACKUP TO CLOUD ON GAME EXIT"), cloudBackup);
	guiSaves->addSaveFunc([cloudBackup] { SystemConf::getInstance()->set("cloud.backup", cloudBackup->getState() ? "1" : "0"); });

	mWindow->pushGui(guiSaves);

}
void GuiMenu::openLatencyReductionConfiguration(Window* mWindow, std::string configName)
{
	GuiSettings* guiLatency = new GuiSettings(mWindow, _("LATENCY REDUCTION").c_str());

	// run-ahead
	auto runahead_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("RUN-AHEAD FRAMES"));
	runahead_enabled->addRange({ { _("DEFAULT"), "" }, { _("NONE"), "0" }, { "1", "1" }, { "2", "2" }, { "3", "3" }, { "4", "4" }, { "5", "5" }, { "6", "6" } }, SystemConf::getInstance()->get(configName + ".runahead"));
	guiLatency->addWithLabel(_("USE RUN-AHEAD FRAMES"), runahead_enabled);
	guiLatency->addSaveFunc([configName, runahead_enabled] { SystemConf::getInstance()->set(configName + ".runahead", runahead_enabled->getSelected()); });

	// second instance
	auto secondinstance = std::make_shared<OptionListComponent<std::string>>(mWindow, _("RUN-AHEAD USE SECOND INSTANCE"));
	secondinstance->addRange({ { _("DEFAULT"), "" }, { _("ON"), "1" }, { _("OFF"), "0" } }, SystemConf::getInstance()->get(configName + ".secondinstance"));
	guiLatency->addWithLabel(_("RUN-AHEAD USE SECOND INSTANCE"), secondinstance);
	guiLatency->addSaveFunc([configName, secondinstance] { SystemConf::getInstance()->set(configName + ".secondinstance", secondinstance->getSelected()); });

	// audio-latency
	auto audio_latency = std::make_shared<OptionListComponent<std::string>>(mWindow, _("AUDIO LATENCY MILLISECONDS"));
	audio_latency->addRange({ { _("DEFAULT"), "" }, { "2", "2" }, { "4", "4" }, { "8", "8" }, { "16", "16" }, { "24", "24" }, { "32", "32" }, { "40", "40" }, { "48", "48" }, { "56", "56" }, { "64", "64" }, { "72", "72" }, { "80", "80" }, { "88", "88" }, { "96", "96" }, { "104", "104" }, { "112", "112" }, { "120", "120" }, { "128", "128" } }, SystemConf::getInstance()->get(configName + ".audiolatency"));
	guiLatency->addWithLabel(_("AUDIO LATENCY MILLISECONDS"), audio_latency);
	guiLatency->addSaveFunc([configName, audio_latency] { SystemConf::getInstance()->set(configName + ".audiolatency", audio_latency->getSelected()); });
	
	mWindow->pushGui(guiLatency);
}

void GuiMenu::openSystemOptionsConfiguration(Window* mWindow, std::string configName)
{
	GuiSettings* guiSystemOptions = new GuiSettings(mWindow, _("SYSTEM OPTIONS").c_str());
    bool cfound = false;

#if defined(S922X) || defined(RK3588)  || defined(RK3588_ACE) || defined(RK3399)
    // Core chooser
    auto cores_used = std::make_shared<OptionListComponent<std::string>>(mWindow, _("CORES USED"));
    cores_used->addRange({ { _("ALL"), "all" },{ _("BIG") , "big" },{ _("LITTLE") , "little" } }, SystemConf::getInstance()->get("global.cores"));
    guiSystemOptions->addWithLabel(_("CORES USED"), cores_used);
    guiSystemOptions->addSaveFunc([cores_used] { SystemConf::getInstance()->set("global.cores", cores_used->getSelected()); });
#endif	

    if (GetEnv("DEVICE_HAS_FAN") == "true") {
      // Provides cooling profile switching
      auto optionsFanProfile = std::make_shared<OptionListComponent<std::string> >(mWindow, _("COOLING PROFILE"), false);
      std::string selectedFanProfile = SystemConf::getInstance()->get(configName + ".cooling.profile");
      if (selectedFanProfile.empty())
            selectedFanProfile = "default";

      optionsFanProfile->add(_("DEFAULT"),"default", selectedFanProfile == "default");
      optionsFanProfile->add(_("AUTO"),"auto", selectedFanProfile == "auto");
      optionsFanProfile->add(_("QUIET"),"quiet", selectedFanProfile == "quiet");
      optionsFanProfile->add(_("MODERATE"),"moderate", selectedFanProfile == "moderate");
      optionsFanProfile->add(_("AGGRESSIVE"),"aggressive", selectedFanProfile == "aggressive");
      optionsFanProfile->add(_("CUSTOM"),"custom", selectedFanProfile == "custom");

      guiSystemOptions->addWithLabel(_("COOLING PROFILE"), optionsFanProfile);
      guiSystemOptions->addSaveFunc([optionsFanProfile, selectedFanProfile, configName]
      {
        if (optionsFanProfile->changed()) {
          SystemConf::getInstance()->set(configName + ".cooling.profile", optionsFanProfile->getSelected());
          SystemConf::getInstance()->saveSystemConf();
        }
      });
	}

    // Per game/core/emu CPU governor
    auto optionsGovernors = std::make_shared<OptionListComponent<std::string> >(mWindow, _("CPU SCALING GOVERNOR"), false);

    std::vector<std::string> availableGovernors = ApiSystem::getInstance()->getAvailableGovernors();
    std::string selectedGovernors = SystemConf::getInstance()->get(configName + ".cpugovernor");
    if (selectedGovernors.empty())
            selectedGovernors = "default";

    cfound = false;
    for (auto it = availableGovernors.begin(); it != availableGovernors.end(); it++)
    {
	optionsGovernors->add((*it), (*it), selectedGovernors == (*it));
	if (selectedGovernors == (*it))
		cfound = true;
    }
    if (!cfound)
            optionsGovernors->add(selectedGovernors, selectedGovernors, true);

    guiSystemOptions->addWithLabel(_("CPU SCALING GOVERNOR"), optionsGovernors);

    guiSystemOptions->addSaveFunc([configName, selectedGovernors, optionsGovernors]
    {
      if (optionsGovernors->changed()) {
        SystemConf::getInstance()->set(configName + ".cpugovernor", optionsGovernors->getSelected());
        SystemConf::getInstance()->saveSystemConf();
      }
    });

	// GPU performance mode with enhanced power savings
	auto gpuPerformance = std::make_shared<OptionListComponent<std::string> >(mWindow, _("GPU PERFORMANCE PROFILE"), false);
	std::string gpu_performance = SystemConf::getInstance()->get(configName + ".gpuperf");
	if (gpu_performance.empty())
		gpu_performance = "default";

	gpuPerformance->add(_("DEFAULT"), "default", gpu_performance == "default");
	gpuPerformance->add(_("Balanced"), "auto", gpu_performance == "auto");
	gpuPerformance->add(_("Battery Focus"), "low", gpu_performance == "low");
#if defined(AMD64)
	gpuPerformance->add(_("Performance Focus"), "profile_standard", gpu_performance == "profile_standard");
#endif
	gpuPerformance->add(_("Best Performance"), "profile_peak", gpu_performance == "profile_peak");

	guiSystemOptions->addWithLabel(_("GPU PERFORMANCE PROFILE"), gpuPerformance);

	guiSystemOptions->addSaveFunc([configName, gpuPerformance, gpu_performance]
	{
		if (gpuPerformance->changed()) {
			SystemConf::getInstance()->set(configName + ".gpuperf", gpuPerformance->getSelected());
			SystemConf::getInstance()->saveSystemConf();
		}
	});

#ifdef _WLROOTS
    // Per game/core/emu Display mode
    auto optionsDisplayModes = std::make_shared<OptionListComponent<std::string> >(mWindow, _("DISPLAY MODE"), false);

    std::vector<std::string> availableDisplayModes = ApiSystem::getInstance()->getAvailableDisplayModes();
    std::string selectedDisplayMode = SystemConf::getInstance()->get(configName + ".display_mode");

    for (auto it = availableDisplayModes.begin(); it != availableDisplayModes.end(); it++)
    {
        if (selectedDisplayMode.empty() && ((*it).find("preferred") != std::string::npos))
        {
            (*it) = (*it).substr((*it).find("px, ") + 4);
            (*it) = (*it).substr(0, (*it).find(" Hz") + 3);
            selectedDisplayMode = (*it);
            continue;
        }
        // Remove resolution at start, and any trailing markers, e.g. "preferred" or  "current"
        (*it) = (*it).substr((*it).find("px, ") + 4);
        (*it) = (*it).substr(0, (*it).find(" Hz") + 3);
    }

    cfound = false;
    for (auto it = availableDisplayModes.begin(); it != availableDisplayModes.end(); it++)
    {
	    optionsDisplayModes->add((*it), (*it), selectedDisplayMode == (*it));
	if (selectedDisplayMode == (*it))
		cfound = true;
    }
    if (!cfound)
            optionsDisplayModes->add(selectedDisplayMode, selectedDisplayMode, true);

    guiSystemOptions->addWithLabel(_("DISPLAY MODE"), optionsDisplayModes);

    guiSystemOptions->addSaveFunc([configName, selectedDisplayMode, optionsDisplayModes]
    {
      if (optionsDisplayModes->changed()) {
        SystemConf::getInstance()->set(configName + ".display_mode", optionsDisplayModes->getSelected());
        SystemConf::getInstance()->saveSystemConf();
      }
    });
#endif

#ifdef _PANFROST
	// Panfrost forcepack
	auto optionsForcepackEnabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("Panfrost Forcepack"));
    std::vector<std::string> availableForcepackEnabled = {{"Off"}, {"On"}};
    std::string selectedForcepackEnabled = SystemConf::getInstance()->get(configName + ".forcepack");
    if (selectedForcepackEnabled.empty())
        selectedForcepackEnabled = "Off";

    cfound = false;
    for (auto it = availableForcepackEnabled.begin(); it != availableForcepackEnabled.end(); it++)
    {
	    optionsForcepackEnabled->add((*it), (*it), selectedForcepackEnabled == (*it));
	if (selectedForcepackEnabled == (*it))
		cfound = true;
    }
    if (!cfound)
        optionsForcepackEnabled->add(selectedForcepackEnabled, selectedForcepackEnabled, true);

	guiSystemOptions->addWithLabel(_("Panfrost Forcepack"), optionsForcepackEnabled);
    guiSystemOptions->addSaveFunc([configName, selectedForcepackEnabled, optionsForcepackEnabled]
    {
      if (optionsForcepackEnabled->changed()) {
        SystemConf::getInstance()->set(configName + ".forcepack", optionsForcepackEnabled->getSelected());
        SystemConf::getInstance()->saveSystemConf();
      }
    });
#endif

	mWindow->pushGui(guiSystemOptions);
}

/*void GuiMenu::openCustomAspectRatioConfiguration(Window* mWindow, std::string configName)
{
	GuiSettings* guiViewport = new GuiSettings(mWindow, _("WIDTH/HEIGHT").c_str());
	std::vector<std::string> NativeResolution;
	NativeResolution = SystemData::getNativeResolution(configName);

	// X Position
	//auto positionx = std::make_shared<OptionListComponent<std::string>>(mWindow, _("POSITION X"));
	//positionx->addRange({ { _("DEFAULT"), "" }, {_("20"), "20"}}, SystemConf::getInstance()->get(configName + ".positionx"));
	//positionx->addRange({ { _("40"), "40" }, {_("60"), "60"}}, SystemConf::getInstance()->get(configName + ".positionx"));
	//guiViewport->addWithLabel(_("POSITION X"), positionx);
	//guiViewport->addSaveFunc([configName, positionx] { SystemConf::getInstance()->set(configName + ".positionx", positionx->getSelected()); });

	//guiViewport->addInputTextRow(_("POSITION X"), positionx.c_str(), false);
	//LOG(LogError) << positionx->getSelected();
	//guiViewport->addSaveFunc([configName, positionx] { SystemConf::getInstance()->set(configName + ".positionx", positionx); });


	// Y Position
//	guiViewport->addInputTextRow(_("POSITION Y"), positiony.c_str(), false);
	//LOG(LogError) << positiony;
	//guiViewport->addSaveFunc([configName, positiony] { SystemConf::getInstance()->set(configName + ".positiony", positiony); });

	// Width
	auto width = std::make_shared<OptionListComponent<std::string>>(mWindow, _("WIDTH"));
	width->addRange({ { _("DEFAULT"), "" }, { NativeResolution[0], NativeResolution[0]}, {std::to_string(std::stoi(NativeResolution[0])*2), std::to_string(std::stoi(NativeResolution[0])*2)},{std::to_string(std::stoi(NativeResolution[0])*3), std::to_string(std::stoi(NativeResolution[0])*3)},{std::to_string(std::stoi(NativeResolution[0])*4), std::to_string(std::stoi(NativeResolution[0])*4)},{std::to_string(std::stoi(NativeResolution[0])*5), std::to_string(std::stoi(NativeResolution[0])*5)}}, SystemConf::getInstance()->get(configName + ".width"));
	guiViewport->addWithLabel(_("WIDTH"), width);
	guiViewport->addSaveFunc([configName, width] { SystemConf::getInstance()->set(configName + ".width", width->getSelected()); });
	
	// Height
	auto height = std::make_shared<OptionListComponent<std::string>>(mWindow, _("HEIGHT"));
	height->addRange({ { _("DEFAULT"), "" }, { NativeResolution[1], NativeResolution[1]}, {std::to_string(std::stoi(NativeResolution[1])*2), std::to_string(std::stoi(NativeResolution[1])*2)},{std::to_string(std::stoi(NativeResolution[1])*3), std::to_string(std::stoi(NativeResolution[1])*3)},{std::to_string(std::stoi(NativeResolution[1])*4), std::to_string(std::stoi(NativeResolution[1])*4)},{std::to_string(std::stoi(NativeResolution[1])*5), std::to_string(std::stoi(NativeResolution[1])*5)}}, SystemConf::getInstance()->get(configName + ".height"));
	guiViewport->addWithLabel(_("HEIGHT"), height);
	guiViewport->addSaveFunc([configName, height] { SystemConf::getInstance()->set(configName + ".height", height->getSelected()); });
	
	mWindow->pushGui(guiViewport);
}
*/
void GuiMenu::openRetroachievementsSettings()
{
	Window* window = mWindow;
	GuiSettings* retroachievements = new GuiSettings(mWindow, _("RETROACHIEVEMENT SETTINGS").c_str());

	retroachievements->addGroup(_("SETTINGS"));

	bool retroachievementsEnabled = SystemConf::getInstance()->getBool("global.retroachievements");
	std::string username = SystemConf::getInstance()->get("global.retroachievements.username");
	std::string password = SystemConf::getInstance()->get("global.retroachievements.password");

	// retroachievements_enable
	auto retroachievements_enabled = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_enabled->setState(retroachievementsEnabled);
	retroachievements->addWithLabel(_("RETROACHIEVEMENTS"), retroachievements_enabled);

	// retroachievements_hardcore_mode
	auto retroachievements_hardcore_enabled = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_hardcore_enabled->setState(SystemConf::getInstance()->getBool("global.retroachievements.hardcore"));
	retroachievements->addWithLabel(_("HARDCORE MODE"), retroachievements_hardcore_enabled);
	retroachievements->addSaveFunc([retroachievements_hardcore_enabled] { SystemConf::getInstance()->setBool("global.retroachievements.hardcore", retroachievements_hardcore_enabled->getState()); });

	// retroachievements main menu option
	auto retroachievements_menuitem = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_menuitem->setState(Settings::getInstance()->getBool("RetroachievementsMenuitem"));
	retroachievements->addWithLabel(_("SHOW RETROACHIEVEMENTS ENTRY IN MAIN MENU"), retroachievements_menuitem);
	retroachievements->addSaveFunc([retroachievements_menuitem] { Settings::getInstance()->setBool("RetroachievementsMenuitem", retroachievements_menuitem->getState()); });


	//// retroachievements_leaderboards
	//auto retroachievements_leaderboards_enabled = std::make_shared<SwitchComponent>(mWindow);
	//retroachievements_leaderboards_enabled->setState(SystemConf::getInstance()->getBool("global.retroachievements.leaderboards"));
	//retroachievements->addWithLabel(_("LEADERBOARDS"), retroachievements_leaderboards_enabled);
	//retroachievements->addSaveFunc([retroachievements_leaderboards_enabled] { SystemConf::getInstance()->setBool("global.retroachievements.///leaderboards", retroachievements_leaderboards_enabled->getState()); });

	// retroachievements_leaderboards list
	auto retroachievements_leaderboards_list = std::make_shared< OptionListComponent<std::string> >(mWindow, _("LEADERBOARDS"), false);
	std::vector<std::string> leader;
	leader.push_back("disabled");
	leader.push_back("enabled");
	leader.push_back("trackers only");
	leader.push_back("notifications only");

	auto currentLeader = SystemConf::getInstance()->get("global.retroachievements.leaderboards");
	if (currentLeader.empty())
		currentLeader = "disabled";

	for (auto it = leader.cbegin(); it != leader.cend(); it++)
		retroachievements_leaderboards_list->add(_(it->c_str()), *it, currentLeader == *it);

		retroachievements->addWithLabel(_("LEADERBOARDS"), retroachievements_leaderboards_list);
		retroachievements->addSaveFunc([retroachievements_leaderboards_list]
	{
		SystemConf::getInstance()->set("global.retroachievements.leaderboards", retroachievements_leaderboards_list->getSelected());
		SystemConf::getInstance()->saveSystemConf();
	});

	// retroachievements_challenge_indicators
	auto retroachievements_challenge_indicators = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_challenge_indicators->setState(SystemConf::getInstance()->getBool("global.retroachievements.challengeindicators"));
	retroachievements->addWithLabel(_("CHALLENGE INDICATORS"), retroachievements_challenge_indicators);
	retroachievements->addSaveFunc([retroachievements_challenge_indicators] { SystemConf::getInstance()->setBool("global.retroachievements.challengeindicators", retroachievements_challenge_indicators->getState()); });

	// retroachievements_richpresence_enable
	auto retroachievements_richpresence_enable = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_richpresence_enable->setState(SystemConf::getInstance()->getBool("global.retroachievements.richpresence"));
	retroachievements->addWithLabel(_("RICH PRESENCE"), retroachievements_richpresence_enable);
	retroachievements->addSaveFunc([retroachievements_richpresence_enable] { SystemConf::getInstance()->setBool("global.retroachievements.richpresence", retroachievements_richpresence_enable->getState()); });

	// retroachievements_badges_enable
	auto retroachievements_badges_enable = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_badges_enable->setState(SystemConf::getInstance()->getBool("global.retroachievements.badges"));
	retroachievements->addWithLabel(_("BADGES"), retroachievements_badges_enable);
	retroachievements->addSaveFunc([retroachievements_badges_enable] { SystemConf::getInstance()->setBool("global.retroachievements.badges", retroachievements_badges_enable->getState()); });

	// retroachievements_test_unofficial
	auto retroachievements_test_unofficial = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_test_unofficial->setState(SystemConf::getInstance()->getBool("global.retroachievements.testunofficial"));
	retroachievements->addWithLabel(_("TEST UNOFFICIAL ACHIEVEMENTS"), retroachievements_test_unofficial);
	retroachievements->addSaveFunc([retroachievements_test_unofficial] { SystemConf::getInstance()->setBool("global.retroachievements.testunofficial", retroachievements_test_unofficial->getState()); });

	// retroachievements_unlock_sound_enable
	auto retroachievements_unlock_sound_enable = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_unlock_sound_enable->setState(SystemConf::getInstance()->getBool("global.retroachievements.soundenable"));
	retroachievements->addWithLabel(_("UNLOCK SOUND"), retroachievements_unlock_sound_enable);
	retroachievements->addSaveFunc([retroachievements_unlock_sound_enable] { SystemConf::getInstance()->setBool("global.retroachievements.soundenable", retroachievements_unlock_sound_enable->getState()); });

	// retroachievements_verbose_mode
	auto retroachievements_verbose_enabled = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_verbose_enabled->setState(SystemConf::getInstance()->getBool("global.retroachievements.verbose"));
	retroachievements->addWithLabel(_("VERBOSE MODE"), retroachievements_verbose_enabled);
	retroachievements->addSaveFunc([retroachievements_verbose_enabled] { SystemConf::getInstance()->setBool("global.retroachievements.verbose", retroachievements_verbose_enabled->getState()); });

	// retroachievements_automatic_screenshot
	auto retroachievements_screenshot_enabled = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_screenshot_enabled->setState(SystemConf::getInstance()->getBool("global.retroachievements.screenshot"));
	retroachievements->addWithLabel(_("AUTOMATIC SCREENSHOT"), retroachievements_screenshot_enabled);
	retroachievements->addSaveFunc([retroachievements_screenshot_enabled] { SystemConf::getInstance()->setBool("global.retroachievements.screenshot", retroachievements_screenshot_enabled->getState()); });

	// retroachievements_start_active
	auto retroachievements_start_active = std::make_shared<SwitchComponent>(mWindow);
	retroachievements_start_active->setState(SystemConf::getInstance()->getBool("global.retroachievements.active"));
	retroachievements->addWithLabel(_("ENCORE MODE (LOCAL RESET OF ACHIEVEMENTS)"), retroachievements_start_active);
	retroachievements->addSaveFunc([retroachievements_start_active] { SystemConf::getInstance()->setBool("global.retroachievements.active", retroachievements_start_active->getState()); });

	// Unlock sound
	auto installedRSounds = ApiSystem::getInstance()->getRetroachievementsSoundsList();
	if (installedRSounds.size() > 0)
	{
		std::string currentSound = SystemConf::getInstance()->get("global.retroachievements.sound");

		auto rsounds_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("RETROACHIEVEMENT UNLOCK SOUND"), false);
		rsounds_choices->add(_("none"), "none", currentSound.empty() || currentSound == "none");

		for (auto snd : installedRSounds)
			rsounds_choices->add(_(Utils::String::toUpper(snd).c_str()), snd, currentSound == snd);

		if (!rsounds_choices->hasSelection())
			rsounds_choices->selectFirstItem();

		retroachievements->addWithLabel(_("UNLOCK SOUND"), rsounds_choices);
		retroachievements->addSaveFunc([rsounds_choices] { SystemConf::getInstance()->set("global.retroachievements.sound", rsounds_choices->getSelected()); });
	}

	// retroachievements, username, password
	retroachievements->addInputTextRow(_("USERNAME"), "global.retroachievements.username", false);
	retroachievements->addInputTextRow(_("PASSWORD"), "global.retroachievements.password", true);

	retroachievements->addGroup(_("GAME INDEXES"));

	// CheckOnStart
	auto checkOnStart = std::make_shared<SwitchComponent>(mWindow);
	checkOnStart->setState(Settings::getInstance()->getBool("CheevosCheckIndexesAtStart"));
	retroachievements->addWithLabel(_("INDEX NEW GAMES AT STARTUP"), checkOnStart);
	retroachievements->addSaveFunc([checkOnStart] { Settings::getInstance()->setBool("CheevosCheckIndexesAtStart", checkOnStart->getState()); });

	// Index games
	retroachievements->addEntry(_("INDEX GAMES"), true, [this]
	{
		if (ThreadedHasher::checkCloseIfRunning(mWindow))
			mWindow->pushGui(new GuiHashStart(mWindow, ThreadedHasher::HASH_CHEEVOS_MD5));
	});


	retroachievements->addSaveFunc([retroachievementsEnabled, retroachievements_enabled, username, password, window]
	{
		bool newState = retroachievements_enabled->getState();
		std::string newUsername = SystemConf::getInstance()->get("global.retroachievements.username");
		std::string newPassword = SystemConf::getInstance()->get("global.retroachievements.password");

		if (newState && (!retroachievementsEnabled || username != newUsername || password != newPassword))
		{
			std::string error;
			if (!RetroAchievements::testAccount(newUsername, newPassword, error))
			{
				window->pushGui(new GuiMsgBox(window, _("UNABLE TO ACTIVATE RETROACHIEVEMENTS:") + "\n" + error, _("OK"), nullptr, GuiMsgBoxIcon::ICON_ERROR));
				retroachievements_enabled->setState(false);
				newState = false;
			}
		}

		if (SystemConf::getInstance()->setBool("global.retroachievements", newState))
			if (!ThreadedHasher::isRunning() && newState)
				ThreadedHasher::start(window, ThreadedHasher::HASH_CHEEVOS_MD5, false, true);
	});

	mWindow->pushGui(retroachievements);
}

template <typename StructType, typename FieldSelectorUnaryFn>
static auto groupBy(const std::vector<StructType>& instances, const FieldSelectorUnaryFn& fieldChooser) // -> std::map<decltype(forward<FieldSelectorUnaryFn>(fieldChooser)), std::vector<StructType>>
{
	StructType _;
	using FieldType = decltype(fieldChooser(_));
	std::map<FieldType, std::vector<StructType>> instancesByField;
	for (auto& instance : instances)
	{
		instancesByField[fieldChooser(instance)].push_back(instance);
	}
	return instancesByField;
}

void GuiMenu::openNetplaySettings()
{
	GuiSettings* settings = new GuiSettings(mWindow, _("NETPLAY SETTINGS").c_str());

	settings->addGroup(_("SETTINGS"));

	// Enable
	auto enableNetplay = std::make_shared<SwitchComponent>(mWindow);
	enableNetplay->setState(SystemConf::getInstance()->getBool("global.netplay"));
	settings->addWithLabel(_("ENABLE NETPLAY"), enableNetplay);

	std::string port = SystemConf::getInstance()->get("global.netplay.port");
	if (port.empty())
		SystemConf::getInstance()->set("global.netplay.port", "55435");

	settings->addInputTextRow(_("NICKNAME"), "global.netplay.nickname", false);

	bool adhocEnabled = SystemConf::getInstance()->getBool("network.adhoc.enabled");
	std::string mitm = SystemConf::getInstance()->get("global.netplay.relay");
	auto mitms = std::make_shared<OptionListComponent<std::string> >(mWindow, _("USE RELAY SERVER"), false);

        if (!adhocEnabled)
	{
		settings->addInputTextRow(_("HOST"), "global.netplay.host", false);
		settings->addInputTextRow(_("PORT"), "global.netplay.port", false);

		// RELAY SERVER
		mitms->add(_("NONE"), "", mitm.empty() || mitm == "none");
		mitms->add("NEW YORK", "nyc", mitm == "nyc");
		mitms->add("MADRID", "madrid", mitm == "madrid");
		mitms->add("MONTREAL", "montreal", mitm == "montreal");
		mitms->add("SAO PAULO", "saopaulo", mitm == "saopaulo");

		if (!mitms->hasSelection())
			mitms->selectFirstItem();

		settings->addWithLabel(_("USE RELAY SERVER"), mitms);

	}
	settings->addGroup(_("GAME INDEXES"));

	// CheckOnStart
	auto checkOnStart = std::make_shared<SwitchComponent>(mWindow);
	checkOnStart->setState(Settings::getInstance()->getBool("NetPlayCheckIndexesAtStart"));
	settings->addWithLabel(_("INDEX NEW GAMES AT STARTUP"), checkOnStart);

	Window* window = mWindow;
	settings->addSaveFunc([enableNetplay, checkOnStart, mitms, window]
	{
		Settings::getInstance()->setBool("NetPlayCheckIndexesAtStart", checkOnStart->getState());
		SystemConf::getInstance()->set("global.netplay.relay", mitms->getSelected());

		if (SystemConf::getInstance()->setBool("global.netplay", enableNetplay->getState()))
		{
			if (!ThreadedHasher::isRunning() && enableNetplay->getState())
			{
				ThreadedHasher::start(window, ThreadedHasher::HASH_NETPLAY_CRC, false, true);
			}
		}
	});

	settings->addEntry(_("INDEX GAMES"), true, [this]
	{
		if (ThreadedHasher::checkCloseIfRunning(mWindow))
			mWindow->pushGui(new GuiHashStart(mWindow, ThreadedHasher::HASH_NETPLAY_CRC));
	});

	//settings->addEntry(_("FIND ALL GAMES"), false, [this] { ThreadedHasher::start(mWindow, ThreadedHasher::HASH_NETPLAY_CRC, true); });
	//settings->addEntry(_("FIND NEW GAMES"), false, [this] { ThreadedHasher::start(mWindow, ThreadedHasher::HASH_NETPLAY_CRC); });

	mWindow->pushGui(settings);
}

void GuiMenu::addDecorationSetOptionListComponent(Window* window, GuiSettings* parentWindow, const std::vector<DecorationSetInfo>& sets, const std::string& configName)
{
	auto decorations = std::make_shared<OptionListComponent<std::string> >(window, _("DECORATION SET"), false);
	decorations->setRowTemplate([window, sets](std::string data, ComponentListRow& row) { createDecorationItemTemplate(window, sets, data, row); });

	std::vector<std::string> items = { _("DEFAULT"), _("NONE") };
	for (auto set : sets)
		items.push_back(set.name);

	std::string bezel = SystemConf::getInstance()->get(configName + ".bezel");

	for (auto item : items)
		decorations->add(item, item, (bezel == item) || (bezel == "none" && item == _("NONE")) || (bezel == "" && item == _("DEFAULT")));

	if (!decorations->hasSelection())
		decorations->selectFirstItem();

	parentWindow->addWithLabel(_("DECORATION SET"), decorations);
	parentWindow->addSaveFunc([decorations, configName]
	{
		SystemConf::getInstance()->set(configName + ".bezel", decorations->getSelected() == _("NONE") ? "none" : decorations->getSelected() == _("DEFAULT") ? "" : decorations->getSelected());
	});
};

void GuiMenu::openGamesSettings_batocera()
{
	Window* window = mWindow;

	auto s = new GuiSettings(mWindow, _("GAME SETTINGS").c_str());

	if (SystemConf::getInstance()->get("system.es.menu") != "bartop")
	{
		s->addGroup(_("TOOLS"));

		// Game List Update
		s->addEntry(_("UPDATE GAMELISTS"), false, [this, window] { updateGameLists(window); });

	}

	s->addGroup(_("DEFAULT GLOBAL SETTINGS"));

	// Screen ratio choice
	if (SystemConf::getInstance()->get("system.es.menu") != "bartop")
	{
		auto ratio_choice = createRatioOptionList(mWindow, "global");
		s->addWithLabel(_("GAME ASPECT RATIO"), ratio_choice);
		s->addSaveFunc([ratio_choice] { SystemConf::getInstance()->set("global.ratio", ratio_choice->getSelected()); });
	}

	// bilinear filtering
	auto smoothing_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("BILINEAR FILTERING"));
	smoothing_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get("global.smooth"));
	s->addWithLabel(_("BILINEAR FILTERING"), smoothing_enabled);
	s->addSaveFunc([smoothing_enabled] { SystemConf::getInstance()->set("global.smooth", smoothing_enabled->getSelected()); });

	// rewind
	auto rewind_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("REWIND"));
	rewind_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get("global.rewind"));
	s->addWithLabel(_("REWIND"), rewind_enabled);
	s->addSaveFunc([rewind_enabled] { SystemConf::getInstance()->set("global.rewind", rewind_enabled->getSelected()); });

	// Integer scale
	auto integerscale_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("INTEGER SCALING (PIXEL PERFECT)"));
	integerscale_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get("global.integerscale"));
	s->addWithLabel(_("INTEGER SCALING (PIXEL PERFECT)"), integerscale_enabled);
	s->addSaveFunc([integerscale_enabled] { SystemConf::getInstance()->set("global.integerscale", integerscale_enabled->getSelected()); });

	// Integer scale overscale
	auto integerscaleoverscale_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("INTEGER SCALE OVERSCALE"));
	integerscaleoverscale_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get("global.integerscaleoverscale"));
	s->addWithLabel(_("INTEGER SCALE OVERSCALE"), integerscaleoverscale_enabled);
	s->addSaveFunc([integerscaleoverscale_enabled] { SystemConf::getInstance()->set("global.integerscaleoverscale", integerscaleoverscale_enabled->getSelected()); });
	
	// Saves Menu
	s->addEntry(_("SAVE STATE CONFIG"), true, [this] { openSavestatesConfiguration(mWindow, "global"); });


	std::string currentShader = SystemConf::getInstance()->get("global.shaderset");

	// Shaders preset
	auto shaders_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("SHADER SET"), false);
	shaders_choices->add(_("DEFAULT"), "default", currentShader.empty() || currentShader == "default");
	shaders_choices->add(_("NONE"), "none", currentShader == "none");
	std::string a;
	for(std::stringstream ss(getShOutput(R"(/usr/bin/getshaders)")); getline(ss, a, ','); )
	shaders_choices->add(a, a, currentShader == a); // emuelec
	s->addWithLabel(_("SHADER SET"), shaders_choices);
	s->addSaveFunc([shaders_choices] { SystemConf::getInstance()->set("global.shaderset", shaders_choices->getSelected()); });

	// Filters preset
	std::string currentFilter = SystemConf::getInstance()->get("global.filterset");
	auto filters_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("FILTER SET"), false);
	filters_choices->add(_("DEFAULT"), "default", currentFilter.empty() || currentFilter == "default");
	filters_choices->add(_("NONE"), "none", currentFilter == "none");
	std::string filterList;
	for(std::stringstream ss(getShOutput(R"(/usr/bin/getfilters)")); getline(ss, filterList, ','); )
		filters_choices->add(filterList, filterList, currentFilter == filterList); // emuelec
	s->addWithLabel(_("FILTER SET"), filters_choices);
	s->addSaveFunc([filters_choices] { SystemConf::getInstance()->set("global.filterset", filters_choices->getSelected()); });

	// decorations
	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::DECORATIONS))
	{
		auto sets = GuiMenu::getDecorationsSets(ViewController::get()->getState().getSystem());
		if (sets.size() > 0)
		{
				auto decorations = std::make_shared<OptionListComponent<std::string> >(mWindow, _("DECORATION SET"), false);
				decorations->setRowTemplate([window, sets](std::string data, ComponentListRow& row)
				{
					createDecorationItemTemplate(window, sets, data, row);
				});

				std::vector<std::string> decorations_item;
				decorations_item.push_back(_("DEFAULT"));
				decorations_item.push_back(_("NONE"));
				for (auto set : sets)
					decorations_item.push_back(set.name);

				for (auto it = decorations_item.begin(); it != decorations_item.end(); it++)
					decorations->add(*it, *it,
					(SystemConf::getInstance()->get("global.bezel") == *it) ||
						(SystemConf::getInstance()->get("global.bezel") == "none" && *it == _("NONE")) ||
						(SystemConf::getInstance()->get("global.bezel") == "" && *it == _("DEFAULT")));

			s->addWithLabel(_("DECORATION SET"), decorations);

			decorations->setSelectedChangedCallback([decorations](std::string value)
			{
				if (Utils::String::toLower(value) == "default") {
					value = "";
				}
				LOG(LogDebug) << "Setting bezel on change to: " << value;

				SystemConf::getInstance()->set("global.bezel", value);
			});

			if (decorations->getSelectedName() == "")
			{
				decorations->selectFirstItem();
			}

		}
	}

	// latency reduction
	s->addEntry(_("LATENCY REDUCTION"), true, [this] { openLatencyReductionConfiguration(mWindow, "global"); });

	//AI-enabled translations
	s->addEntry(_("AI GAME TRANSLATION"), true, [this]
	{
		GuiSettings *ai_service = new GuiSettings(mWindow, _("AI GAME TRANSLATION").c_str());

		// AI service enabled?
		auto ai_service_enabled = std::make_shared<SwitchComponent>(mWindow);
		ai_service_enabled->setState(
			SystemConf::getInstance()->get("global.ai_service_enabled") == "1");
		ai_service->addWithLabel(_("ENABLE AI TRANSLATION SERVICE"), ai_service_enabled);

		// Target language - order is: popular languages in the Batocera community first
		// then alphabetical order of the 2-char lang code (because the strings are localized)
		auto lang_choices = std::make_shared<OptionListComponent<std::string> >(mWindow,
			_("TARGET LANGUAGE"), false);
		std::string currentLang = SystemConf::getInstance()->get("global.ai_target_lang");
		if (currentLang.empty())
			currentLang = std::string("En");
		lang_choices->add("ENGLISH", "En", currentLang == "En");
		lang_choices->add("FRANÇAIS", "Fr", currentLang == "Fr");
		lang_choices->add("PORTUGUES", "Pt", currentLang == "Pt");
		lang_choices->add("DEUTSCH", "De", currentLang == "De");
		lang_choices->add("GREEK", "El", currentLang == "El");
		lang_choices->add("ESPAÑOL", "Es", currentLang == "Es");
		lang_choices->add("CZECH", "Cs", currentLang == "Cs");
		lang_choices->add("DANISH", "Da", currentLang == "Da");
		lang_choices->add("CROATIAN", "Hr", currentLang == "Hr");
		lang_choices->add("HUNGARIAN", "Hu", currentLang == "Hu");
		lang_choices->add("ITALIANO", "It", currentLang == "It");
		lang_choices->add("JAPANESE", "Ja", currentLang == "Ja");
		lang_choices->add("KOREAN", "Ko", currentLang == "Ko");
		lang_choices->add("DUTCH", "Nl", currentLang == "Nl");
		lang_choices->add("NORWEGIAN", "Nn", currentLang == "Nn");
		lang_choices->add("POLISH", "Po", currentLang == "Po");
		lang_choices->add("ROMANIAN", "Ro", currentLang == "Ro");
		lang_choices->add("РУССКИЙ", "Ru", currentLang == "Ru");
		lang_choices->add("SVENSKA", "Sv", currentLang == "Sv");
		lang_choices->add("TÜRKÇE", "Tr", currentLang == "Tr");
		lang_choices->add("简体中文", "Zh", currentLang == "Zh");
		ai_service->addWithLabel(_("TARGET LANGUAGE"), lang_choices);

		// Service  URL
		ai_service->addInputTextRow(_("AI TRANSLATION SERVICE URL"), "global.ai_service_url", false);

		// Pause game for translation?
		auto ai_service_pause = std::make_shared<SwitchComponent>(mWindow);
		ai_service_pause->setState(
			SystemConf::getInstance()->get("global.ai_service_pause") == "1");
		ai_service->addWithLabel(_("PAUSE ON TRANSLATED SCREEN"), ai_service_pause);

		ai_service->addSaveFunc([ai_service_enabled, lang_choices, ai_service_pause] {
			if (ai_service_enabled->changed())
				SystemConf::getInstance()->set("global.ai_service_enabled",
					ai_service_enabled->getState() ? "1" : "0");
			if (lang_choices->changed())
				SystemConf::getInstance()->set("global.ai_target_lang",
					lang_choices->getSelected());
			if (ai_service_pause->changed())
				SystemConf::getInstance()->set("global.ai_service_pause",
					ai_service_pause->getState() ? "1" : "0");
			SystemConf::getInstance()->saveSystemConf();
		});

		mWindow->pushGui(ai_service);
	});

	auto groups = groupBy(SystemData::mGlobalFeatures, [](const CustomFeature& item) { return item.submenu; });
	for (auto group : groups)
	{
		if (!group.first.empty())
		{
			s->addEntry(group.first, true, [this, group]
			{
				GuiSettings* groupSettings = new GuiSettings(mWindow, _(group.first.c_str()));

				for (auto feat : group.second)
				{
					std::string storageName = "global." + feat.value;
					std::string storedValue = SystemConf::getInstance()->get(storageName);

					auto cf = std::make_shared<OptionListComponent<std::string>>(mWindow, _(feat.name.c_str()));
					cf->add(_("DEFAULT"), "", storedValue.empty() || storedValue == "default");

					for (auto fval : feat.choices)
						cf->add(_(fval.name.c_str()), fval.value, storedValue == fval.value);

					if (!cf->hasSelection())
						cf->selectFirstItem();

					if (!feat.description.empty())
						groupSettings->addWithDescription(_(feat.name.c_str()), _(feat.description.c_str()), cf);
					else
						groupSettings->addWithLabel(_(feat.name.c_str()), cf);

					groupSettings->addSaveFunc([cf, storageName] { SystemConf::getInstance()->set(storageName, cf->getSelected()); });
				}

				mWindow->pushGui(groupSettings);
			});
		}
		else
		{
			// Load global custom features
			for (auto feat : group.second)
			{
				std::string storageName = "global." + feat.value;
				std::string storedValue = SystemConf::getInstance()->get(storageName);

			auto cf = std::make_shared<OptionListComponent<std::string>>(mWindow, _(feat.name.c_str()));
			cf->add(_("DEFAULT"), "", storedValue.empty() || storedValue == "default");

				for (auto fval : feat.choices)
					cf->add(_(fval.name.c_str()), fval.value, storedValue == fval.value);

				if (!cf->hasSelection())
					cf->selectFirstItem();

				if (!feat.description.empty())
					s->addWithDescription(_(feat.name.c_str()), _(feat.description.c_str()), cf);
				else
					s->addWithLabel(_(feat.name.c_str()), cf);

				s->addSaveFunc([cf, storageName] { SystemConf::getInstance()->set(storageName, cf->getSelected()); });
			}
		}
	}

	// Custom config for systems
	s->addGroup(_("SETTINGS"));

	s->addEntry(_("PER SYSTEM ADVANCED CONFIGURATION"), true, [this, s, window]
	{
		s->save();
		GuiSettings* configuration = new GuiSettings(window, _("PER SYSTEM ADVANCED CONFIGURATION").c_str());

		// For each activated system
		std::vector<SystemData *> systems = SystemData::sSystemVector;
		for (auto system : systems)
		{
			if (system->isCollection() || !system->isGameSystem())
				continue;

			if (system->hasPlatformId(PlatformIds::PLATFORM_IGNORE))
				continue;

			if (!system->hasFeatures() && !system->hasEmulatorSelection())
				continue;

			configuration->addEntry(system->getFullName(), true, [this, system, window] {
				popSystemConfigurationGui(window, system);
			});
		}

		window->pushGui(configuration);
	});

	if (SystemConf::getInstance()->get("system.es.menu") != "bartop")
	{
		s->addGroup(_("SYSTEM SETTINGS"));

		// Retroachievements
		if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::RETROACHIVEMENTS))
		{
			s->addEntry(_("RETROACHIEVEMENT SETTINGS"), true, [this] { openRetroachievementsSettings(); });
		}

		// Netplay
		if (SystemData::isNetplayActivated() && ApiSystem::getInstance()->isScriptingSupported(ApiSystem::NETPLAY))
			s->addEntry(_("NETPLAY SETTINGS"), true, [this] { openNetplaySettings(); }, "iconNetplay");

	}

	mWindow->pushGui(s);
}

void GuiMenu::openMissingBiosSettings()
{
	GuiBios::show(mWindow);
}

void GuiMenu::updateGameLists(Window* window, bool confirm)
{
	if (ThreadedScraper::isRunning())
	{
		window->pushGui(new GuiMsgBox(window, _("SCRAPER IS RUNNING. DO YOU WANT TO STOP IT?"),
			_("YES"), [] { ThreadedScraper::stop(); },
			_("NO"), nullptr));

		return;
	}

	if (ThreadedHasher::isRunning())
	{
		window->pushGui(new GuiMsgBox(window, _("GAME HASHING IS RUNNING. DO YOU WANT TO STOP IT?"),
			_("YES"), [] { ThreadedHasher::stop(); },
			_("NO"), nullptr));

		return;
	}

	if (!confirm)
	{
		ViewController::reloadAllGames(window, true, true);
		return;
	}

	window->pushGui(new GuiMsgBox(window, _("REALLY UPDATE GAMELISTS?"), _("YES"), [window]
		{
		ViewController::reloadAllGames(window, true, true);
		},
		_("NO"), nullptr));
}

void GuiMenu::openSystemEmulatorSettings(SystemData* system)
{
	auto theme = ThemeData::getMenuTheme();

	GuiSettings* s = new GuiSettings(mWindow, system->getFullName().c_str());

	auto emul_choice = std::make_shared<OptionListComponent<std::string>>(mWindow, _("Emulator"), false);
	auto core_choice = std::make_shared<OptionListComponent<std::string>>(mWindow, _("Core"), false);

	std::string currentEmul = system->getEmulator(false);
	std::string defaultEmul = system->getDefaultEmulator();

	emul_choice->add(_("DEFAULT"), "", false);

	bool found = false;
	for (auto emul : system->getEmulators())
	{
		if (emul.name == currentEmul)
			found = true;

		emul_choice->add(emul.name, emul.name, emul.name == currentEmul);
	}

	if (!found)
		emul_choice->selectFirstItem();

	ComponentListRow row;
	row.addElement(std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(_("Emulator")), theme->Text.font, theme->Text.color), true);
	row.addElement(emul_choice, false);

	s->addRow(row);

	emul_choice->setSelectedChangedCallback([this, system, core_choice](std::string emulatorName)
	{
		std::string currentCore = system->getCore(false);
		std::string defaultCore = system->getDefaultCore(emulatorName);

		core_choice->clear();
		core_choice->add(_("DEFAULT"), "", false);

		bool found = false;

		for (auto& emulator : system->getEmulators())
		{
			if (emulatorName != emulator.name)
				continue;

			for (auto core : emulator.cores)
			{
				core_choice->add(core.name, core.name, currentCore == core.name);
				if (currentCore == core.name)
					found = true;
			}
		}

		if (!found)
			core_choice->selectFirstItem();
		else
			core_choice->invalidate();
	});

	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, Utils::String::toUpper(_("Core")), theme->Text.font, theme->Text.color), true);
	row.addElement(core_choice, false);
	s->addRow(row);

	// force change event to load core list
	emul_choice->invalidate();


	s->addSaveFunc([system, emul_choice, core_choice]
	{
		Settings::getInstance()->setString(system->getName() + ".emulator", emul_choice->getSelected());
		Settings::getInstance()->setString(system->getName() + ".core", core_choice->getSelected());
	});

	mWindow->pushGui(s);
}

void GuiMenu::openEmulatorSettings()
{
	GuiSettings* configuration = new GuiSettings(mWindow, _("EMULATOR SETTINGS").c_str());

	Window* window = mWindow;

	// For each activated system
	for (auto system : SystemData::sSystemVector)
	{
		if (system->isCollection())
			continue;

		if (system->getEmulators().size() == 0)
			continue;

		if (system->getEmulators().size() == 1 && system->getEmulators().cbegin()->cores.size() <= 1)
			continue;

		configuration->addEntry(system->getFullName(), true, [this, system] { openSystemEmulatorSettings(system); });
	}

	window->pushGui(configuration);
}

void GuiMenu::openControllersSettings_batocera(int autoSel)
{
	GuiSettings* s = new GuiSettings(mWindow, controllers_settings_label.c_str());

	Window *window = mWindow;

	// CONTROLLER CONFIGURATION

	// Provides a mechanism to disable automatic hotkey assignment
	bool HotKeysEnabled = SystemConf::getInstance()->getBool("system.autohotkeys");
	auto autohotkeys = std::make_shared<SwitchComponent>(mWindow);
	autohotkeys->setState(HotKeysEnabled);
	s->addWithLabel(_("AUTOCONFIGURE RETROARCH HOTKEYS"), autohotkeys);
	s->addSaveFunc([autohotkeys] {
		SystemConf::getInstance()->setBool("system.autohotkeys", autohotkeys->getState());
	});

	s->addEntry(_("CONTROLLER MAPPING"), false, [window, this, s]
	{
		window->pushGui(new GuiMsgBox(window,
			_("YOU ARE GOING TO MAP A CONTROLLER. MAP BASED ON THE BUTTON'S POSITION "
				"RELATIVE TO ITS EQUIVALENT ON A SNES CONTROLLER, NOT ITS PHYSICAL LABEL. "
				"IF YOU DO NOT HAVE A SPECIAL KEY FOR HOTKEY, USE THE SELECT BUTTON. SKIP "
				"ALL BUTTONS/STICKS YOU DO NOT HAVE BY HOLDING ANY KEY. PRESS THE "
				"SOUTH BUTTON TO CONFIRM WHEN DONE."), _("OK"),
			[window, this, s] {
			window->pushGui(new GuiDetectDevice(window, false, [this, s] {
				s->setSave(false);
				delete s;
				this->openControllersSettings_batocera();
			}));
		}));
	});

	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::BLUETOOTH))
	{
		// BLUETOOTH TOGGLE
		auto bluetoothd_enabled = std::make_shared<SwitchComponent>(mWindow);
		bool btbaseEnabled = SystemConf::getInstance()->get("bluetooth.enabled") == "1";
		bluetoothd_enabled->setState(btbaseEnabled);
		s->addWithLabel(_("ENABLE BLUETOOTH"), bluetoothd_enabled);
		bluetoothd_enabled->setOnChangedCallback([this, s, bluetoothd_enabled]() {
			if (bluetoothd_enabled->getState() == false) {
                                runSystemCommand("systemctl stop bluetooth bluetoothsense bluetooth-agent", "", nullptr);
                                runSystemCommand("rfkill block bluetooth", "", nullptr);
			} else {
                                runSystemCommand("systemctl start bluetooth bluetooth-agent bluetoothsense", "", nullptr);
                                runSystemCommand("rfkill unblock bluetooth", "", nullptr);
			}
			bool bluetoothenabled = bluetoothd_enabled->getState();
			SystemConf::getInstance()->set("bluetooth.enabled", bluetoothenabled ? "1" : "0");
			SystemConf::getInstance()->saveSystemConf();
		});

		// PAIR A BLUETOOTH CONTROLLER OR BT AUDIO DEVICE
		s->addEntry(_("PAIR A BLUETOOTH DEVICE"), false, [window, bluetoothd_enabled] {
			if (bluetoothd_enabled->getState() == false) {
				window->pushGui(new GuiMsgBox(window, _("BLUETOOTH IS DISABLED")));
			} else {
				ThreadedBluetooth::start(window);
			}
		});

		// FORGET BLUETOOTH CONTROLLERS OR BT AUDIO DEVICES
		s->addEntry(_("FORGET A BLUETOOTH DEVICE"), false, [window, this, s]
		{
			window->pushGui(new GuiBluetooth(window));
		});
	}

	ComponentListRow row;

	// Here we go; for each player
	std::list<int> alreadyTaken = std::list<int>();

	// clear the current loaded inputs
	clearLoadedInput();

	std::vector<std::shared_ptr<OptionListComponent<StrInputConfig *>>> options;
	//char strbuf[256];

	auto configList = InputManager::getInstance()->getInputConfigs();

	for (int player = 0; player < MAX_PLAYERS; player++)
	{
		std::string label = Utils::String::format(_("P%i'S CONTROLLER").c_str(), player + 1);
		std::string confName = Utils::String::format("INPUT P%iNAME", player + 1);
		std::string confGuid = Utils::String::format("INPUT P%iGUID", player + 1);

		LOG(LogInfo) << player + 1 << " " << confName << " " << confGuid;
		auto inputOptionList = std::make_shared<OptionListComponent<StrInputConfig *> >(mWindow, label, false);
		inputOptionList->add(_("default"), nullptr, false);
		options.push_back(inputOptionList);

		// Checking if a setting has been saved, else setting to default
		std::string configuratedName = Settings::getInstance()->getString(confName);
		std::string configuratedGuid = Settings::getInstance()->getString(confGuid);
		bool found = false;

		// For each available and configured input
		for (auto config : configList)
		{
			// create name
			std::stringstream dispNameSS;
			dispNameSS << "#" << config->getDeviceIndex() << " ";

			std::string deviceName = config->getDeviceName();
			if (deviceName.size() > 25)
				dispNameSS << deviceName.substr(0, 16) << "..." << deviceName.substr(deviceName.size() - 5, deviceName.size() - 1);
			else
				dispNameSS << deviceName;

			std::string displayName = dispNameSS.str();

			bool foundFromConfig = configuratedName == config->getDeviceName() && configuratedGuid == config->getDeviceGUIDString();
			int deviceID = config->getDeviceId();
			// Si la manette est configurée, qu'elle correspond a la configuration, et qu'elle n'est pas
			// deja selectionnée on l'ajoute en séléctionnée
			StrInputConfig* newInputConfig = new StrInputConfig(config->getDeviceName(), config->getDeviceGUIDString());
			mLoadedInput.push_back(newInputConfig);

			if (foundFromConfig && std::find(alreadyTaken.begin(), alreadyTaken.end(), deviceID) == alreadyTaken.end() && !found)
			{
				found = true;
				alreadyTaken.push_back(deviceID);

				LOG(LogWarning) << "adding entry for player" << player << " (selected): " << config->getDeviceName() << "  " << config->getDeviceGUIDString();
				inputOptionList->add(displayName, newInputConfig, true);
			}
			else
			{
				LOG(LogInfo) << "adding entry for player" << player << " (not selected): " << config->getDeviceName() << "  " << config->getDeviceGUIDString();
				inputOptionList->add(displayName, newInputConfig, false);
			}
		}

		if (!inputOptionList->hasSelection())
			inputOptionList->selectFirstItem();

		// Populate controllers list
		s->addWithLabel(label, inputOptionList);
	}

	s->addSaveFunc([this, options, window]
	{
		bool changed = false;

		for (int player = 0; player < MAX_PLAYERS; player++)
		{
			std::stringstream sstm;
			sstm << "INPUT P" << player + 1;
			std::string confName = sstm.str() + "NAME";
			std::string confGuid = sstm.str() + "GUID";

			auto input = options.at(player);

			StrInputConfig* selected = input->getSelected();
			if (selected == nullptr)
			{
				changed |= Settings::getInstance()->setString(confName, "DEFAULT");
				changed |= Settings::getInstance()->setString(confGuid, "");
			}
			else if (input->changed())
			{
				LOG(LogWarning) << "Found the selected controller ! : name in list  = " << input->getSelectedName();
				LOG(LogWarning) << "Found the selected controller ! : guid  = " << selected->deviceGUIDString;

				changed |= Settings::getInstance()->setString(confName, selected->deviceName);
				changed |= Settings::getInstance()->setString(confGuid, selected->deviceGUIDString);
			}
		}

		if (changed)
			Settings::getInstance()->saveFile();

		// this is dependant of this configuration, thus update it
		InputManager::getInstance()->computeLastKnownPlayersDeviceIndexes();
	});

	// CONTROLLER ACTIVITY
	auto activity = std::make_shared<SwitchComponent>(mWindow);
	activity->setState(Settings::getInstance()->getBool("ShowControllerActivity"));
	s->addWithLabel(_("SHOW CONTROLLER ACTIVITY"), activity, autoSel == 1);
	activity->setOnChangedCallback([this, s, activity]
	{
		if (Settings::getInstance()->setBool("ShowControllerActivity", activity->getState()))
		{
			delete s;
			openControllersSettings_batocera(1);
		}
	});

	if (Settings::getInstance()->getBool("ShowControllerActivity"))
	{
		// CONTROLLER BATTERY
		auto battery = std::make_shared<SwitchComponent>(mWindow);
		battery->setState(Settings::getInstance()->getBool("ShowControllerBattery"));
		s->addWithLabel(_("SHOW CONTROLLER BATTERY LEVEL"), battery);
		s->addSaveFunc([battery] { Settings::getInstance()->setBool("ShowControllerBattery", battery->getState()); });
	}

	window->pushGui(s);
}

struct ThemeConfigOption
{
	std::string defaultSettingName;
	std::string subset;
	std::shared_ptr<OptionListComponent<std::string>> component;
};

void GuiMenu::openThemeConfiguration(Window* mWindow, GuiComponent* s, std::shared_ptr<OptionListComponent<std::string>> theme_set, const std::string systemTheme)
{
	if (theme_set != nullptr && Settings::getInstance()->getString("ThemeSet") != theme_set->getSelected())
	{
		mWindow->pushGui(new GuiMsgBox(mWindow, _("YOU MUST APPLY THE THEME BEFORE EDITING ITS CONFIGURATION"), _("OK")));
		return;
	}

	Window* window = mWindow;

	auto system = ViewController::get()->getState().getSystem();
	auto theme = system->getTheme();

	auto themeconfig = new GuiSettings(mWindow, (systemTheme.empty() ? _("THEME CONFIGURATION") : _("VIEW CUSTOMIZATION")).c_str());

	auto themeSubSets = theme->getSubSets();

	std::string viewName;
	bool showGridFeatures = true;
	if (!systemTheme.empty())
	{
		auto glv = ViewController::get()->getGameListView(system);
		viewName = glv->getName();
		std::string baseType = theme->getCustomViewBaseType(viewName);

		showGridFeatures = (viewName == "grid" || baseType == "grid");
	}

	// gamelist_style
	std::shared_ptr<OptionListComponent<std::string>> gamelist_style = nullptr;

	if (systemTheme.empty() || showGridFeatures && system != NULL && theme->hasView("grid"))
		themeconfig->addGroup(_("GAMELIST STYLE"));

	if (systemTheme.empty())
	{
		gamelist_style = std::make_shared< OptionListComponent<std::string> >(mWindow, _("GAMELIST VIEW STYLE"), false);

		std::vector<std::pair<std::string, std::string>> styles;
		styles.push_back(std::pair<std::string, std::string>("automatic", _("automatic")));

		bool showViewStyle = true;

		if (system != NULL)
		{
			auto mViews = theme->getViewsOfTheme();

			showViewStyle = mViews.size() > 1;

			for (auto it = mViews.cbegin(); it != mViews.cend(); ++it)
			{
				if (it->first == "basic" || it->first == "detailed" || it->first == "grid" || it->first == "video" || it->first == "gamecarousel")
					styles.push_back(std::pair<std::string, std::string>(it->first, _(it->first.c_str())));
				else
					styles.push_back(*it);
			}
		}
		else
		{
			styles.push_back(std::pair<std::string, std::string>("basic", _("basic")));
			styles.push_back(std::pair<std::string, std::string>("detailed", _("detailed")));
		}

		auto viewPreference = systemTheme.empty() ? Settings::getInstance()->getString("GamelistViewStyle") : system->getSystemViewMode();
		if (!theme->hasView(viewPreference))
			viewPreference = "automatic";

		for (auto it = styles.cbegin(); it != styles.cend(); it++)
			gamelist_style->add(it->second, it->first, viewPreference == it->first);

		if (!gamelist_style->hasSelection())
			gamelist_style->selectFirstItem();

		if (showViewStyle)
			themeconfig->addWithLabel(_("GAMELIST VIEW STYLE"), gamelist_style);
	}

	// Default grid size
	std::shared_ptr<OptionListComponent<std::string>> mGridSize = nullptr;
	if (showGridFeatures && system != NULL && theme->hasView("grid"))
	{
		Vector2f gridOverride =
			systemTheme.empty() ? Vector2f::parseString(Settings::getInstance()->getString("DefaultGridSize")) :
			system->getGridSizeOverride();

		auto ovv = std::to_string((int)gridOverride.x()) + "x" + std::to_string((int)gridOverride.y());

		mGridSize = std::make_shared<OptionListComponent<std::string>>(mWindow, _("DEFAULT GRID SIZE"), false);

		bool found = false;
		for (auto it = GuiGamelistOptions::gridSizes.cbegin(); it != GuiGamelistOptions::gridSizes.cend(); it++)
		{
			bool sel = (gridOverride == Vector2f(0, 0) && *it == "automatic") || ovv == *it;
			if (sel)
				found = true;

			mGridSize->add(_(it->c_str()), *it, sel);
		}

		if (!found)
			mGridSize->selectFirstItem();

		themeconfig->addWithLabel(_("DEFAULT GRID SIZE"), mGridSize);
	}



	std::map<std::string, ThemeConfigOption> options;

	Utils::String::stringVector subsetNames = theme->getSubSetNames(viewName);

	// push appliesTo at end of list
	std::sort(subsetNames.begin(), subsetNames.end(), [themeSubSets](const std::string& a, const std::string& b) -> bool
	{
		auto sa = ThemeData::getSubSet(themeSubSets, a);
		auto sb = ThemeData::getSubSet(themeSubSets, b);

		bool aHasApplies = sa.size() > 0 && !sa.cbegin()->appliesTo.empty();
		bool bHasApplies = sb.size() > 0 && !sb.cbegin()->appliesTo.empty();

		return aHasApplies < bHasApplies;
	});

	bool hasThemeOptionGroup = false;
	bool hasApplyToGroup = false;
	for (std::string subset : subsetNames) // theme->getSubSetNames(viewName)
	{
		std::string settingName = "subset." + subset;
		std::string perSystemSettingName = systemTheme.empty() ? "" : "subset." + systemTheme + "." + subset;

		if (subset == "colorset") settingName = "ThemeColorSet";
		else if (subset == "iconset") settingName = "ThemeIconSet";
		else if (subset == "menu") settingName = "ThemeMenu";
		else if (subset == "systemview") settingName = "ThemeSystemView";
		else if (subset == "gamelistview") settingName = "ThemeGamelistView";
		else if (subset == "region") settingName = "ThemeRegionName";

		auto themeColorSets = ThemeData::getSubSet(themeSubSets, subset);

		if (themeColorSets.size() > 0)
		{
			auto selectedColorSet = themeColorSets.end();
			auto selectedName = !perSystemSettingName.empty() ? Settings::getInstance()->getString(perSystemSettingName) : Settings::getInstance()->getString(settingName);

			if (!perSystemSettingName.empty() && selectedName.empty())
				selectedName = Settings::getInstance()->getString(settingName);

			for (auto it = themeColorSets.begin(); it != themeColorSets.end() && selectedColorSet == themeColorSets.end(); it++)
				if (it->name == selectedName)
					selectedColorSet = it;

			std::shared_ptr<OptionListComponent<std::string>> item = std::make_shared<OptionListComponent<std::string> >(mWindow, _(("THEME " + Utils::String::toUpper(subset)).c_str()), false);
			item->setTag(!perSystemSettingName.empty() ? perSystemSettingName : settingName);

			std::string defaultName;
			for (auto it = themeColorSets.begin(); it != themeColorSets.end(); it++)
			{
				std::string displayName = it->displayName;

				if (!systemTheme.empty())
				{
					std::string defaultValue = Settings::getInstance()->getString(settingName);
					if (defaultValue.empty())
						defaultValue = system->getTheme()->getDefaultSubSetValue(subset);

					if (it->name == defaultValue)
					{
						defaultName = Utils::String::toUpper(displayName);
						// displayName = displayName + " (" + _("DEFAULT") + ")";
					}
				}

				item->add(displayName, it->name, it == selectedColorSet);
			}

			if (selectedColorSet == themeColorSets.end())
				item->selectFirstItem();

			if (!themeColorSets.empty())
			{
				std::string displayName = themeColorSets.cbegin()->subSetDisplayName;
				if (!displayName.empty())
				{
					bool hasApplyToSubset = themeColorSets.cbegin()->appliesTo.size() > 0;

					std::string prefix;

					if (systemTheme.empty())
					{
						for (auto subsetName : themeColorSets.cbegin()->appliesTo)
						{
							std::string pfx = theme->getViewDisplayName(subsetName);
							if (!pfx.empty())
							{
								if (prefix.empty())
									prefix = pfx;
								else
									prefix = prefix + ", " + pfx;
							}
						}

						prefix = Utils::String::toUpper(prefix);
					}

					if (hasApplyToSubset && !hasApplyToGroup)
					{
						hasApplyToGroup = true;
						themeconfig->addGroup(_("GAMELIST THEME OPTIONS"));
					}
					else if (!hasApplyToSubset && !hasThemeOptionGroup)
					{
						hasThemeOptionGroup = true;
						themeconfig->addGroup(_("THEME OPTIONS"));
					}

					if (!prefix.empty())
						themeconfig->addWithDescription(displayName, prefix, item);
					else if (!defaultName.empty())
						themeconfig->addWithDescription(displayName, _("DEFAULT VALUE") + " : " + defaultName, item);
					else
						themeconfig->addWithLabel(displayName + prefix, item);
				}
				else
				{
					if (!hasThemeOptionGroup)
					{
						hasThemeOptionGroup = true;
						themeconfig->addGroup(_("THEME OPTIONS"));
					}

					themeconfig->addWithLabel(_(("THEME " + Utils::String::toUpper(subset)).c_str()), item);
				}
			}

			ThemeConfigOption opt;
			opt.component = item;
			opt.subset = subset;
			opt.defaultSettingName = settingName;
			options[!perSystemSettingName.empty() ? perSystemSettingName : settingName] = opt;
		}
		else
		{
			ThemeConfigOption opt;
			opt.component = nullptr;
			options[!perSystemSettingName.empty() ? perSystemSettingName : settingName] = opt;
		}
	}


	if (!systemTheme.empty())
	{
		themeconfig->addGroup(_("GAMELIST OPTIONS"));

		// Show favorites first in gamelists
		auto fav = Settings::getInstance()->getString(system->getName() + ".FavoritesFirst");
		auto favoritesFirst = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW FAVORITES ON TOP"), false);
		std::string defFav = Settings::getInstance()->getBool("FavoritesFirst") ? _("YES") : _("NO");
		favoritesFirst->add(_("DEFAULT"), "", fav == "" || fav == "default");
		favoritesFirst->add(_("YES"), "1", fav == "1");
		favoritesFirst->add(_("NO"), "0", fav == "0");
		themeconfig->addWithDescription(_("SHOW FAVORITES ON TOP"), _("DEFAULT VALUE") + " : " + defFav, favoritesFirst);
		themeconfig->addSaveFunc([themeconfig, favoritesFirst, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".FavoritesFirst", favoritesFirst->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Show favorites first in gamelists
		auto defHid = Settings::ShowHiddenFiles() ? _("YES") : _("NO");
		auto curhid = Settings::getInstance()->getString(system->getName() + ".ShowHiddenFiles");
		auto hiddenFiles = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW HIDDEN FILES"), false);
		hiddenFiles->add(_("DEFAULT"), "", curhid == "" || curhid == "default");
		hiddenFiles->add(_("YES"), "1", curhid == "1");
		hiddenFiles->add(_("NO"), "0", curhid == "0");
		themeconfig->addWithDescription(_("SHOW HIDDEN FILES"), _("DEFAULT VALUE") + " : " + defHid, hiddenFiles);
		themeconfig->addSaveFunc([themeconfig, hiddenFiles, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".ShowHiddenFiles", hiddenFiles->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Folder View Mode
		auto folderView = Settings::getInstance()->getString("FolderViewMode");
		auto defFol = folderView.empty() ? "" : Utils::String::toUpper(_(folderView.c_str()));
		auto curFol = Settings::getInstance()->getString(system->getName() + ".FolderViewMode");

		auto foldersBehavior = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW FOLDERS"), false);
		foldersBehavior->add(_("DEFAULT"), "", curFol == "" || curFol == "default"); //  + " (" + defFol + ")"
		foldersBehavior->add(_("always"), "always", curFol == "always");
		foldersBehavior->add(_("never"), "never", curFol == "never");
		foldersBehavior->add(_("having multiple games"), "having multiple games", curFol == "having multiple games");

		themeconfig->addWithDescription(_("SHOW FOLDERS"), _("DEFAULT VALUE") + " : " + defFol, foldersBehavior);
		themeconfig->addSaveFunc([themeconfig, foldersBehavior, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".FolderViewMode", foldersBehavior->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Show parent folder in gamelists
		auto defPf = Settings::getInstance()->getBool("ShowParentFolder") ? _("YES") : _("NO");
		auto curPf = Settings::getInstance()->getString(system->getName() + ".ShowParentFolder");
		auto parentFolder = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW '..' PARENT FOLDER"), false);
		parentFolder->add(_("DEFAULT"), "", curPf == "" || curPf == "default");
		parentFolder->add(_("YES"), "1", curPf == "1");
		parentFolder->add(_("NO"), "0", curPf == "0");
		themeconfig->addWithDescription(_("SHOW '..' PARENT FOLDER"), _("DEFAULT VALUE") + " : " + defPf, parentFolder);
		themeconfig->addSaveFunc([themeconfig, parentFolder, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".ShowParentFolder", parentFolder->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Show flags

		auto defSF = Settings::getInstance()->getString("ShowFlags");
		if (defSF == "1")
			defSF = _("BEFORE NAME");
		else if (defSF == "2")
			defSF = _("AFTER NAME");
		else
			defSF = _("NO");

		auto curSF = Settings::getInstance()->getString(system->getName() + ".ShowFlags");
		auto showRegionFlags = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW REGION FLAG"), false);

		showRegionFlags->addRange({{_("DEFAULT"), "default"},
								   {_("NO"), "0"},
								   {_("BEFORE NAME"), "1"},
								   {_("AFTER NAME"), "2"}},
								  curSF);

		themeconfig->addWithDescription(_("SHOW REGION FLAG"), _("DEFAULT VALUE") + " : " + defSF, showRegionFlags);
		themeconfig->addSaveFunc([themeconfig, showRegionFlags, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".ShowFlags", showRegionFlags->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Show SaveStates
		auto defSS = Settings::getInstance()->getBool("ShowSaveStates") ? _("YES") : _("NO");
		auto curSS = Settings::getInstance()->getString(system->getName() + ".ShowSaveStates");
		auto showSaveStates = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW SAVESTATE ICON"), false);
		showSaveStates->add(_("DEFAULT"), "", curSS == "" || curSS == "default");
		showSaveStates->add(_("YES"), "1", curSS == "1");
		showSaveStates->add(_("NO"), "0", curSS == "0");
		themeconfig->addWithDescription(_("SHOW SAVESTATE ICON"), _("DEFAULT VALUE") + " : " + defSS, showSaveStates);
		themeconfig->addSaveFunc([themeconfig, showSaveStates, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".ShowSaveStates", showSaveStates->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Show Manual
		auto defMM = Settings::getInstance()->getBool("ShowManualIcon") ? _("YES") : _("NO");
		auto curMM = Settings::getInstance()->getString(system->getName() + ".ShowManualIcon");
		auto showManual = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW MANUAL ICON"), false);
		showManual->add(_("DEFAULT"), "", curMM == "" || curMM == "default");
		showManual->add(_("YES"), "1", curMM == "1");
		showManual->add(_("NO"), "0", curMM == "0");
		themeconfig->addWithDescription(_("SHOW MANUAL ICON"), _("DEFAULT VALUE") + " : " + defMM, showManual);
		themeconfig->addSaveFunc([themeconfig, showManual, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".ShowManualIcon", showManual->getSelected()))
				themeconfig->setVariable("reloadAll", true);
		});

		// Show filenames
		auto defFn = Settings::getInstance()->getBool("ShowFilenames") ? _("YES") : _("NO");
		auto curFn = Settings::getInstance()->getString(system->getName() + ".ShowFilenames");

		auto showFilenames = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW FILENAMES INSTEAD"), false);
		showFilenames->add(_("DEFAULT"), "", curFn == "");
		showFilenames->add(_("YES"), "1", curFn == "1");
		showFilenames->add(_("NO"), "0", curFn == "0");
		themeconfig->addWithDescription(_("SHOW FILENAMES INSTEAD"), _("DEFAULT VALUE") + " : " + defFn, showFilenames);
		themeconfig->addSaveFunc([themeconfig, showFilenames, system]
		{
			if (Settings::getInstance()->setString(system->getName() + ".ShowFilenames", showFilenames->getSelected()))
			{
				SystemData::resetSettings();
				FileData::resetSettings();

		//		themeconfig->setVariable("reloadCollections", true);
				themeconfig->setVariable("reloadAll", true);
			}
		});


		// File extensions
		if (!system->isCollection() && system->isGameSystem())
		{
			auto hiddenExts = Utils::String::split(Settings::getInstance()->getString(system->getName() + ".HiddenExt"), ';');

			auto hiddenCtrl = std::make_shared<OptionListComponent<std::string>>(mWindow, _("FILE EXTENSIONS"), true);

			for (auto ext : system->getExtensions())
			{
				std::string extid = Utils::String::toLower(Utils::String::replace(ext, ".", ""));
				hiddenCtrl->add(ext, extid, std::find(hiddenExts.cbegin(), hiddenExts.cend(), extid) == hiddenExts.cend());
			}

			themeconfig->addWithLabel(_("FILE EXTENSIONS"), hiddenCtrl);
			themeconfig->addSaveFunc([themeconfig, system, hiddenCtrl]
			{
				std::string hiddenSystems;

				std::vector<std::string> sel = hiddenCtrl->getSelectedObjects();

				for (auto ext : system->getExtensions())
				{
					std::string extid = Utils::String::toLower(Utils::String::replace(ext, ".", ""));
					if (std::find(sel.cbegin(), sel.cend(), extid) == sel.cend())
					{
						if (hiddenSystems.empty())
							hiddenSystems = extid;
						else
							hiddenSystems = hiddenSystems + ";" + extid;
					}
				}

				if (Settings::getInstance()->setString(system->getName() + ".HiddenExt", hiddenSystems))
				{
					Settings::getInstance()->saveFile();

					themeconfig->setVariable("reloadAll", true);
					themeconfig->setVariable("forceReloadGames", true);
				}
			});
		}
	}

	if (systemTheme.empty())
	{
		themeconfig->addGroup(_("TOOLS"));

		themeconfig->addEntry(_("RESET CUSTOMIZATIONS"), false, [s, themeconfig, window]
		{
			themeconfig->setVariable("resetTheme", true);
			themeconfig->setVariable("reloadAll", true);
			themeconfig->close();
		});
	}

	//  theme_colorset, theme_iconset, theme_menu, theme_systemview, theme_gamelistview, theme_region,
	themeconfig->addSaveFunc([systemTheme, system, themeconfig, options, gamelist_style, mGridSize, window]
	{
		bool reloadAll = false;

		for (auto option : options)
		{
			ThemeConfigOption& opt = option.second;

			std::string value;

			if (opt.component != nullptr)
			{
				value = opt.component->getSelected();

				if (!systemTheme.empty() && !value.empty())
				{
					std::string defaultValue = Settings::getInstance()->getString(opt.defaultSettingName);
					if (defaultValue.empty())
						defaultValue = system->getTheme()->getDefaultSubSetValue(opt.subset);

					if (value == defaultValue)
						value = "";
				}
				else if (systemTheme.empty() && value == system->getTheme()->getDefaultSubSetValue(opt.subset))
					value = "";
			}

			if (value != Settings::getInstance()->getString(option.first))
				reloadAll |= Settings::getInstance()->setString(option.first, value);
		}

		Vector2f gridSizeOverride(0, 0);

		if (mGridSize != nullptr)
		{
			std::string str = mGridSize->getSelected();
			std::string value = "";

			size_t divider = str.find('x');
			if (divider != std::string::npos)
			{
				std::string first = str.substr(0, divider);
				std::string second = str.substr(divider + 1, std::string::npos);

				gridSizeOverride = Vector2f((float)atof(first.c_str()), (float)atof(second.c_str()));
				value = Utils::String::replace(Utils::String::replace(gridSizeOverride.toString(), ".000000", ""), "0 0", "");
			}

			if (systemTheme.empty())
				reloadAll |= Settings::getInstance()->setString("DefaultGridSize", value);
		}
		else if (systemTheme.empty())
			reloadAll |= Settings::getInstance()->setString("DefaultGridSize", "");

		if (systemTheme.empty())
			reloadAll |= Settings::getInstance()->setString("GamelistViewStyle", gamelist_style == nullptr ? "" : gamelist_style->getSelected());
		else
		{
			std::string viewMode = gamelist_style == nullptr ? system->getSystemViewMode() : gamelist_style->getSelected();
			reloadAll |= system->setSystemViewMode(viewMode, gridSizeOverride);
		}

		if (themeconfig->getVariable("resetTheme"))
		{
			Settings::getInstance()->setString("GamelistViewStyle", "");
			Settings::getInstance()->setString("DefaultGridSize", "");
			Settings::getInstance()->setString("ThemeRegionName", "");
			Settings::getInstance()->setString("ThemeColorSet", "");
			Settings::getInstance()->setString("ThemeIconSet", "");
			Settings::getInstance()->setString("ThemeMenu", "");
			Settings::getInstance()->setString("ThemeSystemView", "");
			Settings::getInstance()->setString("ThemeGamelistView", "");
			Settings::getInstance()->setString("GamelistViewStyle", "");
			Settings::getInstance()->setString("DefaultGridSize", "");

			for (auto sm : Settings::getInstance()->getStringMap())
				if (Utils::String::startsWith(sm.first, "subset."))
					Settings::getInstance()->setString(sm.first, "");

			for (auto system : SystemData::sSystemVector)
			{
				system->setSystemViewMode("automatic", Vector2f(0, 0));

				Settings::getInstance()->setString(system->getName() + ".FavoritesFirst", "");
				Settings::getInstance()->setString(system->getName() + ".ShowHiddenFiles", "");
				Settings::getInstance()->setString(system->getName() + ".FolderViewMode", "");
				Settings::getInstance()->setString(system->getName() + ".ShowFilenames", "");
				Settings::getInstance()->setString(system->getName() + ".ShowParentFolder", "");
			}

			Settings::getInstance()->saveFile();
			std::string path = Utils::FileSystem::getEsConfigPath() + "/themesettings/" + Settings::getInstance()->getString("ThemeSet") + ".cfg";
			if (Utils::FileSystem::exists(path))
				Utils::FileSystem::removeFile(path);
		}

		if (reloadAll || themeconfig->getVariable("reloadAll"))
		{
			if (themeconfig->getVariable("forceReloadGames"))
			{
				ViewController::reloadAllGames(window, false);
			}
			else if (systemTheme.empty())
			{
				CollectionSystemManager::get()->updateSystemsList();
				ViewController::get()->reloadAll(window);
			}
			else
			{
				system->loadTheme();
				system->resetFilters();

				ViewController::get()->reloadSystemListViewTheme(system);
				ViewController::get()->reloadGameListView(system);
			}
		}
	});

	mWindow->pushGui(themeconfig);
}

void GuiMenu::openUISettings()
{
	auto pthis = this;
	Window* window = mWindow;

	auto s = new GuiSettings(mWindow, _("UI SETTINGS").c_str());

	// theme set
	auto theme = ThemeData::getMenuTheme();
	auto themeSets = ThemeData::getThemeSets();
	auto system = ViewController::get()->getState().getSystem();

	s->addGroup(_("APPEARANCE"));

	if (system != nullptr && !themeSets.empty())
	{
		auto selectedSet = themeSets.find(Settings::getInstance()->getString("ThemeSet"));
		if (selectedSet == themeSets.end())
			selectedSet = themeSets.begin();

		auto theme_set = std::make_shared<OptionListComponent<std::string> >(mWindow, _("THEME SET"), false);

		std::vector<std::string> themeList;
		for (auto it = themeSets.begin(); it != themeSets.end(); it++)
			themeList.push_back(it->first);

		std::sort(themeList.begin(), themeList.end(), [](const std::string& a, const std::string& b) -> bool { return Utils::String::toLower(a).compare(Utils::String::toLower(b)) < 0; });

		for (auto themeName : themeList)
			theme_set->add(themeName, themeName, themeName == selectedSet->first);

		//for (auto it = themeSets.begin(); it != themeSets.end(); it++)
		//	theme_set->add(it->first, it->first, it == selectedSet);

		s->addWithLabel(_("THEME SET"), theme_set);
		s->addSaveFunc([s, theme_set, pthis, window, system]
		{
			std::string oldTheme = Settings::getInstance()->getString("ThemeSet");
			if (oldTheme != theme_set->getSelected())
			{
				saveSubsetSettings();

				Settings::getInstance()->setString("ThemeSet", theme_set->getSelected());

				// theme changed without setting options, forcing options to avoid crash/blank theme
				Settings::getInstance()->setString("ThemeRegionName", "");
				Settings::getInstance()->setString("ThemeColorSet", "");
				Settings::getInstance()->setString("ThemeIconSet", "");
				Settings::getInstance()->setString("ThemeMenu", "");
				Settings::getInstance()->setString("ThemeSystemView", "");
				Settings::getInstance()->setString("ThemeGamelistView", "");
				Settings::getInstance()->setString("GamelistViewStyle", "");
				Settings::getInstance()->setString("DefaultGridSize", "");

				for(auto sm : Settings::getInstance()->getStringMap())
					if (Utils::String::startsWith(sm.first, "subset."))
						Settings::getInstance()->setString(sm.first, "");

				for (auto sysIt = SystemData::sSystemVector.cbegin(); sysIt != SystemData::sSystemVector.cend(); sysIt++)
					(*sysIt)->setSystemViewMode("automatic", Vector2f(0,0));

				loadSubsetSettings(theme_set->getSelected());

				s->setVariable("reloadCollections", true);
				s->setVariable("reloadAll", true);
				s->setVariable("reloadGuiMenu", true);

				Scripting::fireEvent("theme-changed", theme_set->getSelected(), oldTheme);
			}
		});

		bool showThemeConfiguration = system->getTheme()->hasSubsets() || system->getTheme()->hasView("grid");
		if (showThemeConfiguration)
		{
			s->addSubMenu(_("THEME CONFIGURATION"), [this, s, theme_set]() { openThemeConfiguration(mWindow, s, theme_set); });
		}
		else // GameList view style only, acts like Retropie for simple themes
		{
			auto gamelist_style = std::make_shared< OptionListComponent<std::string> >(mWindow, _("GAMELIST VIEW STYLE"), false);
			std::vector<std::pair<std::string, std::string>> styles;
			styles.push_back(std::pair<std::string, std::string>("automatic", _("automatic")));

			auto system = ViewController::get()->getState().getSystem();
			if (system != NULL)
			{
				auto mViews = system->getTheme()->getViewsOfTheme();
				for (auto it = mViews.cbegin(); it != mViews.cend(); ++it)
					styles.push_back(*it);
			}
			else
			{
				styles.push_back(std::pair<std::string, std::string>("basic", _("basic")));
				styles.push_back(std::pair<std::string, std::string>("detailed", _("detailed")));
				styles.push_back(std::pair<std::string, std::string>("video", _("video")));
				styles.push_back(std::pair<std::string, std::string>("grid", _("grid")));
			}

			auto viewPreference = Settings::getInstance()->getString("GamelistViewStyle");
			if (!system->getTheme()->hasView(viewPreference))
				viewPreference = "automatic";

			for (auto it = styles.cbegin(); it != styles.cend(); it++)
				gamelist_style->add(it->second, it->first, viewPreference == it->first);

			s->addWithLabel(_("GAMELIST VIEW STYLE"), gamelist_style);
			s->addSaveFunc([s, gamelist_style, window] {
				if (Settings::getInstance()->setString("GamelistViewStyle", gamelist_style->getSelected()))
				{
					s->setVariable("reloadAll", true);
					s->setVariable("reloadGuiMenu", true);
				}
			});
		}
	}

	// language choice
	auto language_choice = std::make_shared<OptionListComponent<std::string> >(window, _("LANGUAGE"), false);

	std::string language = SystemConf::getInstance()->get("system.language");
	if (language.empty())
		language = "en_US";

	language_choice->add("ARABIC",               "ar_YE", language == "ar_YE");
	language_choice->add("CATALÀ",               "ca_ES", language == "ca_ES");
	language_choice->add("CYMRAEG",              "cy_GB", language == "cy_GB");
	language_choice->add("DEUTSCH", 	     "de_DE", language == "de_DE");
	language_choice->add("GREEK",                "el_GR", language == "el_GR");
	language_choice->add("ENGLISH", 	     "en_US", language == "en_US" || language == "en");
	language_choice->add("ESPAÑOL", 	     "es_ES", language == "es_ES" || language == "es");
	language_choice->add("ESPAÑOL MEXICANO",     "es_MX", language == "es_MX");
	language_choice->add("BASQUE",               "eu_ES", language == "eu_ES");
	language_choice->add("FRANÇAIS",             "fr_FR", language == "fr_FR" || language == "fr");
	language_choice->add("עברית",                "he_IL", language == "he_IL");
	language_choice->add("HUNGARIAN",            "hu_HU", language == "hu_HU");
	language_choice->add("ITALIANO",             "it_IT", language == "it_IT");
	language_choice->add("JAPANESE", 	     "ja_JP", language == "ja_JP");
	language_choice->add("KOREAN",   	     "ko_KR", language == "ko_KR" || language == "ko");
	language_choice->add("NORWEGIAN BOKMAL",     "nb_NO", language == "nb_NO");
	language_choice->add("DUTCH",                "nl_NL", language == "nl_NL");
	language_choice->add("NORWEGIAN",            "nn_NO", language == "nn_NO");
	language_choice->add("OCCITAN",              "oc_FR", language == "oc_FR");
	language_choice->add("POLISH",               "pl_PL", language == "pl_PL");
	language_choice->add("PORTUGUES BRASILEIRO", "pt_BR", language == "pt_BR");
	language_choice->add("PORTUGUES PORTUGAL",   "pt_PT", language == "pt_PT");
	language_choice->add("РУССКИЙ",              "ru_RU", language == "ru_RU");
	language_choice->add("SVENSKA", 	     "sv_SE", language == "sv_SE");
	language_choice->add("TÜRKÇE",  	     "tr_TR", language == "tr_TR");
	language_choice->add("Українська",           "uk_UA", language == "uk_UA");
	language_choice->add("简体中文", 	     "zh_CN", language == "zh_CN");
	language_choice->add("正體中文", 	     "zh_TW", language == "zh_TW");
	s->addWithLabel(_("LANGUAGE"), language_choice);

	s->addSaveFunc([window, language_choice, language, s]
	{
		bool reboot = false;

		if (language_choice->changed())
		{
			std::string selectedLanguage = language_choice->getSelected();
			std::string msg = _("You are about to set your language to:") +"\n" +  selectedLanguage + "\n";
			msg += _("Emulationstation will restart")+"\n";
			msg += _("Do you want to proceed ?");
			window->pushGui(new GuiMsgBox(window, msg, _("YES"), [selectedLanguage] {
				SystemConf::getInstance()->set("system.language", selectedLanguage);
				SystemConf::getInstance()->saveSystemConf();
				quitES(QuitMode::QUIT);
			}, "NO",nullptr));
#ifdef HAVE_INTL
				reboot = true;
#endif
		}

		if (reboot)
			window->displayNotificationMessage(_U("\uF011  ") + _("A REBOOT OF THE SYSTEM IS REQUIRED TO APPLY THE NEW CONFIGURATION"));

	});

	// UI RESTRICTIONS
	auto UImodeSelection = std::make_shared< OptionListComponent<std::string> >(mWindow, _("UI MODE"), false);
	std::vector<std::string> UImodes = UIModeController::getInstance()->getUIModes();
	for (auto it = UImodes.cbegin(); it != UImodes.cend(); it++)
		UImodeSelection->add(_(it->c_str()), *it, Settings::getInstance()->getString("UIMode") == *it);

	s->addWithLabel(_("UI MODE"), UImodeSelection);
	s->addSaveFunc([UImodeSelection, window]
	{
		std::string selectedMode = UImodeSelection->getSelected();
		if (selectedMode != "Full")
		{
			std::string msg = _("You are changing the UI to a restricted mode:\nThis will hide most menu-options to prevent changes to the system.\nTo unlock and return to the full UI, enter this code:") + "\n";
			msg += "\"" + UIModeController::getInstance()->getFormattedPassKeyStr() + "\"\n\n";
			msg += _("Do you want to proceed ?");
			window->pushGui(new GuiMsgBox(window, msg,
				_("YES"), [selectedMode] {
				LOG(LogDebug) << "Setting UI mode to " << selectedMode;
				Settings::getInstance()->setString("UIMode", selectedMode);
				Settings::getInstance()->saveFile();
			}, _("NO"), nullptr));
		}
	});

	// retroarch.menu_driver choose from 'auto' (default), 'xmb', 'rgui', 'ozone', 'glui'
	auto retroarchRgui = std::make_shared< OptionListComponent<std::string> >(mWindow, _("RETROARCH MENU DRIVER"), false);
	std::vector<std::string> driver;
	driver.push_back("default");
	driver.push_back("xmb");
	driver.push_back("rgui");
	driver.push_back("ozone");
	driver.push_back("glui");

	auto currentDriver = SystemConf::getInstance()->get("global.retroarch.menu_driver");
	if (currentDriver.empty())
		currentDriver = "default";

	for (auto it = driver.cbegin(); it != driver.cend(); it++)
		retroarchRgui->add(_(it->c_str()), *it, currentDriver == *it);

	s->addWithLabel(_("RETROARCH MENU DRIVER"), retroarchRgui);
	s->addSaveFunc([retroarchRgui]
	{
		SystemConf::getInstance()->set("global.retroarch.menu_driver", retroarchRgui->getSelected());
		SystemConf::getInstance()->saveSystemConf();
	});

        auto invertJoy = std::make_shared<SwitchComponent>(mWindow);
        invertJoy->setState(Settings::getInstance()->getBool("InvertButtons"));
        s->addWithLabel(_("SWITCH A & B BUTTONS IN EMULATIONSTATION"), invertJoy);
        s->addSaveFunc([this, s, invertJoy]
        {
                if (Settings::getInstance()->setBool("InvertButtons", invertJoy->getState()))
                {
			std::string trueFalse = "false";
			if ( invertJoy->getState() == true ) {
				trueFalse = "true";
			}
			Settings::getInstance()->setString("subset.swap-a-b", trueFalse);
			SystemConf::getInstance()->saveSystemConf();
                        InputConfig::AssignActionButtons();
                        s->setVariable("reloadAll", true);
                }
        });

	auto fps_enabled = std::make_shared<SwitchComponent>(mWindow);
	bool fpsEnabled = SystemConf::getInstance()->get("global.showFPS") == "1";
	fps_enabled->setState(fpsEnabled);
	s->addWithLabel(_("SHOW RETROARCH FPS"), fps_enabled);
	s->addSaveFunc([fps_enabled] {
		bool fpsenabled = fps_enabled->getState();
	SystemConf::getInstance()->set("global.showFPS", fpsenabled ? "1" : "0");
			SystemConf::getInstance()->saveSystemConf();
		});

	auto desktop_enabled = std::make_shared<SwitchComponent>(mWindow);
	bool desktopEnabled = SystemConf::getInstance()->get("desktop.enabled") == "1";
	desktop_enabled->setState(desktopEnabled);
	s->addWithLabel(_("DESKTOP MODE"), desktop_enabled);
	s->addSaveFunc([this,desktop_enabled] {
		if (desktop_enabled->changed()) {
	                std::string msg = _("The system will restart")+"\n";
	                msg += _("Do you want to continue?");
			mWindow->pushGui(new GuiMsgBox(mWindow,msg, _("YES"), 
				[this,desktop_enabled] {
					bool desktopenabled = desktop_enabled->getState();
					SystemConf::getInstance()->set("desktop.enabled", desktopenabled ? "1" : "0");
					SystemConf::getInstance()->saveSystemConf();
					quitES(QuitMode::REBOOT);
			}, "NO",nullptr));
		}
	});


	s->addGroup(_("DISPLAY OPTIONS"));

	s->addEntry(_("SCREENSAVER SETTINGS"), true, std::bind(&GuiMenu::openScreensaverOptions, this));

	// transition style
	auto transition_style = std::make_shared<OptionListComponent<std::string>>(mWindow, _("LIST TRANSITION STYLE"), false);
	transition_style->addRange({"default", "fade", "slide", "fade & slide", "instant"}, Settings::TransitionStyle());
	s->addWithLabel(_("LIST TRANSITION STYLE"), transition_style);
	s->addSaveFunc([transition_style] { Settings::setTransitionStyle(transition_style->getSelected()); });

	// game transition style
	auto transitionOfGames_style = std::make_shared<OptionListComponent<std::string>>(mWindow, _("GAME LAUNCH TRANSITION"), false);
	transitionOfGames_style->addRange({"default", "fade", "slide", "instant"}, Settings::GameTransitionStyle());
	s->addWithLabel(_("GAME LAUNCH TRANSITION"), transitionOfGames_style);
	s->addSaveFunc([transitionOfGames_style] { Settings::setGameTransitionStyle(transitionOfGames_style->getSelected()); });

	// clock
	auto clock = std::make_shared<SwitchComponent>(mWindow);
	clock->setState(Settings::getInstance()->getBool("DrawClock"));
	s->addWithLabel(_("SHOW CLOCK"), clock);
	s->addSaveFunc(
		[clock] { Settings::getInstance()->setBool("DrawClock", clock->getState()); });

	// show help
	auto show_help = std::make_shared<SwitchComponent>(mWindow);
	show_help->setState(Settings::getInstance()->getBool("ShowHelpPrompts"));
	s->addWithLabel(_("ON-SCREEN HELP"), show_help);
	s->addSaveFunc([s, show_help]
	{
		if (Settings::getInstance()->setBool("ShowHelpPrompts", show_help->getState()))
			s->setVariable("reloadAll", true);
	});

	// Battery indicator
	if (queryBatteryInformation().hasBattery)
	{
		auto batteryStatus = std::make_shared<OptionListComponent<std::string> >(mWindow, _("SHOW BATTERY STATUS"), false);
		batteryStatus->addRange({ { _("NO"), "" },{ _("ICON"), "icon" },{ _("ICON AND TEXT"), "text" } }, Settings::getInstance()->getString("ShowBattery"));
		s->addWithLabel(_("SHOW BATTERY STATUS"), batteryStatus);
		s->addSaveFunc([batteryStatus] { Settings::getInstance()->setString("ShowBattery", batteryStatus->getSelected()); });
	}

	s->addGroup(_("GAMELIST OPTIONS"));

	// Enable Video Previews
	auto enable_preview = std::make_shared<SwitchComponent>(mWindow);
	enable_preview->setState(Settings::getInstance()->getBool("EnableVideoPreviews"));
	s->addWithLabel(_("SHOW VIDEO PREVIEWS"), enable_preview);
	s->addSaveFunc([enable_preview] { Settings::getInstance()->setBool("EnableVideoPreviews", enable_preview->getState()); });

	// Show favorites first in gamelists
	auto favoritesFirstSwitch = std::make_shared<SwitchComponent>(mWindow);
	favoritesFirstSwitch->setState(Settings::getInstance()->getBool("FavoritesFirst"));
	s->addWithLabel(_("SHOW FAVORITES ON TOP"), favoritesFirstSwitch);
	s->addSaveFunc([s, favoritesFirstSwitch]
	{
		if (Settings::getInstance()->setBool("FavoritesFirst", favoritesFirstSwitch->getState()))
			s->setVariable("reloadAll", true);
	});

	// hidden files
	auto hidden_files = std::make_shared<SwitchComponent>(mWindow);
	hidden_files->setState(Settings::ShowHiddenFiles());
	s->addWithLabel(_("SHOW HIDDEN FILES"), hidden_files);
	s->addSaveFunc([s, hidden_files]
	{
		if (Settings::setShowHiddenFiles(hidden_files->getState()))
			s->setVariable("reloadAll", true);
	});

	// Folder View Mode
	auto foldersBehavior = std::make_shared< OptionListComponent<std::string> >(mWindow, _("SHOW FOLDERS"), false);

	foldersBehavior->add(_("always"), "always", Settings::getInstance()->getString("FolderViewMode") == "always");
	foldersBehavior->add(_("never"), "never", Settings::getInstance()->getString("FolderViewMode") == "never");
	foldersBehavior->add(_("having multiple games"), "having multiple games", Settings::getInstance()->getString("FolderViewMode") == "having multiple games");

	s->addWithLabel(_("SHOW FOLDERS"), foldersBehavior);
	s->addSaveFunc([s, foldersBehavior]
	{
		if (Settings::getInstance()->setString("FolderViewMode", foldersBehavior->getSelected()))
			s->setVariable("reloadAll", true);
	});

	// Show parent folder
	auto parentFolder = std::make_shared<SwitchComponent>(mWindow);
	parentFolder->setState(Settings::getInstance()->getBool("ShowParentFolder"));
	s->addWithLabel(_("SHOW '..' PARENT FOLDER"), parentFolder);
	s->addSaveFunc([s, parentFolder]
	{
		if (Settings::getInstance()->setBool("ShowParentFolder", parentFolder->getState()))
			s->setVariable("reloadAll", true);
	});

	// Show flags
	auto showRegionFlags = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SHOW REGION FLAG"), false);
	showRegionFlags->addRange({ { _("NO"), "default" },{ _("BEFORE NAME") , "1" },{ _("AFTER NAME"), "2" } }, Settings::getInstance()->getString("ShowFlags"));
	s->addWithLabel(_("SHOW REGION FLAG"), showRegionFlags);
	s->addSaveFunc([s, showRegionFlags]
	{
		if (Settings::getInstance()->setString("ShowFlags", showRegionFlags->getSelected()))
			s->setVariable("reloadAll", true);
	});

	// Show SaveStates
	auto showSaveStates = std::make_shared<SwitchComponent>(mWindow);
	showSaveStates->setState(Settings::getInstance()->getBool("ShowSaveStates"));
	s->addWithLabel(_("SHOW SAVESTATE ICON"), showSaveStates);
	s->addSaveFunc([s, showSaveStates]
	{
		if (Settings::getInstance()->setBool("ShowSaveStates", showSaveStates->getState()))
			s->setVariable("reloadAll", true);
	});

	// Show Manual
	auto showManual = std::make_shared<SwitchComponent>(mWindow);
	showManual->setState(Settings::getInstance()->getBool("ShowManualIcon"));
	s->addWithLabel(_("SHOW MANUAL ICON"), showManual);
	s->addSaveFunc([s, showManual]
	{
		if (Settings::getInstance()->setBool("ShowManualIcon", showManual->getState()))
			s->setVariable("reloadAll", true);
	});

	// filenames
	auto showFilesnames = std::make_shared<SwitchComponent>(mWindow);
	showFilesnames->setState(Settings::getInstance()->getBool("ShowFilenames"));
	s->addWithLabel(_("SHOW FILENAMES INSTEAD"), showFilesnames);
	s->addSaveFunc([showFilesnames, s]
	{
		if (Settings::getInstance()->setBool("ShowFilenames", showFilesnames->getState()))
		{
			SystemData::resetSettings();
			FileData::resetSettings();

			s->setVariable("reloadCollections", true);
			s->setVariable("reloadAll", true);
		}
	});

	auto ignoreArticles = std::make_shared<SwitchComponent>(mWindow);
	ignoreArticles->setState(Settings::getInstance()->getBool("IgnoreLeadingArticles"));
	s->addWithLabel(_("IGNORE LEADING ARTICLES WHEN SORTING"), ignoreArticles);
	s->addSaveFunc([s, ignoreArticles]
	{
		if (Settings::getInstance()->setBool("IgnoreLeadingArticles", ignoreArticles->getState()))
		{
			s->setVariable("reloadAll", true);
		}
	});

	s->onFinalize([s, pthis, window]
	{
		if (s->getVariable("reloadCollections"))
			CollectionSystemManager::get()->updateSystemsList();

		if (s->getVariable("reloadAll"))
		{
			ViewController::get()->reloadAll(window);
		}

		if (s->getVariable("reloadGuiMenu"))
		{
			delete pthis;
			window->pushGui(new GuiMenu(window));
		}
	});

	mWindow->pushGui(s);
}

void GuiMenu::openSoundSettings()
{
	auto s = new GuiSettings(mWindow, _("SOUND SETTINGS").c_str());

	if (GetEnv("DEVICE_SW_HP_SWITCH") == "true") {
		s->addGroup(_("OUTPUT"));

		// sw headphone enable
		auto sw_hp_enabled = std::make_shared<SwitchComponent>(mWindow);
		bool hpbaseEnabled = SystemConf::getInstance()->get("headphone.enabled") == "1";
		sw_hp_enabled->setState(hpbaseEnabled);
		s->addWithLabel(_("ENABLE HEADPHONE JACK"), sw_hp_enabled);
		s->addSaveFunc([sw_hp_enabled]
			{
				if (sw_hp_enabled->getState() == false) {
					runSystemCommand("amixer -c0 sset \"Playback Mux\" \"SPK\"", "", nullptr);
				} else {
					runSystemCommand("amixer -c0 sset \"Playback Mux\" \"HP\"", "", nullptr);
				}
				bool swhpenabled = sw_hp_enabled->getState();
				SystemConf::getInstance()->set("headphone.enabled", swhpenabled ? "1" : "0");
				SystemConf::getInstance()->saveSystemConf();
			});
	}


	if (VolumeControl::getInstance()->isAvailable())
	{
		s->addGroup(_("VOLUME"));

		// volume
		auto volume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 10.f, "%");
		volume->setValue((float)VolumeControl::getInstance()->getVolume());
		volume->setOnValueChanged([](const float &newVal) { VolumeControl::getInstance()->setVolume((int)Math::round(newVal)); });
		s->addWithLabel(_("SYSTEM VOLUME"), volume);
		s->addSaveFunc([this, volume]
		{
			VolumeControl::getInstance()->setVolume((int)Math::round(volume->getValue()));
			SystemConf::getInstance()->set("audio.volume", std::to_string((int)round(volume->getValue())));
		});

		// Music Volume
		auto musicVolume = std::make_shared<SliderComponent>(mWindow, 0.f, 100.f, 1.f, "%");
		musicVolume->setValue(Settings::getInstance()->getInt("MusicVolume"));
		musicVolume->setOnValueChanged([](const float &newVal) { Settings::getInstance()->setInt("MusicVolume", (int)round(newVal)); });
		s->addWithLabel(_("MUSIC VOLUME"), musicVolume);
		//s->addSaveFunc([this, musicVolume] { Settings::getInstance()->setInt("MusicVolume", (int)round(musicVolume->getValue())); });

		auto volumePopup = std::make_shared<SwitchComponent>(mWindow);
		volumePopup->setState(Settings::getInstance()->getBool("VolumePopup"));
		s->addWithLabel(_("SHOW OVERLAY WHEN VOLUME CHANGES"), volumePopup);
		s->addSaveFunc([volumePopup] { Settings::getInstance()->setBool("VolumePopup", volumePopup->getState()); });
	}

	s->addGroup(_("MUSIC"));

	// disable sounds
	auto music_enabled = std::make_shared<SwitchComponent>(mWindow);
	music_enabled->setState(Settings::getInstance()->getBool("audio.bgmusic"));
	s->addWithLabel(_("FRONTEND MUSIC"), music_enabled);
	s->addSaveFunc([music_enabled]
	{
		if (Settings::getInstance()->setBool("audio.bgmusic", music_enabled->getState()))
		{
			if (music_enabled->getState())
				AudioManager::getInstance()->playRandomMusic();
			else
				AudioManager::getInstance()->stopMusic();
		}
	});

	auto display_titles = std::make_shared<SwitchComponent>(mWindow);
	display_titles->setState(Settings::getInstance()->getBool("audio.display_titles"));
	s->addWithLabel(_("DISPLAY SONG TITLES"), display_titles);
	s->addSaveFunc([display_titles] {
		Settings::getInstance()->setBool("audio.display_titles", display_titles->getState());
	});

	auto titles_time = std::make_shared<SliderComponent>(mWindow, 2.f, 120.f, 2.f, "s");
	titles_time->setValue(Settings::getInstance()->getInt("audio.display_titles_time"));
	s->addWithLabel(_("SONG TITLE DISPLAY DURATION"), titles_time);
	s->addSaveFunc([titles_time] {
		Settings::getInstance()->setInt("audio.display_titles_time", (int)Math::round(titles_time->getValue()));
	});

	auto music_per_system = std::make_shared<SwitchComponent>(mWindow);
	music_per_system->setState(Settings::getInstance()->getBool("audio.persystem"));
	s->addWithLabel(_("ONLY PLAY SYSTEM-SPECIFIC MUSIC FOLDER"), music_per_system);
	s->addSaveFunc([music_per_system] {
		if (Settings::getInstance()->setBool("audio.persystem", music_per_system->getState()))
			AudioManager::getInstance()->changePlaylist(ViewController::get()->getState().getSystem()->getTheme(), true);
	});

	auto enableThemeMusics = std::make_shared<SwitchComponent>(mWindow);
	enableThemeMusics->setState(Settings::getInstance()->getBool("audio.thememusics"));
	s->addWithLabel(_("PLAY SYSTEM-SPECIFIC MUSIC"), enableThemeMusics);
	s->addSaveFunc([enableThemeMusics] {
		if (Settings::getInstance()->setBool("audio.thememusics", enableThemeMusics->getState()))
			AudioManager::getInstance()->changePlaylist(ViewController::get()->getState().getSystem()->getTheme(), true);
	});

	auto videolowermusic = std::make_shared<SwitchComponent>(mWindow);
	videolowermusic->setState(Settings::getInstance()->getBool("VideoLowersMusic"));
	s->addWithLabel(_("LOWER MUSIC WHEN PLAYING VIDEO"), videolowermusic);
	s->addSaveFunc([videolowermusic] { Settings::getInstance()->setBool("VideoLowersMusic", videolowermusic->getState()); });

	s->addGroup(_("SOUNDS"));

	// disable sounds
	auto sounds_enabled = std::make_shared<SwitchComponent>(mWindow);
	sounds_enabled->setState(Settings::getInstance()->getBool("EnableSounds"));
	s->addWithLabel(_("ENABLE NAVIGATION SOUNDS"), sounds_enabled);
	s->addSaveFunc([sounds_enabled]
	{
	    if (sounds_enabled->getState() && !Settings::getInstance()->getBool("EnableSounds") && PowerSaver::getMode() == PowerSaver::INSTANT)
		{
			Settings::getInstance()->setPowerSaverMode("default");
			PowerSaver::init();
		}
	    Settings::getInstance()->setBool("EnableSounds", sounds_enabled->getState());
	  });

        auto batteryWarning = std::make_shared<SwitchComponent>(mWindow);
        bool batteryWarningEnabled = SystemConf::getInstance()->get("system.battery.warning") == "1";
        batteryWarning->setState(batteryWarningEnabled);
        s->addWithLabel(_("ENABLE AUDIBLE BATTERY WARNING"), batteryWarning);
        s->addSaveFunc([batteryWarning] {
                bool batteryWarningEnabled = batteryWarning->getState();
                SystemConf::getInstance()->set("system.battery.warning", batteryWarningEnabled ? "1" : "0");
                SystemConf::getInstance()->saveSystemConf();
        });

	auto video_audio = std::make_shared<SwitchComponent>(mWindow);
	video_audio->setState(Settings::getInstance()->getBool("VideoAudio"));
	s->addWithLabel(_("ENABLE VIDEO PREVIEW AUDIO"), video_audio);
	s->addSaveFunc([video_audio] { Settings::getInstance()->setBool("VideoAudio", video_audio->getState()); });



	mWindow->pushGui(s);
}

void GuiMenu::openWifiSettings(Window* win, std::string title, std::string data, const std::function<void(std::string)>& onsave)
{
	win->pushGui(new GuiWifi(win, title, data, onsave));
}

void GuiMenu::openNetworkSettings_batocera(bool selectWifiEnable, bool selectAdhocEnable)
{
	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	Window *window = mWindow;

	auto s = new GuiSettings(mWindow, _("NETWORK SETTINGS").c_str());
	s->addGroup(_("INFORMATION"));

	auto ip = std::make_shared<TextComponent>(mWindow, ApiSystem::getInstance()->getIpAdress(), font, color);
	s->addWithLabel(_("IP ADDRESS"), ip);

	auto status = std::make_shared<TextComponent>(mWindow, ApiSystem::getInstance()->ping() ? _("CONNECTED") : _("NOT CONNECTED"), font, color);
	s->addWithLabel(_("INTERNET STATUS"), status);

	// Network Indicator
	auto networkIndicator = std::make_shared<SwitchComponent>(mWindow);
	networkIndicator->setState(Settings::getInstance()->getBool("ShowNetworkIndicator"));
	s->addWithLabel(_("SHOW NETWORK INDICATOR"), networkIndicator);
	networkIndicator->setOnChangedCallback([networkIndicator] { Settings::getInstance()->setBool("ShowNetworkIndicator", networkIndicator->getState()); });

	s->addGroup(_("NETWORK CONFIGURATION"));

	// Hostname
	s->addInputTextRow(_("HOSTNAME"), "system.hostname", false);

        // Wifi enable
        auto enable_net = std::make_shared<SwitchComponent>(mWindow);
        bool networkEnabled = SystemConf::getInstance()->getBool("network.enabled");
        enable_net->setState(networkEnabled);

	// Define enable_adhoc early so it is available to addSaveFunc.
	auto enable_adhoc = std::make_shared<SwitchComponent>(mWindow);

        s->addWithLabel(_("ENABLE NETWORK"), enable_net, selectWifiEnable);

	const std::string wifiSSID = SystemConf::getInstance()->get("wifi.ssid");
	const std::string wifiKEY = SystemConf::getInstance()->get("wifi.key");

        s->addInputTextRow(_("WIFI SSID"), "wifi.ssid", false, false, &openWifiSettings);
	s->addSaveFunc([this, enable_net, enable_adhoc, wifiSSID] {
		SystemConf::getInstance()->saveSystemConf();
		const std::string wifissid = SystemConf::getInstance()->get("wifi.ssid");
                if (enable_net->getState() == true && enable_adhoc->getState() == false && wifiSSID != wifissid)
                {
                        std::string wifikey = SystemConf::getInstance()->get("wifi.key");
			ApiSystem::getInstance()->disableWifi();
                        ApiSystem::getInstance()->enableWifi(wifissid, wifikey);
                }
	});

        s->addInputTextRow(_("WIFI KEY"), "wifi.key", true);
        s->addSaveFunc([this, enable_net, enable_adhoc, wifiKEY] {
                SystemConf::getInstance()->saveSystemConf();
                const std::string wifikey = SystemConf::getInstance()->get("wifi.key");
                if (enable_net->getState() == true && enable_adhoc->getState() == false && wifiKEY != wifikey)
                {
                        std::string wifissid = SystemConf::getInstance()->get("wifi.ssid");
			ApiSystem::getInstance()->disableWifi();
                        ApiSystem::getInstance()->enableWifi(wifissid, wifikey);
                }
        });

	auto optionsAdhocID = std::make_shared<OptionListComponent<std::string> >(mWindow, _("LOCAL PLAY ID"), false);
	std::string selectedAdhocID = SystemConf::getInstance()->get("wifi.adhoc.id");

        auto optionsChannels = std::make_shared<OptionListComponent<std::string> >(mWindow, _("LOCAL NETWORK CHANNEL"), false);

        std::vector<std::string> availableChannels = ApiSystem::getInstance()->getAvailableChannels();
        std::string selectedChannel = SystemConf::getInstance()->get("wifi.adhoc.channel");

        // Enable or disable ipv6
        auto ipv6_enable = std::make_shared<SwitchComponent>(mWindow);
        bool ipv6Enabled = SystemConf::getInstance()->get("ipv6.enabled") == "1";
        ipv6_enable->setState(ipv6Enabled);
        s->addWithLabel(_("ENABLE IPV6"), ipv6_enable);
        ipv6_enable->setOnChangedCallback([ipv6_enable] {
                bool ipv6Enabled = ipv6_enable->getState();
                SystemConf::getInstance()->set("ipv6.enabled", ipv6Enabled ? "1" : "0");
                SystemConf::getInstance()->saveSystemConf();
                runSystemCommand("/usr/bin/toggle-ipv6", "", nullptr);
        });

	s->addGroup(_("LOCAL NETPLAY SETTINGS"));

	// Adhoc mode options
	bool adhocEnabled = SystemConf::getInstance()->getBool("network.adhoc.enabled");
	enable_adhoc->setState(adhocEnabled);
	s->addWithLabel(_("LOCAL PLAY MODE"), enable_adhoc, selectAdhocEnable);

	if (selectedAdhocID.empty())
	{
		selectedAdhocID = "1";
	}
	optionsAdhocID->add(_("1 (HOST)"),"1", selectedAdhocID == "1");
	optionsAdhocID->add(_("2 (CLIENT 1)"),"2", selectedAdhocID == "2");
	optionsAdhocID->add(_("3 (CLIENT 2)"),"3", selectedAdhocID == "3");
	optionsAdhocID->add(_("4 (CLIENT 3)"),"4", selectedAdhocID == "4");
	s->addWithLabel(_("LOCAL PLAY ID"), optionsAdhocID);

       if (selectedChannel.empty())
                selectedChannel = "6";

        bool wfound = false;
        for (auto it = availableChannels.begin(); it != availableChannels.end(); it++)
        {
		optionsChannels->add((*it), (*it), selectedChannel == (*it));
		if (selectedChannel == (*it))
			wfound = true;
        }

        if (!wfound)
                optionsChannels->add(selectedChannel, selectedChannel, true);

        s->addWithLabel(_("LOCAL NETWORK CHANNEL"), optionsChannels);

	enable_adhoc->setOnChangedCallback([adhocEnabled, networkEnabled, enable_net, enable_adhoc, optionsAdhocID, selectedAdhocID, optionsChannels, selectedChannel, window]
	{
		SystemConf::getInstance()->set("wifi.adhoc.id", optionsAdhocID->getSelected());
		SystemConf::getInstance()->set("wifi.adhoc.channel", optionsChannels->getSelected());

		std::string newSSID = SystemConf::getInstance()->get("wifi.ssid");
		std::string newKey = SystemConf::getInstance()->get("wifi.key");

		SystemConf::getInstance()->set("global.netplay.host", "192.168.80.1");
		SystemConf::getInstance()->set("global.netplay.port", "55435");
		SystemConf::getInstance()->set("global.netplay.relay", "none");

		bool adhocenabled = enable_adhoc->getState();
		SystemConf::getInstance()->setBool("network.adhoc.enabled", adhocenabled);
		SystemConf::getInstance()->saveSystemConf();

		if (enable_net->getState() == true)
		{
			ApiSystem::getInstance()->disableWifi();
			ApiSystem::getInstance()->enableWifi(newSSID, newKey);
		}
	});

        enable_net->setOnChangedCallback([enable_net, enable_adhoc, networkEnabled, adhocEnabled, window]
        {
                if (enable_net->getState() == true && enable_adhoc->getState() == false)
                {
                        std::string newSSID = SystemConf::getInstance()->get("wifi.ssid");
                        std::string newKey = SystemConf::getInstance()->get("wifi.key");
                        ApiSystem::getInstance()->enableWifi(newSSID, newKey);
                }
                else
                {
                        ApiSystem::getInstance()->disableWifi();
                }
		bool networkenabled = enable_net->getState();
		SystemConf::getInstance()->setBool("network.enabled", networkenabled);
                SystemConf::getInstance()->saveSystemConf();
        });

	s->addGroup(_("NETWORK SERVICES"));

       auto sshd_enabled = std::make_shared<SwitchComponent>(mWindow);
                bool sshbaseEnabled = SystemConf::getInstance()->get("ssh.enabled") == "1";
                sshd_enabled->setState(sshbaseEnabled);
                s->addWithLabel(_("ENABLE SSH"), sshd_enabled);
                sshd_enabled->setOnChangedCallback([sshd_enabled] {
                        if (sshd_enabled->getState() == false) {
                                runSystemCommand("systemctl stop sshd", "", nullptr);
                                runSystemCommand("systemctl disable sshd", "", nullptr);
                                runSystemCommand("rm /storage/.cache/services/sshd.conf", "", nullptr);
                        } else {
                                runSystemCommand("mkdir -p /storage/.cache/services/", "", nullptr);
                                runSystemCommand("touch /storage/.cache/services/sshd.conf", "", nullptr);
                                runSystemCommand("systemctl enable sshd", "", nullptr);
                                runSystemCommand("systemctl start sshd", "", nullptr);
                        }
			bool sshenabled = sshd_enabled->getState();
			SystemConf::getInstance()->set("ssh.enabled", sshenabled ? "1" : "0");
			SystemConf::getInstance()->saveSystemConf();
                });

       auto samba_enabled = std::make_shared<SwitchComponent>(mWindow);
                bool smbbaseEnabled = SystemConf::getInstance()->get("samba.enabled") == "1";
                samba_enabled->setState(smbbaseEnabled);
                s->addWithLabel(_("ENABLE SAMBA"), samba_enabled);
                samba_enabled->setOnChangedCallback([samba_enabled] {
                        if (samba_enabled->getState() == false) {
                                runSystemCommand("systemctl stop nmbd", "", nullptr);
                                runSystemCommand("systemctl stop smbd", "", nullptr);
                                runSystemCommand("rm /storage/.cache/services/smb.conf", "", nullptr);
                        } else {
                                runSystemCommand("mkdir -p /storage/.cache/services/", "", nullptr);
                                runSystemCommand("touch /storage/.cache/services/smb.conf", "", nullptr);
                                runSystemCommand("systemctl start nmbd", "", nullptr);
                                runSystemCommand("systemctl start smbd", "", nullptr);
                        }
                bool sambaenabled = samba_enabled->getState();
                SystemConf::getInstance()->set("samba.enabled", sambaenabled ? "1" : "0");
                                SystemConf::getInstance()->saveSystemConf();
                });

     auto simple_http_enabled = std::make_shared<SwitchComponent>(mWindow);
                bool simplehttpEnabled = SystemConf::getInstance()->get("simplehttp.enabled") == "1";
                simple_http_enabled->setState(simplehttpEnabled);
                s->addWithLabel(_("ENABLE SIMPLE HTTP SERVER"), simple_http_enabled);
                simple_http_enabled->setOnChangedCallback([simple_http_enabled] {
                        if(simple_http_enabled->getState() == false) {
                                runSystemCommand("systemctl disable --now simple-http-server", "", nullptr);
                        } else {
                                runSystemCommand("systemctl enable --now simple-http-server", "", nullptr);
                        }
                bool simplehttpenabled = simple_http_enabled->getState();
                SystemConf::getInstance()->set("simplehttp.enabled", simplehttpenabled ? "1" : "0");
                                SystemConf::getInstance()->saveSystemConf();
                });

       auto optionsUSBGadget = std::make_shared<OptionListComponent<std::string> >(mWindow, _("USB GADGET FUNCTION"), false);
                std::string selectedUSBGadget = SystemConf::getInstance()->get("usbgadget.function");
                        if (selectedUSBGadget.empty())
                                selectedUSBGadget = "disabled";

                optionsUSBGadget->add(_("DISABLED"), "disabled", selectedUSBGadget == "disabled");
                optionsUSBGadget->add(_("MTP"), "mtp", selectedUSBGadget == "mtp");
                optionsUSBGadget->add(_("ECM"), "ecm", selectedUSBGadget == "ecm");

                s->addWithLabel(_("USB GADGET FUNCTION"), optionsUSBGadget);

                s->addSaveFunc([this, optionsUSBGadget, selectedUSBGadget]
                {
                        if (optionsUSBGadget->changed()) {
                                SystemConf::getInstance()->set("usbgadget.function", optionsUSBGadget->getSelected());
				SystemConf::getInstance()->saveSystemConf();
                                runSystemCommand("/usr/bin/usbgadget stop", "", nullptr);
                                        if (optionsUSBGadget->getSelected() == "mtp")
                                                runSystemCommand("/usr/bin/usbgadget start mtp", "", nullptr);
                                        else if (optionsUSBGadget->getSelected() == "ecm") {
                                                runSystemCommand("/usr/bin/usbgadget start cdc", "", nullptr);
                                                std::string usbip = std::string(getShOutput(R"(cat /storage/.cache/usbgadget/ip_address.conf)"));
                                                mWindow->pushGui(new GuiMsgBox(mWindow, _("USB Networking enabled, the device IP is ") + usbip, _("OK"), nullptr));
                                        }
                        }
                });


	s->addGroup(_("CLOUD SERVICES"));

       auto enable_syncthing = std::make_shared<SwitchComponent>(mWindow);
                bool syncthingEnabled = SystemConf::getInstance()->get("syncthing.enabled") == "1";
                enable_syncthing->setState(syncthingEnabled);
                s->addWithLabel(_("ENABLE SYNCTHING"), enable_syncthing);
                enable_syncthing->setOnChangedCallback([enable_syncthing] {
                        if (enable_syncthing->getState() == false) {
                                runSystemCommand("systemctl stop syncthing", "", nullptr);
                        } else {
                                runSystemCommand("systemctl start syncthing", "", nullptr);
                        }
                bool syncthingenabled = enable_syncthing->getState();
                SystemConf::getInstance()->set("syncthing.enabled", syncthingenabled ? "1" : "0");
                                SystemConf::getInstance()->saveSystemConf();
                });

       auto mount_cloud = std::make_shared<SwitchComponent>(mWindow);
                bool mntcloudEnabled = SystemConf::getInstance()->get("clouddrive.mounted") == "1";
                mount_cloud->setState(mntcloudEnabled);
                s->addWithLabel(_("MOUNT CLOUD DRIVE"), mount_cloud);
                mount_cloud->setOnChangedCallback([mount_cloud] {
                        if (mount_cloud->getState() == false) {
                                runSystemCommand("rclonectl unmount", "", nullptr);
                        } else {
                                runSystemCommand("rclonectl mount", "", nullptr);
                        }
                bool cloudenabled = mount_cloud->getState();
                SystemConf::getInstance()->set("clouddrive.mounted", cloudenabled ? "1" : "0");
                                SystemConf::getInstance()->saveSystemConf();
                });

	s->addGroup(_("VPN SERVICES"));

	const std::string wireguardConfigFile = "/storage/.config/wireguard/wg0.conf";
	if (Utils::FileSystem::exists(wireguardConfigFile)) {
		auto wireguard = std::make_shared<SwitchComponent>(mWindow);
		bool wgUp = SystemConf::getInstance()->get("wireguard.up") == "1";
		wireguard->setState(wgUp);
		s->addWithLabel(_("WIREGUARD VPN"), wireguard);
		wireguard->setOnChangedCallback([wireguard, wireguardConfigFile] {
			if (wireguard->getState() == false) {
				runSystemCommand("wg-quick down " + wireguardConfigFile, "", nullptr);
				runSystemCommand("systemctl stop connman-vpn", "", nullptr);
			} else {
				runSystemCommand("systemctl start connman-vpn", "", nullptr);
				runSystemCommand("wg-quick up " + wireguardConfigFile, "", nullptr);
			}
			SystemConf::getInstance()->set("wireguard.up", wireguard->getState() ? "1" : "0");
			SystemConf::getInstance()->saveSystemConf();
		});
	}

	auto tailscale = std::make_shared<SwitchComponent>(mWindow);
	bool tsUp = SystemConf::getInstance()->get("tailscale.up") == "1";
	tailscale->setState(tsUp);
	s->addWithLabel(_("TAILSCALE VPN"), tailscale);
	tailscale->setOnChangedCallback([tailscale] {
  		bool tsEnabled = tailscale->getState();
		if (tsEnabled) {
			runSystemCommand("systemctl start tailscaled", "", nullptr);
			runSystemCommand("tailscale up --timeout=7s", "", nullptr);
			tsEnabled = IsTailscaleUp();
		} else {
			runSystemCommand("tailscale down", "", nullptr);
			runSystemCommand("systemctl stop tailscaled", "", nullptr);
		}
		SystemConf::getInstance()->set("tailscale.up", tsEnabled ? "1" : "0");
		SystemConf::getInstance()->saveSystemConf();
	});

	std::string tsUrl;
	if ( tsUp == true) {
		if (!IsTailscaleUp(&tsUrl) && !tsUrl.empty()) {
			s->addGroup("TAILSCALE REAUTHENTICATE:");
			s->addGroup(tsUrl);
		}
	}

	auto zerotier = std::make_shared<SwitchComponent>(mWindow);
	bool ztUp = SystemConf::getInstance()->get("zerotier.up") == "1";
	zerotier->setState(ztUp);
	s->addWithLabel(_("ZeroTier One"), zerotier);
	zerotier->setOnChangedCallback([zerotier] {
	bool ztEnabled = zerotier->getState();
	    if(ztEnabled) {
			runSystemCommand("systemctl start zerotier-one", "", nullptr);
			ztEnabled = IsZeroTierUp();
		} else {
			runSystemCommand("systemctl stop zerotier-one", "", nullptr);
		}
		SystemConf::getInstance()->set("zerotier.up", ztEnabled ? "1" : "0");
		SystemConf::getInstance()->saveSystemConf();
	});

	mWindow->pushGui(s);
}

bool GuiMenu::IsTailscaleUp(std::string* loginUrl) {
	bool loggedOut = false;
	ApiSystem::executeScript("tailscale status", [loginUrl, &loggedOut](std::string line) {
		 const std::string prompt = "Log in at: ";
		 if (loginUrl && line.find(prompt) == 0)
		 	 *loginUrl = line.substr(prompt.length());

		 if (line.find("Logged out.") != std::string::npos) loggedOut = true;
	});
	return !loggedOut;
}

bool GuiMenu::IsZeroTierUp(std::string* networkId) {
	bool running = false;
	ApiSystem::executeScript("zerotier-cli -D/storage/.config/zerotier/ info", [networkId, &running](std::string line) {
		if (line.find("Error connecting to the ZeroTier") != std::string::npos ) running = false;
		else running = true;
	});
	return running;
}

void GuiMenu::openQuitMenu_batocera()
{
  GuiMenu::openQuitMenu_batocera_static(mWindow);
}

void GuiMenu::openQuitMenu_batocera_static(Window *window, bool quickAccessMenu, bool animate)
{
	auto s = new GuiSettings(window, (quickAccessMenu ? _("QUICK ACCESS") : _("QUIT")).c_str());
	s->setCloseButton("select");

	if (quickAccessMenu)
	{
		s->addGroup(_("QUICK ACCESS"));

		// Don't like one of the songs? Press next
		if (AudioManager::getInstance()->isSongPlaying())
		{
			auto sname = AudioManager::getInstance()->getSongName();
			if (!sname.empty())
			{
				s->addWithDescription(_("SKIP TO NEXT SONG"), _("LISTENING NOW") + " : " + sname, nullptr, [s, window]
				{
					Window* w = window;
					AudioManager::getInstance()->playRandomMusic(false);
					delete s;
					openQuitMenu_batocera_static(w, true, false);
				}, "iconSound");
			}
		}

		s->addEntry(_("LAUNCH SCREENSAVER"), false, [s, window]
		{
			Window* w = window;
			window->postToUiThread([w]()
			{
				w->startScreenSaver();
				w->renderScreenSaver();
			});
			delete s;

		}, "iconScraper", true);

#define USER_MANUAL_FILE "/usr/share/doc/user_guide.pdf"

		if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::ScriptId::PDFEXTRACTION) && Utils::FileSystem::exists(USER_MANUAL_FILE))
		{
			s->addEntry(_("VIEW USER MANUAL"), false, [s, window]
			{
				GuiImageViewer::showPdf(window, USER_MANUAL_FILE);
				delete s;
			}, "iconManual");
		}
	}

	if (quickAccessMenu)
		s->addGroup(_("QUIT"));

	s->addEntry(_("RESTART EMULATIONSTATION"), false, [window] {
		window->pushGui(new GuiMsgBox(window, _("REALLY RESTART EMULATIONSTATION?"), _("YES"),
			[] {
    		   Scripting::fireEvent("quit", "restart");
			   quitES(QuitMode::QUIT);
		}, _("NO"), nullptr));
	}, "iconRestart");

	s->addEntry(_("RESTART SYSTEM"), false, [window] {
		window->pushGui(new GuiMsgBox(window, _("REALLY RESTART?"),
			_("YES"), [] { quitES(QuitMode::REBOOT); },
			_("NO"), nullptr));
	}, "iconRestart");


	s->addEntry(_("SHUTDOWN SYSTEM"), false, [window] {
		window->pushGui(new GuiMsgBox(window, _("REALLY SHUTDOWN?"),
			_("YES"), [] { quitES(QuitMode::SHUTDOWN); },
			_("NO"), nullptr));
	}, "iconShutdown");

	if (quickAccessMenu && animate)
		s->getMenu().animateTo(Vector2f((Renderer::getScreenWidth() - s->getMenu().getSize().x()) / 2, (Renderer::getScreenHeight() - s->getMenu().getSize().y()) / 2));
	else if (quickAccessMenu)
		s->getMenu().setPosition((Renderer::getScreenWidth() - s->getMenu().getSize().x()) / 2, (Renderer::getScreenHeight() - s->getMenu().getSize().y()) / 2);

	window->pushGui(s);
}

void GuiMenu::createDecorationItemTemplate(Window* window, std::vector<DecorationSetInfo> sets, std::string data, ComponentListRow& row)
{
	Vector2f maxSize(Renderer::getScreenWidth() * 0.14, Renderer::getScreenHeight() * 0.14);

	int IMGPADDING = Renderer::getScreenHeight()*0.01f;

	auto theme = ThemeData::getMenuTheme();
	std::shared_ptr<Font> font = theme->Text.font;
	unsigned int color = theme->Text.color;

	// spacer between icon and text
	auto spacer = std::make_shared<GuiComponent>(window);
	spacer->setSize(IMGPADDING, 0);
	row.addElement(spacer, false);
	row.addElement(std::make_shared<TextComponent>(window, Utils::String::toUpper(Utils::String::replace(data, "_", " ")), font, color, ALIGN_LEFT), true, true);

	std::string imageUrl;

	for (auto set : sets)
		if (set.name == data)
			imageUrl = set.imageUrl;

	// image
	if (!imageUrl.empty())
	{
		auto icon = std::make_shared<ImageComponent>(window);
		icon->setImage(imageUrl, false, maxSize);
		icon->setMaxSize(maxSize);
		icon->setColorShift(theme->Text.color);
		icon->setPadding(IMGPADDING);
		row.addElement(icon, false);
	}
}

void GuiMenu::popSystemConfigurationGui(Window* mWindow, SystemData* systemData)
{
	popSpecificConfigurationGui(mWindow,
		systemData->getFullName(),
		systemData->getName(),
		systemData,
		nullptr);
}

void GuiMenu::popGameConfigurationGui(Window* mWindow, FileData* fileData)
{
	popSpecificConfigurationGui(mWindow,
		fileData->getName(),
		fileData->getConfigurationName(),
		fileData->getSourceFileData()->getSystem(),
		fileData);
}

void GuiMenu::popSpecificConfigurationGui(Window* mWindow, std::string title, std::string configName, SystemData *systemData, FileData* fileData, bool selectCoreLine)
{
	// The system configuration
	GuiSettings* systemConfiguration = new GuiSettings(mWindow, title.c_str());

	if (fileData != nullptr)
		systemConfiguration->setSubTitle(systemData->getFullName());

	std::string currentEmulator = fileData != nullptr ? fileData->getEmulator(false) : systemData->getEmulator(false);
	std::string currentCore = fileData != nullptr ? fileData->getCore(false) : systemData->getCore(false);

	if (systemData->hasEmulatorSelection())
	{
		auto emulChoice = std::make_shared<OptionListComponent<std::string>>(mWindow, _("Emulator"), false);
		emulChoice->add(_("DEFAULT"), "", false);
		for (auto& emul : systemData->getEmulators())
		{
			if (emul.cores.size() == 0)
				emulChoice->add(emul.name, emul.name, emul.name == currentEmulator);
			else
			{
				for (auto& core : emul.cores)
				{
					bool selected = (emul.name == currentEmulator && core.name == currentCore);

					if (emul.name == core.name)
						emulChoice->add(emul.name, emul.name + "/" + core.name, selected);
					else
						emulChoice->add(emul.name + ": " + Utils::String::replace(core.name, "_", " "), emul.name + "/" + core.name, selected);
				}
			}
		}

		if (!emulChoice->hasSelection())
			emulChoice->selectFirstItem();

		emulChoice->setSelectedChangedCallback([mWindow, title, systemConfiguration, systemData, fileData, configName, emulChoice](std::string s)
		{
			std::string newEmul;
			std::string newCore;

			auto values = Utils::String::split(emulChoice->getSelected(), '/');
			if (values.size() > 0)
				newEmul = values[0];

			if (values.size() > 1)
				newCore = values[1];

			if (fileData != nullptr)
			{
				fileData->setEmulator(newEmul);
				fileData->setCore(newCore);
			}
			else
			{
				SystemConf::getInstance()->set(configName + ".emulator", newEmul);
				SystemConf::getInstance()->set(configName + ".core", newCore);
			}
			popSpecificConfigurationGui(mWindow, title, configName, systemData, fileData);
			delete systemConfiguration;

		});

		systemConfiguration->addWithLabel(_("Emulator"), emulChoice);
	}

    // System settings
    systemConfiguration->addEntry(_("SYSTEM OPTIONS"), true, [mWindow, configName] { openSystemOptionsConfiguration(mWindow, configName); });

	// Screen ratio choice
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::ratio))
	{
		auto ratio_choice = createRatioOptionList(mWindow, configName);
		systemConfiguration->addWithLabel(_("GAME ASPECT RATIO"), ratio_choice);
		systemConfiguration->addSaveFunc([configName, ratio_choice] { SystemConf::getInstance()->set(configName + ".ratio", ratio_choice->getSelected()); });
/*		if (ratio_choice->getSelected() == "custom") {
			systemConfiguration->addEntry(_("WIDTH/HEIGHT"), true, [mWindow, configName] { openCustomAspectRatioConfiguration(mWindow, configName); });
		}
*/
		auto rotation_choice = std::make_shared<OptionListComponent<std::string>>(mWindow, _("SCREEN ROTATION"));
		rotation_choice->addRange({{_("DEFAULT"), "default"}, {_("90"), "1"}, {_("180"), "2"}, {_("270"), "3"}}, SystemConf::getInstance()->get(configName + ".rotation"));
		systemConfiguration->addWithLabel(_("SCREEN ROTATION"), rotation_choice);
		systemConfiguration->addSaveFunc([configName, rotation_choice] { SystemConf::getInstance()->set(configName + ".rotation", rotation_choice->getSelected()); });
	}

	// video resolution mode
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::videomode))
	{
		auto videoResolutionMode_choice = createVideoResolutionModeOptionList(mWindow, configName);
		systemConfiguration->addWithLabel(_("VIDEO MODE"), videoResolutionMode_choice);
		systemConfiguration->addSaveFunc([configName, videoResolutionMode_choice] { SystemConf::getInstance()->set(configName + ".videomode", videoResolutionMode_choice->getSelected()); });
	}

	// smoothing
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::smooth))
	{
		auto smoothing_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("BILINEAR FILTERING"));
		smoothing_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get(configName + ".smooth"));
		systemConfiguration->addWithLabel(_("BILINEAR FILTERING"), smoothing_enabled);
		systemConfiguration->addSaveFunc([configName, smoothing_enabled] { SystemConf::getInstance()->set(configName + ".smooth", smoothing_enabled->getSelected()); });
	}

	// rewind
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::rewind))
	{
		auto rewind_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("REWIND"));
		rewind_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get(configName + ".rewind"));
		systemConfiguration->addWithLabel(_("REWIND"), rewind_enabled);
		systemConfiguration->addSaveFunc([configName, rewind_enabled] { SystemConf::getInstance()->set(configName + ".rewind", rewind_enabled->getSelected()); });
	}

	// autosave
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::autosave))
	{
		auto autosave_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("AUTO SAVE/LOAD ON GAME LAUNCH"));
		autosave_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}, {_("SHOW SAVE STATES"), "2"}, {_("SHOW SAVE STATES IF NOT EMPTY"), "3"}}, SystemConf::getInstance()->get(configName + ".autosave"));
		systemConfiguration->addWithLabel(_("AUTO SAVE/LOAD ON GAME LAUNCH"), autosave_enabled);
		systemConfiguration->addSaveFunc([configName, autosave_enabled] { SystemConf::getInstance()->set(configName + ".autosave", autosave_enabled->getSelected()); });
	}

	// Overlays preset
	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::SHADERS) &&
		systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::shaders))
	{
		std::string a;
		auto overlays_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("OVERLAY SET"),false);
		std::string currentOverlay = SystemConf::getInstance()->get(configName + ".overlayset");
		if (currentOverlay.empty()) {
			currentOverlay = std::string("default");
		}

		overlays_choices->add(_("DEFAULT"), "default", currentOverlay == "default");
		overlays_choices->add(_("NONE"), "none", currentOverlay == "none");
		for(std::stringstream ss(getShOutput(R"(/usr/bin/getoverlays)")); getline(ss, a, ','); )
		overlays_choices->add(a, a, currentOverlay == a); // emuelec
		systemConfiguration->addWithLabel(_("OVERLAY SET"), overlays_choices);
		systemConfiguration->addSaveFunc([overlays_choices, configName] { SystemConf::getInstance()->set(configName + ".overlayset", overlays_choices->getSelected()); });
	}
	

	// Shaders preset
	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::SHADERS) &&
		systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::shaders))
	{
        std::string a;
		auto shaders_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("SHADER SET"),false);
		std::string currentShader = SystemConf::getInstance()->get(configName + ".shaderset");
		if (currentShader.empty()) {
			currentShader = std::string("default");
		}

		shaders_choices->add(_("DEFAULT"), "default", currentShader == "default");
		shaders_choices->add(_("NONE"), "none", currentShader == "none");
		for(std::stringstream ss(getShOutput(R"(/usr/bin/getshaders)")); getline(ss, a, ','); )
		shaders_choices->add(a, a, currentShader == a); // emuelec
		systemConfiguration->addWithLabel(_("SHADER SET"), shaders_choices);
		systemConfiguration->addSaveFunc([shaders_choices, configName] { SystemConf::getInstance()->set(configName + ".shaderset", shaders_choices->getSelected()); });
	}

	// Filters preset
	if (ApiSystem::getInstance()->isScriptingSupported(ApiSystem::SHADERS) &&
		systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::shaders))
	{
		std::string a;
		auto filters_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("FILTER SET"),false);
		std::string currentFilter = SystemConf::getInstance()->get(configName + ".filterset");
		if (currentFilter.empty()) {
			currentFilter = std::string("default");
		}

		filters_choices->add(_("DEFAULT"), "default", currentFilter == "default");
		filters_choices->add(_("NONE"), "none", currentFilter == "none");
		for(std::stringstream ss(getShOutput(R"(/usr/bin/getfilters)")); getline(ss, a, ','); )
		filters_choices->add(a, a, currentFilter == a); // emuelec
		systemConfiguration->addWithLabel(_("FILTER SET"), filters_choices);
		systemConfiguration->addSaveFunc([filters_choices, configName] { SystemConf::getInstance()->set(configName + ".filterset", filters_choices->getSelected()); });
	}

	// Vertical Game
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::vertical))
	{
		auto vertical_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("ENABLE VERTICAL"));
		vertical_enabled->add(_("OFF"), "default", SystemConf::getInstance()->get(configName + ".vertical") != "1");
		vertical_enabled->add(_("ON"), "1", SystemConf::getInstance()->get(configName + ".vertical") == "1");
		systemConfiguration->addWithLabel(_("ENABLE VERTICAL"), vertical_enabled);
		systemConfiguration->addSaveFunc([configName, vertical_enabled] { SystemConf::getInstance()->set(configName + ".vertical", vertical_enabled->getSelected()); });

        auto vert_aspect_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("VERTICAL ASPECT RATIO"));
		vert_aspect_enabled->addRange({ { _("16:9") , "1" }, { _("3:2") , "7" }, { _("21:9"), "4" }, { _("4:3") , "0" } }, SystemConf::getInstance()->get(configName + ".vert_aspect"));
		systemConfiguration->addWithLabel(_("VERTICAL ASPECT RATIO"), vert_aspect_enabled);
		systemConfiguration->addSaveFunc([configName, vert_aspect_enabled] { SystemConf::getInstance()->set(configName + ".vert_aspect", vert_aspect_enabled->getSelected()); });
	}
	// Integer scale
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::pixel_perfect))
	{
		auto integerscale_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("INTEGER SCALING (PIXEL PERFECT)"));
		integerscale_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get(configName + ".integerscale"));
		systemConfiguration->addWithLabel(_("INTEGER SCALING (PIXEL PERFECT)"), integerscale_enabled);
		systemConfiguration->addSaveFunc([integerscale_enabled, configName] { SystemConf::getInstance()->set(configName + ".integerscale", integerscale_enabled->getSelected()); });
	}

	// Integer scale overscale
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::pixel_perfect))
	{
		auto integerscaleoverscale_enabled = std::make_shared<OptionListComponent<std::string>>(mWindow, _("INTEGER SCALE OVERSCALE"));
		integerscaleoverscale_enabled->addRange({{_("DEFAULT"), "default"}, {_("ON"), "1"}, {_("OFF"), "0"}}, SystemConf::getInstance()->get(configName + ".integerscaleoverscale"));
		systemConfiguration->addWithLabel(_("INTEGER SCALE OVERSCALE"), integerscaleoverscale_enabled);
		systemConfiguration->addSaveFunc([integerscaleoverscale_enabled, configName] { SystemConf::getInstance()->set(configName + ".integerscaleoverscale", integerscaleoverscale_enabled->getSelected()); });
	}

#if defined(AMD64)

        // Allow offlining all but n threads
        auto optionsThreads = std::make_shared<OptionListComponent<std::string> >(mWindow, _("AVAILABLE THREADS"), false);

	std::vector<std::string> availableThreads = ApiSystem::getInstance()->getAvailableThreads();
	std::string selectedThreads = SystemConf::getInstance()->get(configName + ".threads");
	if (selectedThreads.empty())
		selectedThreads = "default";

        bool wfound = false;
        for (auto it = availableThreads.begin(); it != availableThreads.end(); it++)
        {
                optionsThreads->add((*it), (*it), selectedThreads == (*it));
                if (selectedThreads == (*it))
                wfound = true;
        }
        if (!wfound)
                optionsThreads->add(selectedThreads, selectedThreads, true);

        systemConfiguration->addWithLabel(_("AVAILABLE THREADS"), optionsThreads);

        systemConfiguration->addSaveFunc([configName, optionsThreads, selectedThreads]
        {
                if (optionsThreads->changed()) {
                        SystemConf::getInstance()->set(configName +".threads", optionsThreads->getSelected());
                        runSystemCommand("/usr/bin/sh -lc \". /etc/profile.d/099-freqfunctions; onlinethreads " + optionsThreads->getSelected() + " 0" + "\"" , "", nullptr);
                        SystemConf::getInstance()->saveSystemConf();
                }
        });

#endif

// Prep for additional device support.
#if defined(AMD64)
        std::vector<std::string> cpuVendor = ApiSystem::getInstance()->getCPUVendor();
	std::vector<std::string> tdpRange = ApiSystem::getInstance()->getTdpRange();
        auto it = cpuVendor.begin();

        if (*it == "AuthenticAMD") {
	        // Provides overclock profile switching
	        auto optionsOCProfile = std::make_shared<OptionListComponent<std::string> >(mWindow, _("CPU TDP Max"), false);
	        std::string selectedOCProfile = SystemConf::getInstance()->get(configName + ".overclock");
	        if (selectedOCProfile.empty())
	                selectedOCProfile = "default";

		bool xfound = false;
		for (auto it = tdpRange.begin(); it != tdpRange.end(); it++)
		{
			optionsOCProfile->add((*it), (*it), selectedOCProfile == (*it));
			if (selectedOCProfile == (*it))
				xfound = true;
		}


		if (!xfound)
			optionsOCProfile->add(selectedOCProfile, selectedOCProfile, true);

	        systemConfiguration->addWithLabel(_("CPU TDP Max"), optionsOCProfile);

	        systemConfiguration->addSaveFunc([optionsOCProfile, selectedOCProfile, configName, mWindow]
	        {
	                if (optionsOCProfile->changed()) {
	                        mWindow->pushGui(new GuiMsgBox(mWindow, _("WARNING: OVERCLOCKING YOUR DEVICE MAY RESULT IN STABILITY PROBLEMS OR CAUSE HARDWARE DAMAGE!\n\nUSING THE QUIET COOLING PROFILE WHILE USING CERTAIN OVERCLOCKS MAY CAUSE PANIC REBOOTS!\n\nJELOS IS NOT RESPONSIBLE FOR ANY DAMAGE THAT MAY OCCUR USING THESE SETTINGS!\n\nCLICK YES THAT YOU AGREE, OR NO TO CANCEL."), _("YES"),
				[optionsOCProfile,configName] {
	                                SystemConf::getInstance()->set(configName + ".overclock", optionsOCProfile->getSelected());
	                                SystemConf::getInstance()->saveSystemConf();
	                        }, _("NO"), nullptr));
	                }
	        });
	}

	if (Utils::FileSystem::exists("/sys/devices/system/cpu/cpufreq/policy0/energy_performance_preference")) {
                // Provides EPP Profile switching
                auto optionsEPP = std::make_shared<OptionListComponent<std::string> >(mWindow, _("Energy Performance Preference"), false);
                std::string selectedEPP = SystemConf::getInstance()->get(configName + ".power.epp");
                if (selectedEPP.empty())
                        selectedEPP = "default";

                optionsEPP->add(_("DEFAULT"), "default", selectedEPP == "default");

                optionsEPP->add(_("Performance"),"performance", selectedEPP == "performance");
                optionsEPP->add(_("Balance Performance"),"balance_performance", selectedEPP == "balance_performance");
                optionsEPP->add(_("Balance Power Saving"),"balance_power", selectedEPP == "balance_power");
                optionsEPP->add(_("Power Saving"),"power", selectedEPP == "power");

                systemConfiguration->addWithLabel(_("Energy Performance Preference"), optionsEPP);

                systemConfiguration->addSaveFunc([optionsEPP, selectedEPP, configName]
                {
                        if (optionsEPP->changed()) {
                                SystemConf::getInstance()->set(configName + ".power.epp", optionsEPP->getSelected());
                                SystemConf::getInstance()->saveSystemConf();
                        }
                });
        }

#endif

	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::latency_reduction))
		systemConfiguration->addEntry(_("LATENCY REDUCTION"), true, [mWindow, configName] { openLatencyReductionConfiguration(mWindow, configName); });

	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::colorization))
	{

		std::string currentEmulator = fileData != nullptr ? fileData->getEmulator(false) : systemData->getEmulator(false);
		LOG(LogError) << currentEmulator;


		// gameboy colorize
		auto colorizations_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("COLORIZATION"), false);
		auto twb1_colorizations_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("TWB - PACK 1 PALETTE"), false);
		auto twb2_colorizations_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("TWB - PACK 2 PALETTE"), false);
		auto twb3_colorizations_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("TWB - PACK 3 PALETTE"), false);
		auto pixelshift1_colorizations_choices = std::make_shared<OptionListComponent<std::string> >(mWindow, _("PIXELSHIFT - PACK 1 PALETTE"), false);

		std::string currentColorization = SystemConf::getInstance()->get(configName + ".renderer.colorization");
		std::string twb1_currentColorization = SystemConf::getInstance()->get(configName + ".renderer.twb1_colorization");
		std::string twb2_currentColorization = SystemConf::getInstance()->get(configName + ".renderer.twb2_colorization");
		std::string twb3_currentColorization = SystemConf::getInstance()->get(configName + ".renderer.twb3_colorization");
		std::string pixelshift1_currentColorization = SystemConf::getInstance()->get(configName + ".renderer.pixelshift1_colorization");




		if (currentColorization.empty())
			currentColorization = std::string("default");
		if (twb1_currentColorization.empty())
			twb1_currentColorization = std::string("TWB64 001 - Aqours Blue");
		if (twb2_currentColorization.empty())
			twb2_currentColorization = std::string("TWB64 101 - 765PRO Pink");
		if (twb3_currentColorization.empty())
			twb3_currentColorization = std::string("TWB64 201 - DMG-GOLD");
		if (pixelshift1_currentColorization.empty())
			pixelshift1_currentColorization = std::string("PixelShift 01 - Arctic Green");

		colorizations_choices->add(_("DEFAULT"), "default", currentColorization == "default");
		colorizations_choices->add(_("NONE"), "none", currentColorization == "none");
		colorizations_choices->add(_("GBC"), "GBC", currentColorization == "GBC");
		colorizations_choices->add(_("SGB"), "SGB", currentColorization == "SGB");
		colorizations_choices->add(_("Best Guess"), "Best Guess", currentColorization == "Best Guess");

		const char* all_gambate_gc_colors_modes[] = { "GB - DMG",
								 "GB - Light",
								 "GB - Pocket",
								 "GBC - Blue",
								 "GBC - Brown",
								 "GBC - Dark Blue",
								 "GBC - Dark Brown",
								 "GBC - Dark Green",
								 "GBC - Grayscale",
								 "GBC - Green",
								 "GBC - Inverted",
								 "GBC - Orange",
								 "GBC - Pastel Mix",
								 "GBC - Red",
								 "GBC - Yellow",
								 "SGB - 1A",
								 "SGB - 1B",
								 "SGB - 1C",
								 "SGB - 1D",
								 "SGB - 1E",
								 "SGB - 1F",
								 "SGB - 1G",
								 "SGB - 1H",
								 "SGB - 2A",
								 "SGB - 2B",
								 "SGB - 2C",
								 "SGB - 2D",
								 "SGB - 2E",
								 "SGB - 2F",
								 "SGB - 2G",
								 "SGB - 2H",
								 "SGB - 3A",
								 "SGB - 3B",
								 "SGB - 3C",
								 "SGB - 3D",
								 "SGB - 3E",
								 "SGB - 3F",
								 "SGB - 3G",
								 "SGB - 3H",
								 "SGB - 4A",
								 "SGB - 4B",
								 "SGB - 4C",
								 "SGB - 4D",
								 "SGB - 4E",
								 "SGB - 4F",
								 "SGB - 4G",
								 "SGB - 4H",
								 "Special 1",
								 "Special 2",
								 "Special 3",
								 "Special 4 (TI-83 Legacy)",
								 "TWB64 - Pack 1",
								 "TWB64 - Pack 2",
								 "TWB64 - Pack 3",
								 "PixelShift - Pack 1"};

		const char* twb1_colors_modes[] = {
								 "TWB64 001 - Aqours Blue",
								 "TWB64 002 - Anime Expo Ver.",
								 "TWB64 003 - SpongeBob Yellow",
								 "TWB64 004 - Patrick Star Pink",
								 "TWB64 005 - Neon Red",
								 "TWB64 006 - Neon Blue",
								 "TWB64 007 - Neon Yellow",
								 "TWB64 008 - Neon Green",
								 "TWB64 009 - Neon Pink",
								 "TWB64 010 - Mario Red",
								 "TWB64 011 - Nick Orange",
								 "TWB64 012 - Virtual Vision",
								 "TWB64 013 - Golden Wild",
								 "TWB64 014 - Builder Yellow",
								 "TWB64 015 - Classic Blurple",
								 "TWB64 016 - 765 Production Ver.",
								 "TWB64 017 - Superball Ivory",
								 "TWB64 018 - Crunchyroll Orange",
								 "TWB64 019 - Muse Pink",
								 "TWB64 020 - Nijigasaki Yellow",
								 "TWB64 021 - Gamate Ver.",
								 "TWB64 022 - Greenscale Ver.",
								 "TWB64 023 - Odyssey Gold",
								 "TWB64 024 - Super Saiyan God",
								 "TWB64 025 - Super Saiyan Blue",
								 "TWB64 026 - Bizarre Pink",
								 "TWB64 027 - Nintendo Switch Lite Ver.",
								 "TWB64 028 - Game.com Ver.",
								 "TWB64 029 - Sanrio Pink",
								 "TWB64 030 - BANDAI NAMCO Ver.",
								 "TWB64 031 - Cosmo Green",
								 "TWB64 032 - Wanda Pink",
								 "TWB64 033 - Link's Awakening DX Ver.",
								 "TWB64 034 - Travel Wood",
								 "TWB64 035 - Pokemon Ver.",
								 "TWB64 036 - Game Grump Orange",
								 "TWB64 037 - Scooby-Doo Mystery Ver.",
								 "TWB64 038 - Pokemon mini Ver.",
								 "TWB64 039 - Supervision Ver.",
								 "TWB64 040 - DMG Ver.",
								 "TWB64 041 - Pocket Ver.",
								 "TWB64 042 - Light Ver.",
								 "TWB64 043 - Miraitowa Blue",
								 "TWB64 044 - Someity Pink",
								 "TWB64 045 - Pikachu Yellow",
								 "TWB64 046 - Eevee Brown",
								 "TWB64 047 - Microvision Ver.",
								 "TWB64 048 - TI-83 Ver.",
								 "TWB64 049 - Aegis Cherry",
								 "TWB64 050 - Labo Fawn",
								 "TWB64 051 - MILLION LIVE GOLD!",
								 "TWB64 052 - Tokyo Midtown Ver.",
								 "TWB64 053 - VMU Ver.",
								 "TWB64 054 - Game Master Ver.",
								 "TWB64 055 - Android Green",
								 "TWB64 056 - Walmart Discount Blue",
								 "TWB64 057 - Google Red",
								 "TWB64 058 - Google Blue",
								 "TWB64 059 - Google Yellow",
								 "TWB64 060 - Google Green",
								 "TWB64 061 - WonderSwan Ver.",
								 "TWB64 062 - Neo Geo Pocket Ver.",
								 "TWB64 063 - Dew Green",
								 "TWB64 064 - Coca-Cola Red",
								 "TWB64 065 - GameKing Ver.",
								 "TWB64 066 - Do The Dew Ver.",
								 "TWB64 067 - Digivice Ver.",
								 "TWB64 068 - Bikini Bottom Ver.",
								 "TWB64 069 - Blossom Pink",
								 "TWB64 070 - Bubbles Blue",
								 "TWB64 071 - Buttercup Green",
								 "TWB64 072 - NASCAR Ver.",
								 "TWB64 073 - Lemon-Lime Green",
								 "TWB64 074 - Mega Man V Ver.",
								 "TWB64 075 - Tamagotchi Ver.",
								 "TWB64 076 - Phantom Red",
								 "TWB64 077 - Halloween Ver.",
								 "TWB64 078 - Christmas Ver.",
								 "TWB64 079 - Cardcaptor Pink",
								 "TWB64 080 - Pretty Guardian Gold",
								 "TWB64 081 - Camouflage Ver.",
								 "TWB64 082 - Legendary Super Saiyan",
								 "TWB64 083 - Super Saiyan Rose",
								 "TWB64 084 - Super Saiyan",
								 "TWB64 085 - Perfected Ultra Instinct",
								 "TWB64 086 - Saint Snow Red",
								 "TWB64 087 - Yellow Banana",
								 "TWB64 088 - Green Banana",
								 "TWB64 089 - Super Saiyan 3",
								 "TWB64 090 - Super Saiyan Blue Evolved",
								 "TWB64 091 - Pocket Tales Ver.",
								 "TWB64 092 - Investigation Yellow",
								 "TWB64 093 - S.E.E.S. Blue",
								 "TWB64 094 - Game Awards Cyan",
								 "TWB64 095 - Hokage Orange",
								 "TWB64 096 - Straw Hat Red",
								 "TWB64 097 - Sword Art Cyan",
								 "TWB64 098 - Deku Alpha Emerald",
								 "TWB64 099 - Blue Stripes Ver.",
								 "TWB64 100 - Stone Orange"};

		const char* twb2_colors_modes[] = {
								 "TWB64 101 - 765PRO Pink",              
								 "TWB64 102 - CINDERELLA Blue",          
								 "TWB64 103 - MILLION Yellow!",          
								 "TWB64 104 - SideM Green",              
								 "TWB64 105 - SHINY Sky Blue",           
								 "TWB64 106 - Angry Volcano Ver.",       
								 "TWB64 107 - Yo-kai Pink",              
								 "TWB64 108 - Yo-kai Green",             
								 "TWB64 109 - Yo-kai Blue",              
								 "TWB64 110 - Yo-kai Purple",            
								 "TWB64 111 - Aquatic Iro",              
								 "TWB64 112 - Tea Midori",               
								 "TWB64 113 - Sakura Pink",              
								 "TWB64 114 - Wisteria Murasaki",        
								 "TWB64 115 - Oni Aka",                  
								 "TWB64 116 - Golden Kiiro",             
								 "TWB64 117 - Silver Shiro",             
								 "TWB64 118 - Fruity Orange",            
								 "TWB64 119 - AKB48 Pink",               
								 "TWB64 120 - Miku Blue",                
								 "TWB64 121 - Fairy Tail Red",           
								 "TWB64 122 - Survey Corps Brown",       
								 "TWB64 123 - Island Green",             
								 "TWB64 124 - Mania Plus Green",         
								 "TWB64 125 - Ninja Turtle Green",       
								 "TWB64 126 - Slime Blue",               
								 "TWB64 127 - Lime Midori",              
								 "TWB64 128 - Ghostly Aoi",              
								 "TWB64 129 - Retro Bogeda",             
								 "TWB64 130 - Royal Blue",               
								 "TWB64 131 - Neon Purple",              
								 "TWB64 132 - Neon Orange",              
								 "TWB64 133 - Moonlight Vision",         
								 "TWB64 134 - Tokyo Red",                
								 "TWB64 135 - Paris Gold",               
								 "TWB64 136 - Beijing Blue",             
								 "TWB64 137 - Pac-Man Yellow",           
								 "TWB64 138 - Irish Green",              
								 "TWB64 139 - Kakarot Orange",           
								 "TWB64 140 - Dragon Ball Orange",       
								 "TWB64 141 - Christmas Gold",           
								 "TWB64 142 - Pepsi Vision",             
								 "TWB64 143 - Bubblun Green",            
								 "TWB64 144 - Bobblun Blue",             
								 "TWB64 145 - Baja Blast Storm",         
								 "TWB64 146 - Olympic Gold",             
								 "TWB64 147 - Value Orange",             
								 "TWB64 148 - Liella Purple!",           
								 "TWB64 149 - Olympic Silver",           
								 "TWB64 150 - Olympic Bronze",           
								 "TWB64 151 - ANA Sky Blue",             
								 "TWB64 152 - Nijigasaki Orange",        
								 "TWB64 153 - holoblue",                 
								 "TWB64 154 - Wrestling Red",            
								 "TWB64 155 - Yoshi Egg Green",          
								 "TWB64 156 - Pokedex Red",              
								 "TWB64 157 - Disney Dream Blue",        
								 "TWB64 158 - Xbox Green",               
								 "TWB64 159 - Sonic Mega Blue",          
								 "TWB64 160 - Sprite Green",             
								 "TWB64 161 - Scarlett Green",           
								 "TWB64 162 - Glitchy Blue",             
								 "TWB64 163 - Classic LCD",              
								 "TWB64 164 - 3DS Virtual Console Ver.", 
								 "TWB64 165 - PocketStation Ver.",       
								 "TWB64 166 - Timeless Gold and Red",    
								 "TWB64 167 - Smurfy Blue",              
								 "TWB64 168 - Swampy Ogre Green",        
								 "TWB64 169 - Sailor Spinach Green",     
								 "TWB64 170 - Shenron Green",            
								 "TWB64 171 - Berserk Blood",            
								 "TWB64 172 - Super Star Pink",          
								 "TWB64 173 - Gamebuino Classic Ver.",   
								 "TWB64 174 - Barbie Pink",              
								 "TWB64 175 - Star Command Green",       
								 "TWB64 176 - Nokia 3310 Ver.",          
								 "TWB64 177 - Clover Green",             
								 "TWB64 178 - Crash Orange",             
								 "TWB64 179 - Famicom Disk Yellow",      
								 "TWB64 180 - Team Rocket Red",          
								 "TWB64 181 - SEIKO Timer Yellow",       
								 "TWB64 182 - PINK109",                  
								 "TWB64 183 - Doraemon Tricolor",        
								 "TWB64 184 - Fury Blue",                
								 "TWB64 185 - Rockstar Orange",          
								 "TWB64 186 - Puyo Puyo Green",          
								 "TWB64 187 - Susan G. Pink",            
								 "TWB64 188 - Pizza Hut Red",            
								 "TWB64 189 - Plumbob Green",            
								 "TWB64 190 - Grand Ivory",              
								 "TWB64 191 - Demon's Gold",             
								 "TWB64 192 - SEGA Tokyo Blue",          
								 "TWB64 193 - Champion's Tunic",         
								 "TWB64 194 - DK Barrel Brown",          
								 "TWB64 195 - EVA-01",                   
								 "TWB64 196 - Equestrian Purple",        
								 "TWB64 197 - Autobot Red",              
								 "TWB64 198 - niconico sea green",       
								 "TWB64 199 - Duracell Copper",          
								 "TWB64 200 - TOKYO SKYTREE CLOUDY BLUE"};

		const char* twb3_colors_modes[] = {
								 "TWB64 201 - DMG-GOLD",               
								 "TWB64 202 - LCD Clock Green",        
								 "TWB64 203 - Famicom Frenzy",         
								 "TWB64 204 - DK Arcade Blue",         
								 "TWB64 205 - Advanced Indigo",        
								 "TWB64 206 - Ultra Black",            
								 "TWB64 207 - Chaos Emerald Green",    
								 "TWB64 208 - Blue Bomber Azure",      
								 "TWB64 209 - Garry's Blue",           
								 "TWB64 210 - Steam Gray",             
								 "TWB64 211 - Dream Land GB Ver.",     
								 "TWB64 212 - Pokemon Pinball Ver.",   
								 "TWB64 213 - Poketch Ver.",           
								 "TWB64 214 - COLLECTION of SaGa Ver.",
								 "TWB64 215 - Rocky-Valley Holiday",   
								 "TWB64 216 - Giga Kiwi DMG",          
								 "TWB64 217 - DMG Pea Green",          
								 "TWB64 218 - Timing Hero Ver.",       
								 "TWB64 219 - Invincible Blue",        
								 "TWB64 220 - Grinchy Green",          
								 "TWB64 221 - Winter Icy Blue",        
								 "TWB64 222 - School Idol Mix",        
								 "TWB64 223 - Green Awakening",        
								 "TWB64 224 - Goomba Brown",           
								 "TWB64 225 - Devil Red",              
								 "TWB64 226 - Simpson Yellow",         
								 "TWB64 227 - Spooky Purple",          
								 "TWB64 228 - Treasure Gold",          
								 "TWB64 229 - Cherry Blossom Pink",    
								 "TWB64 230 - Golden Trophy",          
								 "TWB64 231 - Winter Icy Blue",        
								 "TWB64 232 - Leprechaun Green",       
								 "TWB64 233 - SAITAMA SUPER BLUE",     
								 "TWB64 234 - SAITAMA SUPER GREEN",    
								 "TWB64 235 - Duolingo Green",         
								 "TWB64 236 - Super Mushroom Vision",  
								 "TWB64 237 - Ancient Hisuian Brown",  
								 "TWB64 238 - Sky Pop Ivory",          
								 "TWB64 239 - LAWSON BLUE",            
								 "TWB64 240 - Anime Expo Red",         
								 "TWB64 241 - Brilliant Diamond Blue", 
								 "TWB64 242 - Shining Pearl Pink",     
								 "TWB64 243 - Funimation Melon",       
								 "TWB64 244 - Teyvat Brown",           
								 "TWB64 245 - Chozo Blue",             
								 "TWB64 246 - Spotify Green",          
								 "TWB64 247 - Dr Pepper Red",          
								 "TWB64 248 - NHK Silver Gray",        
								 "TWB64 249 - Starbucks Green",        
								 "TWB64 250 - Tokyo Disney Magic",     
								 "TWB64 251 - Kingdom Key Gold",       
								 "TWB64 252 - Hogwarts Goldius",       
								 "TWB64 253 - Kentucky Fried Red",     
								 "TWB64 254 - Cheeto Orange",          
								 "TWB64 255 - Namco Idol Pink",        
								 "TWB64 256 - Domino's Blue",          
								 "TWB64 257 - Pac-Man Vision",         
								 "TWB64 258 - Bill's PC Screen",       
								 "TWB64 259 - Sonic Mega Blue",        
								 "TWB64 260 - Fool's Gold and Silver", 
								 "TWB64 261 - UTA RED",                
								 "TWB64 262 - Metallic Paldea Brass",  
								 "TWB64 263 - Classy Christmas",       
								 "TWB64 264 - Winter Christmas",       
								 "TWB64 265 - IDOL WORLD TRICOLOR!!!", 
								 "TWB64 266 - Inkling Tricolor",       
								 "TWB64 267 - 7-Eleven Color Combo",   
								 "TWB64 268 - PAC-PALETTE",            
								 "TWB64 269 - Vulnerable Blue",        
								 "TWB64 270 - Nightvision Green",      
								 "TWB64 271 - Bandai Namco Tricolor",  
								 "TWB64 272 - Gold, Silver, and Bronze",
								 "TWB64 273 - Arendelle Winter Blue",  
								 "TWB64 274 - Super Famicom Supreme",  
								 "TWB64 275 - Absorbent and Yellow",   
								 "TWB64 276 - 765PRO TRICOLOR",        
								 "TWB64 277 - GameCube Glimmer",       
								 "TWB64 278 - 1st Vision Pastel",      
								 "TWB64 279 - Perfect Majin Emperor",  
								 "TWB64 280 - J-Pop Idol Sherbet",     
								 "TWB64 281 - Ryuuguu Sunset",         
								 "TWB64 282 - Tropical Starfall",      
								 "TWB64 283 - Colorful Horizons",      
								 "TWB64 284 - BLACKPINK BLINK PINK",   
								 "TWB64 285 - DMG-SWITCH",             
								 "TWB64 286 - POCKET SWITCH",          
								 "TWB64 287 - Sunny Passion Paradise", 
								 "TWB64 288 - Saiyan Beast Silver",    
								 "TWB64 289 - RADIANT SMILE RAMP",     
								 "TWB64 290 - A-RISE BLUE",            
								 "TWB64 291 - TROPICAL TWICE APRICOT", 
								 "TWB64 292 - Odyssey Boy",            
								 "TWB64 293 - Frog Coin Green",        
								 "TWB64 294 - Garfield Vision",        
								 "TWB64 295 - Bedrock Caveman Vision", 
								 "TWB64 296 - BANGTAN ARMY PURPLE",    
								 "TWB64 297 - Spider-Verse Red",       
								 "TWB64 298 - Baja Blast Beach",       
								 "TWB64 299 - 3DS Virtual Console Green",
								 "TWB64 300 - Wonder Purple"};

		const char* pixelshift1_colors_modes[] = {
								 "PixelShift 01 - Arctic Green",              
								 "PixelShift 02 - Arduboy",                   
								 "PixelShift 03 - BGB 0.3 Emulator",          
								 "PixelShift 04 - Camouflage",                
								 "PixelShift 05 - Chocolate Bar",             
								 "PixelShift 06 - CMYK",                      
								 "PixelShift 07 - Cotton Candy",              
								 "PixelShift 08 - Easy Greens",               
								 "PixelShift 09 - Gamate",                    
								 "PixelShift 10 - Game Boy Light",            
								 "PixelShift 11 - Game Boy Pocket",           
								 "PixelShift 12 - Game Boy Pocket Alt",       
								 "PixelShift 13 - Game Pocket Computer",      
								 "PixelShift 14 - Game & Watch Ball",         
								 "PixelShift 15 - GB Backlight Blue",         
								 "PixelShift 16 - GB Backlight Faded",        
								 "PixelShift 17 - GB Backlight Orange",       
								 "PixelShift 18 - GB Backlight White ",       
								 "PixelShift 19 - GB Backlight Yellow Dark",  
								 "PixelShift 20 - GB Bootleg",                
								 "PixelShift 21 - GB Hunter",                 
								 "PixelShift 22 - GB Kiosk",                  
								 "PixelShift 23 - GB Kiosk 2",                
								 "PixelShift 24 - GB New",                    
								 "PixelShift 25 - GB Nuked",                  
								 "PixelShift 26 - GB Old",                    
								 "PixelShift 27 - GBP Bivert",                
								 "PixelShift 28 - GB Washed Yellow Backlight",
								 "PixelShift 29 - Ghost",                     
								 "PixelShift 30 - Glow In The Dark",          
								 "PixelShift 31 - Gold Bar",                  
								 "PixelShift 32 - Grapefruit",                
								 "PixelShift 33 - Gray Green Mix",            
								 "PixelShift 34 - Missingno",                 
								 "PixelShift 35 - MS-Dos",                    
								 "PixelShift 36 - Newspaper",                 
								 "PixelShift 37 - Pip-Boy",                   
								 "PixelShift 38 - Pocket Girl",               
								 "PixelShift 39 - Silhouette",                
								 "PixelShift 40 - Sunburst",                  
								 "PixelShift 41 - Technicolor",               
								 "PixelShift 42 - Tron",                      
								 "PixelShift 43 - Vaporwave",                 
								 "PixelShift 44 - Virtual Boy",               
								 "PixelShift 45 - Wish"}; 



		int n_all_gambate_gc_colors_modes = 55;
		int n_twb1_colors_modes = 100;
		int n_twb2_colors_modes = 100;
		int n_twb3_colors_modes = 100;
		int n_pixelshift1_colors_modes = 45;

		for (int i = 0; i < n_all_gambate_gc_colors_modes; i++)
			colorizations_choices->add(all_gambate_gc_colors_modes[i], all_gambate_gc_colors_modes[i], currentColorization == std::string(all_gambate_gc_colors_modes[i]));
		
		for (int i = 0; i < n_twb1_colors_modes; i++)
			twb1_colorizations_choices->add(twb1_colors_modes[i], twb1_colors_modes[i], twb1_currentColorization == std::string(twb1_colors_modes[i]));
		
		for (int i = 0; i < n_twb2_colors_modes; i++)
			twb2_colorizations_choices->add(twb2_colors_modes[i], twb2_colors_modes[i], twb2_currentColorization == std::string(twb2_colors_modes[i]));

		for (int i = 0; i < n_twb3_colors_modes; i++)
			twb3_colorizations_choices->add(twb3_colors_modes[i], twb3_colors_modes[i], twb3_currentColorization == std::string(twb3_colors_modes[i]));
		
		for (int i = 0; i < n_pixelshift1_colors_modes; i++)
			pixelshift1_colorizations_choices->add(pixelshift1_colors_modes[i], pixelshift1_colors_modes[i], pixelshift1_currentColorization == std::string(pixelshift1_colors_modes[i]));

        if (SystemData::es_features_loaded || (!SystemData::es_features_loaded && (systemData->getName() == "gb" || systemData->getName() == "gbc" || systemData->getName() == "gb2players" || systemData->getName() == "gbc2players" || systemData->getName() == "gbh" || systemData->getName() == "gbch"))) // only for gb, gbc and gb2players gbh gbch
		{
			systemConfiguration->addWithLabel(_("COLORIZATION"), colorizations_choices);
			systemConfiguration->addWithLabel(_("TWB64 - PACK 1 PALETTE"), twb1_colorizations_choices);
			systemConfiguration->addWithLabel(_("TWB64 - PACK 2 PALETTE"), twb2_colorizations_choices);
			systemConfiguration->addWithLabel(_("TWB64 - PACK 3 PALETTE"), twb3_colorizations_choices);
			systemConfiguration->addWithLabel(_("PIXELSHIFT - PACK 1 PALETTE"), pixelshift1_colorizations_choices);
			systemConfiguration->addSaveFunc([colorizations_choices, configName] { SystemConf::getInstance()->set(configName + ".renderer.colorization", colorizations_choices->getSelected()); });
			systemConfiguration->addSaveFunc([twb1_colorizations_choices, configName] { SystemConf::getInstance()->set(configName + ".renderer.twb1_colorization", twb1_colorizations_choices->getSelected()); });
			systemConfiguration->addSaveFunc([twb2_colorizations_choices, configName] { SystemConf::getInstance()->set(configName + ".renderer.twb2_colorization", twb2_colorizations_choices->getSelected()); });
			systemConfiguration->addSaveFunc([twb3_colorizations_choices, configName] { SystemConf::getInstance()->set(configName + ".renderer.twb3_colorization", twb3_colorizations_choices->getSelected()); });
			systemConfiguration->addSaveFunc([pixelshift1_colorizations_choices, configName] { SystemConf::getInstance()->set(configName + ".renderer.pixelshift1_colorization", pixelshift1_colorizations_choices->getSelected()); });
		}
	}

	// Load per-game / per-emulator / per-system custom features
	std::vector<CustomFeature> customFeatures = systemData->getCustomFeatures(currentEmulator, currentCore);

	auto groups = groupBy(customFeatures, [](const CustomFeature& item) { return item.submenu; });
	for (auto group : groups)
	{
		if (!group.first.empty())
		{
			systemConfiguration->addEntry(group.first, true, [configName, mWindow, group]
			{
				GuiSettings* groupSettings = new GuiSettings(mWindow, _(group.first.c_str()));

				for (auto feat : group.second)
				{
					std::string storageName = configName + "." + feat.value;
					std::string storedValue = SystemConf::getInstance()->get(storageName);

					auto cf = std::make_shared<OptionListComponent<std::string>>(mWindow, _(feat.name.c_str()));
					cf->add(_("DEFAULT"), "", storedValue.empty() || storedValue == "default");

					for (auto fval : feat.choices)
						cf->add(_(fval.name.c_str()), fval.value, storedValue == fval.value);

					if (!cf->hasSelection())
						cf->selectFirstItem();

					if (!feat.description.empty())
						groupSettings->addWithDescription(_(feat.name.c_str()), _(feat.description.c_str()), cf);
					else
						groupSettings->addWithLabel(_(feat.name.c_str()), cf);

					groupSettings->addSaveFunc([cf, storageName]
					{
						SystemConf::getInstance()->set(storageName, cf->getSelected());
					});
				}

				mWindow->pushGui(groupSettings);
			});
		}
		else
		{
			for (auto feat : group.second)
	{
		std::string storageName = configName + "." + feat.value;
		std::string storedValue = SystemConf::getInstance()->get(storageName);

				auto cf = std::make_shared<OptionListComponent<std::string>>(mWindow, _(feat.name.c_str()));
				cf->add(_("DEFAULT"), "", storedValue.empty() || storedValue == "default");

				for (auto fval : feat.choices)
			cf->add(_(fval.name.c_str()), fval.value, storedValue == fval.value);

		if (!cf->hasSelection())
			cf->selectFirstItem();

		if (!feat.description.empty())
			systemConfiguration->addWithDescription(_(feat.name.c_str()), _(feat.description.c_str()), cf);
		else
			systemConfiguration->addWithLabel(_(feat.name.c_str()), cf);

		systemConfiguration->addSaveFunc([cf, storageName]
		{
			SystemConf::getInstance()->set(storageName, cf->getSelected());
		});
	}
		}
	}

	// automatic controller configuration
	if (systemData->isFeatureSupported(currentEmulator, currentCore, EmulatorFeatures::autocontrollers))
	{
		auto autoControllers = std::make_shared<OptionListComponent<std::string>>(mWindow, _("AUTOCONFIGURE CONTROLLERS"));
		autoControllers->addRange({ { _("DEFAULT"), "" },{ _("ON"), "0" },{ _("OFF"), "1" } }, SystemConf::getInstance()->get(configName + ".disableautocontrollers"));
		systemConfiguration->addWithLabel(_("AUTOCONFIGURE CONTROLLERS"), autoControllers);
		systemConfiguration->addSaveFunc([configName, autoControllers] { SystemConf::getInstance()->set(configName + ".disableautocontrollers", autoControllers->getSelected()); });
	}

	if (fileData == nullptr && ApiSystem::getInstance()->isScriptingSupported(ApiSystem::ScriptId::EVMAPY) && systemData->isCurrentFeatureSupported(EmulatorFeatures::Features::padTokeyboard))
	{
		if (systemData->hasKeyboardMapping())
			systemConfiguration->addEntry(_("EDIT PADTOKEY PROFILE"), true, [mWindow, systemData] { editKeyboardMappings(mWindow, systemData, true); });
		else
			systemConfiguration->addEntry(_("CREATE PADTOKEY PROFILE"), true, [mWindow, systemData] { editKeyboardMappings(mWindow, systemData, true); });
	}

	mWindow->pushGui(systemConfiguration);
}

std::shared_ptr<OptionListComponent<std::string>> GuiMenu::createRatioOptionList(Window *window, std::string configname)
{
	auto ratio_choice = std::make_shared<OptionListComponent<std::string> >(window, _("GAME ASPECT RATIO"), false);
	std::string currentRatio = SystemConf::getInstance()->get(configname + ".ratio");
	if (currentRatio.empty())
		currentRatio = std::string("default");

	std::map<std::string, std::string> *ratioMap = LibretroRatio::getInstance()->getRatio();
	for (auto ratio = ratioMap->begin(); ratio != ratioMap->end(); ratio++)
		ratio_choice->add(_(ratio->first.c_str()), ratio->second, currentRatio == ratio->second);

	if (!ratio_choice->hasSelection())
		ratio_choice->selectFirstItem();

	return ratio_choice;
}

std::shared_ptr<OptionListComponent<std::string>> GuiMenu::createVideoResolutionModeOptionList(Window *window, std::string configname)
{
	auto videoResolutionMode_choice = std::make_shared<OptionListComponent<std::string> >(window, _("VIDEO MODE"), false);

	std::string currentVideoMode = SystemConf::getInstance()->get(configname + ".videomode");
	if (currentVideoMode.empty())
		currentVideoMode = std::string("default");

	std::vector<std::string> videoResolutionModeMap = ApiSystem::getInstance()->getVideoModes();
	videoResolutionMode_choice->add(_("DEFAULT"), "default", currentVideoMode == "default");
	for (auto videoMode = videoResolutionModeMap.begin(); videoMode != videoResolutionModeMap.end(); videoMode++)
	{
		std::vector<std::string> tokens = Utils::String::split(*videoMode, ':');

		// concatenat the ending words
		std::string vname;
		for (unsigned int i = 1; i < tokens.size(); i++)
		{
			if (i > 1)
				vname += ":";

			vname += tokens.at(i);
		}

		videoResolutionMode_choice->add(vname, tokens.at(0), currentVideoMode == tokens.at(0));
	}

	if (!videoResolutionMode_choice->hasSelection())
		videoResolutionMode_choice->selectFirstItem();

	return videoResolutionMode_choice;
}

void GuiMenu::clearLoadedInput() {
  for(int i=0; i<mLoadedInput.size(); i++) {
    delete mLoadedInput[i];
  }
  mLoadedInput.clear();
}

GuiMenu::~GuiMenu() {
  clearLoadedInput();
}

std::vector<DecorationSetInfo> GuiMenu::getDecorationsSets(SystemData* system)
{
	std::vector<DecorationSetInfo> sets;
	if (system == nullptr)
		return sets;

	static const size_t pathCount = 3;


	std::vector<std::string> paths = {
		"/storage/roms/bezels",
		"/tmp/overlays/bezels"
	};

	Utils::FileSystem::stringList dirContent;
	std::string folder;

	for (auto path : paths)
	{
		if (!Utils::FileSystem::isDirectory(path))
			continue;

		dirContent = Utils::FileSystem::getDirContent(path);
		for (Utils::FileSystem::stringList::const_iterator it = dirContent.cbegin(); it != dirContent.cend(); ++it)
		{
			if (Utils::FileSystem::isDirectory(*it))
			{
				folder = *it;

				DecorationSetInfo info;
				info.name = folder.substr(path.size() + 1);
				info.path = folder;

				if (system != nullptr && Utils::String::startsWith(info.name, "default"))
				{
					std::string systemImg = path + "/"+ info.name +"/systems/" + system->getName() + ".png";
					if (Utils::FileSystem::exists(systemImg))
						info.imageUrl = systemImg;
				}

				if (info.imageUrl.empty())
				{
					std::string img = folder + "/default.png";
					if (Utils::FileSystem::exists(img))
						info.imageUrl = img;
				}

				sets.push_back(info);
			}
		}
	}

	struct { bool operator()(DecorationSetInfo& a, DecorationSetInfo& b) const { return a.name < b.name; } } compareByName;
	struct { bool operator()(DecorationSetInfo& a, DecorationSetInfo& b) const { return a.name == b.name; } } nameEquals;

	// sort and remove duplicates
	std::sort(sets.begin(), sets.end(), compareByName);
	sets.erase(std::unique(sets.begin(), sets.end(), nameEquals), sets.end());

	return sets;
}


void GuiMenu::openFormatDriveSettings()
{
	Window *window = mWindow;

	auto s = new GuiSettings(mWindow, _("FORMAT DEVICE").c_str());

	// Drive
	auto optionsStorage = std::make_shared<OptionListComponent<std::string> >(window, _("DEVICE TO FORMAT"), false);

	std::vector<std::string> disks = ApiSystem::getInstance()->getFormatDiskList();
	if (disks.size() == 0)
		optionsStorage->add(_("NONE"), "", false);
	else
	{
		for (auto disk : disks)
		{
			auto idx = disk.find(" ");
			if (idx != std::string::npos)
				optionsStorage->add(disk.substr(idx + 1), disk.substr(0, idx), false);
		}
	}

	optionsStorage->selectFirstItem();
	s->addWithLabel(_("DEVICE TO FORMAT"), optionsStorage);

	// File system
	auto fileSystem = std::make_shared<OptionListComponent<std::string> >(window, _("FILE SYSTEM"), false);

	std::vector<std::string> fileSystems = ApiSystem::getInstance()->getFormatFileSystems();
	if (fileSystems.size() == 0)
		fileSystem->add(_("NONE"), "", false);
	else
	{
		for (auto fs : fileSystems)
			fileSystem->add(fs, fs, false);
	}

	fileSystem->selectFirstItem();
	s->addWithLabel(_("FILE SYSTEM"), fileSystem);

	s->addEntry(_("FORMAT NOW"), false, [s, optionsStorage, fileSystem, window]
		{
			std::string disk = optionsStorage->getSelected();
			std::string fs = fileSystem->getSelected();

			if (disk.empty() || fs.empty())
			{
				window->pushGui(new GuiMsgBox(window, _("SELECTED OPTIONS ARE INVALID")));
				return;
			}

			window->pushGui(new GuiMsgBox(window, _("ARE YOU SURE YOU WANT TO FORMAT THIS DRIVE?"), _("YES"), [s, window, disk, fs]
			{
				ThreadedFormatter::start(window, disk, fs);
				s->close();
			}, _("NO"), nullptr));

		});

	mWindow->pushGui(s);
}



void GuiMenu::saveSubsetSettings()
{
	auto currentSystem = ViewController::get()->getState().getSystem();
	if (currentSystem == nullptr || currentSystem->getTheme() == nullptr)
		return;

	std::string fileData;

	auto subsets = currentSystem->getTheme()->getSubSetNames();
	for (auto subset : subsets)
	{
		std::string name = subset;
		std::string value;

		if (name == "colorset")
			value = Settings::getInstance()->getString("ThemeColorSet");
		else if (name == "iconset")
			value = Settings::getInstance()->getString("ThemeIconSet");
		else if (name == "menu")
			value = Settings::getInstance()->getString("ThemeMenu");
		else if (name == "systemview")
			value = Settings::getInstance()->getString("ThemeSystemView");
		else if (name == "gamelistview")
			value = Settings::getInstance()->getString("ThemeGamelistView");
		else if (name == "region")
			value = Settings::getInstance()->getString("ThemeRegionName");
		else
		{
			value = Settings::getInstance()->getString("subset." + name);
			name = "subset." + name;
		}

		if (!value.empty())
			fileData += name + "=" + value + "\r";

		for (auto system : SystemData::sSystemVector)
		{
			value = Settings::getInstance()->getString("subset." + system->getThemeFolder() + "." + subset);
			if (!value.empty())
				fileData += "subset." + system->getThemeFolder() + "." + subset + "=" + value + "\r";
		}
	}

	if (!Settings::getInstance()->getString("GamelistViewStyle").empty() && Settings::getInstance()->getString("GamelistViewStyle") != "automatic")
		fileData += "GamelistViewStyle=" + Settings::getInstance()->getString("GamelistViewStyle") + "\r";

	if (!Settings::getInstance()->getString("DefaultGridSize").empty())
		fileData += "DefaultGridSize=" + Settings::getInstance()->getString("DefaultGridSize") + "\r";

	for (auto system : SystemData::sSystemVector)
	{
		auto defaultView = Settings::getInstance()->getString(system->getName() + ".defaultView");
		if (!defaultView.empty())
			fileData += system->getName() + ".defaultView=" + defaultView + "\r";

		auto gridSizeOverride = Settings::getInstance()->getString(system->getName() + ".gridSize");
		if (!gridSizeOverride.empty())
			fileData += system->getName() + ".gridSize=" + gridSizeOverride + "\r";
	}

	std::string path = Utils::FileSystem::getEsConfigPath() + "/themesettings";
	if (!Utils::FileSystem::exists(path))
		Utils::FileSystem::createDirectory(path);

	std::string themeSet = Settings::getInstance()->getString("ThemeSet");
	std::string fileName = path + "/" + themeSet + ".cfg";

	if (fileData.empty())
	{
		if (Utils::FileSystem::exists(fileName))
			Utils::FileSystem::removeFile(fileName);
	}
	else
		Utils::FileSystem::writeAllText(fileName, fileData);

}

void GuiMenu::loadSubsetSettings(const std::string themeName)
{
	std::string path = Utils::FileSystem::getEsConfigPath() + "/themesettings";
	if (!Utils::FileSystem::exists(path))
		Utils::FileSystem::createDirectory(path);

	std::string fileName = path + "/" + themeName + ".cfg";
	if (!Utils::FileSystem::exists(fileName))
		return;

	std::string line;
	std::ifstream systemConf(fileName);
	if (systemConf && systemConf.is_open())
	{
		while (std::getline(systemConf, line, '\r'))
		{
			int idx = line.find("=");
			if (idx == std::string::npos || line.find("#") == 0 || line.find(";") == 0)
				continue;

			std::string name = line.substr(0, idx);
			std::string value = line.substr(idx + 1);
			if (!name.empty() && !value.empty())
			{
				if (name == "colorset")
					Settings::getInstance()->setString("ThemeColorSet", value);
				else if (name == "iconset")
					Settings::getInstance()->setString("ThemeIconSet", value);
				else if (name == "menu")
					Settings::getInstance()->setString("ThemeMenu", value);
				else if (name == "systemview")
					Settings::getInstance()->setString("ThemeSystemView", value);
				else if (name == "gamelistview")
					Settings::getInstance()->setString("ThemeGamelistView", value);
				else if (name == "region")
					Settings::getInstance()->setString("ThemeRegionName", value);
				else if (name == "GamelistViewStyle")
					Settings::getInstance()->setString("GamelistViewStyle", value);
				else if (name == "DefaultGridSize")
					Settings::getInstance()->setString("DefaultGridSize", value);
				else if (name.find(".defaultView") != std::string::npos)
					Settings::getInstance()->setString(name, value);
				else if (name.find(".gridSize") != std::string::npos)
					Settings::getInstance()->setString(name, value);
				else if (Utils::String::startsWith(name, "subset."))
					Settings::getInstance()->setString(name, value);
			}
		}
		systemConf.close();

		for (auto system : SystemData::sSystemVector)
		{
			auto defaultView = Settings::getInstance()->getString(system->getName() + ".defaultView");
			auto gridSizeOverride = Vector2f::parseString(Settings::getInstance()->getString(system->getName() + ".gridSize"));
			system->setSystemViewMode(defaultView, gridSizeOverride, false);
		}
	}
	else
		LOG(LogError) << "Unable to open " << fileName;
}

void GuiMenu::editKeyboardMappings(Window *window, IKeyboardMapContainer* mapping, bool editable)
{
	window->pushGui(new GuiKeyMappingEditor(window, mapping, editable));
}
