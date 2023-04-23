Welcome, thanks for using my tool.
The RpfStreamer can be used to load every .rpf regardless of its loading state.

Copy everything besides this README.txt file into your RDR2 game directory and start the game.

How to create custom load configs:
1. Edit the RpfStreamer.ini file and set ShowConsole to true (Not required, but helps developing)
2. If not already created, create a folder called "rpfs" in your RDR2 game directory, put your custom configs in this folder.
3. Use any code editor (I prefer Visual Studio Code or VSCodium for the ones who want some privacy) and create a file with the extension: ".RpfStreamer.json"
4. An example config file can be found in the shipped "rpfs" folder called "online_content_example.RpfStreamer.json"
5. Make sure to always use the relative mounted path instead of your custom path.
6. The example is using dlc_mp00*, because this path has been mounted under that specific alias.

Requirements:
ScriptHookRDR2.dll
dinput8.dll or version.dll