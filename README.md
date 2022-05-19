## What is wine-wayland

Wine-wayland allows running DX9/DX11 and Vulkan games using pure Wayland and Wine/DXVK.

## Why wine-wayland

 * You are tired of dealing with X11 and don't care about launchers
 * You want to remove X11 related packages from your PC
 * You want to experience potentially faster and smoother gaming on Wayland
 * You are concerned about insecure X11 games that can spy on other apps running on X11
 * You want to replace all the lib32 packages with only one package, and still be able to play most 32-bit games

## Screenshot

![screenshot](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot.png "Screenshot")
![screenshot1](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot1.png "Screenshot1")

## Requirements

 * Archlinux or Manjaro
 * GPU with Vulkan and Wayland support, 2GB+ VRAM, AMD or Intel, Nvidia not tested
 * Mesa 20.1 or later with Wayland and Vulkan support
 * Weston based compositor (tested on wayward), wlroots based compositor (tested on sway)
 * SDL and Faudio
 
## Download

You can download the 64-bit only archlinux package from https://github.com/varmd/wine-wayland/releases. This version is automatically built via Github Actions. cd to download folder and install
    
    pacman -U wine-wayland*pkg*
    
## Download of 32-bit (optional, for 32-bit games)

You can download the optional 32-bit version from https://github.com/varmd/wine-wayland/releases. It is automatically built via Github Actions. Download both the 64-bit and 32-bit archlinux packages, cd to download folder and install
    
    pacman -U *wine-wayland*pkg* 

## Compile

download or clone from github, cd to zip directory

    makepkg
    pacman -U wine-wayland*pkg*


#### Compile 32-bit (optional, for 32-bit games)

In wine-wayland directory

    WINE_BUILD_32=1 makepkg
    pacman -U *wine-wayland*pkg*

## Using wineland launcher to run games

From command line (or using file manager)

    mkdir -p ~/.local/share/wineland/your-game
    mv YourGameFolder ~/.local/share/wineland/your-game/
    
"your-name" above should be lowercase, no spaces tag. For example, for Subnautica it would be subnautica. After, go to your launcher, click on the blue joystick icon. In the browser tab, click Edit below the card for your-game. Enter name for your game, YourGameFolder/game.exe for exe path. And -EpicPortal for game options (for EGS games). Set mangohud, fsync/esync, and other options if needed. See below screenshot for example of options for Subnautica.

![screenshot](https://raw.githubusercontent.com/varmd/wine-wayland/master/wineland/wineland-screenshot-2.png "Screenshot")

Afterwards, click Submit. Then click Launch.

You can obtain YourGameFolder from EGS, Steam or GOG. See the notes section below for links to command line downloaders and tools for these services.

For troubleshooting you can check the logs at YourGameFolder/log.log
  
## Using terminal to run games with wineland launcher

After setting up your game with the steps above, you can
also run your games from the terminal.

    wineland your-game
    
## Using terminal to run games without wineland launcher

    cd your-dir
    mkdir -p prefix/your-game
    cp -r YourGameFolder prefix/your-game/
   
Copy relevant 64-bit or 32-bit dxvk dlls to YourGameFolder or use winetricks.

Copy start-example.sh to your-dir and modify it for your-game, change your-game and YourGameFolder at the top of the file.

Rename start-example.sh to start-your-game.sh

Then in the terminal run sh start-your-game.sh

### Environment variables when running games without the wineland launcher

* Use `export LD_LIBRARY_PATH="/usr/lib/wineland/lib32:$LD_LIBRARY_PATH"` and `export VK_ICD_FILENAMES="/usr/lib/wineland/vulkan/icd.d/intel_icd.i686.json:/usr/lib/wineland/vulkan/icd.d/radeon_icd.i686.json"` when running 32-bit wine apps outside of the wineland launcher
* If a game is not starting, or there is no keyboard/mouse focus, try `export WINE_VK_VULKAN_ONLY=1` variable and start the game to see if there are any error popups
* Use `export XCURSOR_SIZE="xx"` and `export XCURSOR_THEME=themename` to set cursor theme and increase cursor size 
* Use `export WINE_VK_HIDE_CURSOR=1` to hide cursors, when games do not hide cursors - for example when using a controller
* Use `export WINE_VK_USE_CUSTOM_CURSORS=1` to enable experimental custom game cursors. This will disable cursor size and theme
* Use `export WINE_VK_NO_CLIP_CURSOR=1` to disable cursor locking for games that erroneously try to lock mouse cursor.
* Use `export WINE_VK_FULLSCREEN_GRAB_CURSOR=1` to automatically enable cursor grab in fullscreen.
* Use `export WINE_VK_ALWAYS_FULLSCREEN=1` to automatically set game to fullscreen without using F11.
* For best performance use kernel with the fsync patch, and add `export WINEFSYNC=1` variable
* Use `export WINE_VK_HIDE_CURSOR=1` to hide mouse cursor - for example, when only using a controller

## Keyboard shortcuts

* F11 - Enter fullscreen mode
* F10 - some games may not restrict cursor properly, manually restricts cursor to the game surface. 
* F9 - some games (such as NMS) that draw their own cursor may need this to lock the cursor pointer.


## Notes

* Some Unity games require the game folder to be executable
* While launchers are not working many games do not require launchers to run
* You can use https://github.com/derrod/legendary to download and run games from Epic Games Store
* You can use https://github.com/ValvePython/steamctl to download games from Steam
* GOG games can be extracted with innoextract
* If a game is not starting, try wineserver -k, and start again, or click Launch again in the wineland launcher


## Caveats and issues

* No GDI apps support - though popups and simple launchers may work
* No OpenGL

## Games confirmed working

* ABZU
* Dirt 3
* Subnautica
* Rebel Galaxy
* Endless Space (add -force-d3d11 in Exe options)
* Age of Wonders 3
* Stellaris
* EU4
* Path of Exile
* Pathfinder Kingmaker
* Crusader Kings 2
* Mutant Year Zero
* Tropico 6
* Wasteland 2
* Torchlight 1
* Dungeons 3
* Seven
* Pillars of Eternity
* Ziggurat 1 (add -force-d3d11 in Exe options)
* Warframe (see #25)