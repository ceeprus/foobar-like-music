# foobar-like-music

Clickable like hearts for foobar2000. Click ♡ to like a track, click ♥ to unlike it. A like is a Playback Statistics rating: liking sets 5, unliking clears it.

## Default UI component

Needs foobar2000 v2.0 or newer and the [Playback Statistics](https://www.foobar2000.org/components/view/foo_playcount) component (foo_playcount).

1. Download `foo_likeheart.fb2k-component` from [Releases](../../releases).
2. In foobar2000, open File, Preferences, Components, click Install... and pick the file, then restart.
3. Right-click the playlist's column header, open Columns and pick **Like**.

Click a heart to toggle it. Playback, **Toggle Like** does the same for the selected tracks and can be bound to a key.

## NekoRoX-v2 patch

[`nekorox/heart.patch`](nekorox/heart.patch) gives the [NekoRoX-v2](https://gh.ali360.cloudns.ch/0xNikilite/NekoRoX-v2) playlist a red clickable heart, Ctrl+L to like the selected rows, and Sort, **by liked**, which keeps liked tracks on top.

You need [Git for Windows](https://git-scm.com/download/win). Close foobar2000, open a terminal in the folder that contains `themes\NekoRoX-v2` (as administrator if it is under Program Files), back up `themes\NekoRoX-v2\Scripts\Panel_Playlist.js`, then run:

```
git -c core.autocrlf=false apply --ignore-whitespace C:\path\to\heart.patch
```

Start foobar2000 and turn on **Show rating** under Appearance, Rows in the track list's right-click menu. To undo, run the same command with `-R`.

## Build

Extract the [foobar2000 SDK](https://www.foobar2000.org/SDK) into `sdk/`, then build with Visual Studio 2022:

```
msbuild src\foo_likeheart.vcxproj /p:Configuration=Release /p:Platform=Win32
msbuild src\foo_likeheart.vcxproj /p:Configuration=Release /p:Platform=x64
```

## Credits

NekoRoX-v2 by [0xNikilite](https://gh.ali360.cloudns.ch/0xNikilite/NekoRoX-v2), based on [NekoRoX](https://gh.ali360.cloudns.ch/catlinman/foobar2000) by catlinman and CaTRoX by eXtremeHunter1972. The playlist panel is by TheQwertiest.

## License

MIT. The foobar2000 SDK and the NekoRoX-v2 code the patch changes keep their own terms.
