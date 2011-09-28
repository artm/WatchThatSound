# Watch That Sound workshops tool

## Description: TODO

## Building

### Windows

WTS3's official build environment is QtCreator + MinGW/MSYS.

- WTS3 is dynamically linked to Qt libraries and statically to portaudio / ffmpeg. Combining dynamic and static linking is broken in MinGW 4.4 that comes with QtCreator 2.3.0 (QtSDK 1.1.3). The problem is solved in MinGW 4.5, which is what we use.
- WTS3 build system uses `sed` and `mv` command line utilities that aren't supplied with the QtSDK. At the moment this means we need e.g. MSYS on Windows


#### Prerequisites

* We need **[K-Lite codecs](http://www.codecguide.com/klcp_update.htm)** to support the movies in original WTS format: Jpeg encoded QuickTime (.mov). An alternative solution would be to use avi on windows. It's better because doesn't require additional software installation.

* [ffmpeg](http://ffmpeg.org/download.html). I build against ffmpeg-8.4.0, but have to rename the ffmpeg sources directory to `ffmpeg`.

* [portaudio](http://www.portaudio.com/download.html) v19. I build against a stable snapshot from v19_20110326.

