# RpfStreamer
A tool to stream .rpf files without breaking game functionality, require file replacements, and/or other mod loaders. </br>
It works by calling two functions of RAGE to load individual rpf's:
- rage::strPackfileManager::AddImageToList
- rage::strPackfileManager::LoadRpf

# How to use:
- Copy RpfStreamer.asi, ScriptHookRDR2.dll, and dinput8.dll into your RDR2 installation folder.
- Create a folder with the name **rpfs** in your RDR2 installation folder and copy every **.RpfStreamer.json** file into the **rpfs** folder.

# Example file structure:
```json
{
    "dlc_mp003:":
    [
        {
            "x64":
            [
                {
                    "anim":
                    [
                        {
                            "ingame":
                            [
                                "clip_mp_mech_inspection.rpf",
                                "clip_mp_mech_skin.rpf",
                                "clip_mp_mini_games.rpf"
                            ]
                        }
                    ],
                    "packs":
                    [
                        {
                            "base":
                            [
                                {
                                    "models":
                                    [
                                        "metaped_textures_m.rpf",
                                        "metaped_textures_c.rpf",
                                        "metapeds_c.rpf",
                                        "weapons.rpf"
                                    ]
                                }
                            ]
                        }
                    ]
                }
            ]
        }
    ]
}
```
