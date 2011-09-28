# Watch That Sound workshops tool

## Description: TODO

## Building

### Windows

WTS3's official build environment is QtCreator + MinGW/MSYS.

 - WTS3 is dynamically linked to Qt libraries and statically to
   portaudio / ffmpeg. Combining dynamic and static linking is broken in
MinGW 4.4 that comes with QtCreator 2.3.0 (QtSDK 1.1.3). The problem is
solved in MinGW 4.5, which is what we use.

 - WTS3 build system uses `sed` and `mv` command line utilities that
   aren't supplied with the QtSDK. At the moment this means we need e.g.
MSYS on Windows

#### Prerequisites

- We need **[K-Lite codecs][k-lite]** to support the movies in original
  WTS format: Jpeg encoded QuickTime (.mov). An alternative solution
would be to use avi on windows. It's better because doesn't require
additional software installation.
  - this just needs to be installed that's all

- **[ffmpeg][]**. I build against ffmpeg-8.4.0, but have to rename the
  ffmpeg sources directory to `ffmpeg`.
  - 

- **[portaudio][]** v19. I build against a stable snapshot from
  v19_20110326.

[k-lite]: http://www.codecguide.com/klcp_update.htm
[ffmpeg]: http://ffmpeg.org/download.html
[portaudio]:http://www.portaudio.com/download.html

#### Source tree and building prerequisites

My source tree looks like:

- `WTS3` - sources root, contains prerequisites and our own sources
  - `portaudio`
  - `ffmpeg`
  - `WatchThatSound` - our repository clone, name doesn't really matter

I build the prerequisites in MSYS command prompt like:

    cd portaudio
    PATH=/bin:/mingw/bin ./configure --disable-shared --enable-static
    make
    cd ..

    cd ffmpeg
    PATH=/bin:/mingw/bin ./configure \
         --disable-doc \
         --disable-ffmpeg \
         --disable-ffplay \
         --disable-ffprobe \
         --disable-ffserver \
         --disable-avdevice \
         --disable-avfilter \
         --disable-network \
         --disable-pthreads \
         --disable-yasm
    make
    cd ..

We could probably disable more to speed up building, but we don't.

It is possible to build portaudio with alternative backend, e.g.
DirectSound. This requires different configure command:

    CFLAGS='-DWINVER=0x0501' PATH=/bin:/mingw/bin ./configure \
           --disable-shared --enable-static \
           --with-winapi=wmme,directx \
           --with-dxdir=../dx9mgw

The above command assumes that you have Direct x SDKfor mingw in
`dx9mgw` directory under the source root. The SDK can be downloaded from
[http://trent.gamblin.ca/dx/dx9mgw.zip](http://trent.gamblin.ca/dx/dx9mgw.zip)

#### Preparing the QtCreator

As mentioned above, current version of QtCreator (QtSDK 1.1.3 at the
moment of writing) comes with broken MinGW (todo: link to stackoverflow
discussion). To be able to use alternative MinGW we need to configure
QtCreator first.

1. Install MinGW 4.5 (did it already to build prerequisites)
2. Add it as available toolchain to QtCreator
3. Configure the project to use this toolchain

Further, it is necessary to make sure that MSYS tools are available in
the build environment: the path `c:\MinGW\msys\1.0\bin` has to be
prepended to the build PATH.


