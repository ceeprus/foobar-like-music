# foobar-like-music

Clickable like hearts for foobar2000. A like is a Playback Statistics rating.

## Default UI

Needs foobar2000 v2.0+ and [foo_playcount](https://www.foobar2000.org/components/view/foo_playcount).

Install `foo_likeheart.fb2k-component` from [Releases](../../releases) through File, Preferences, Components. Then right-click the playlist header and pick Columns, **Like**. Click a heart to toggle it, or bind Playback, **Toggle Like** to a key.

## NekoRoX-v2

[`nekorox/heart.patch`](nekorox/heart.patch) adds a red heart, Ctrl+L and a "by liked" sort to [NekoRoX-v2](https://gh.ali360.cloudns.ch/0xNikilite/NekoRoX-v2). Close foobar2000, back up `themes\NekoRoX-v2\Scripts\Panel_Playlist.js`, and run this with [Git for Windows](https://git-scm.com/download/win) from the folder that contains `themes` (as administrator under Program Files):

```
git -c core.autocrlf=false apply --ignore-whitespace C:\path\to\heart.patch
```

Then turn on **Show rating** (right-click the track list, Appearance, Rows). Add `-R` to the command to undo.

## Build

Extract the [foobar2000 SDK](https://www.foobar2000.org/SDK) into `sdk/` and build `src\foo_likeheart.vcxproj` with Visual Studio 2022 for Win32 and x64.

## Credits

NekoRoX-v2 by 0xNikilite, based on NekoRoX by catlinman and CaTRoX by eXtremeHunter1972. The playlist panel is by TheQwertiest.

MIT licensed. The NekoRoX-v2 code the patch changes keeps its authors' terms.
