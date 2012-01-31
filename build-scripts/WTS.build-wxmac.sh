#!/bin/sh

PREFIX=$(cd .. ; echo $(pwd)/wxMac)
echo Will install wxMac into $PREFIX

WXSRC=$(cd .. ; echo $(pwd)/wxMac-2.8.12)
if [ -x "$WXSRC/configure" ] ; then
  echo "Found wxMac sources in $WXSRC"
else
  echo "NOT Found wxMac sources in $WXSRC"
  exit 1
fi

MAKEFLAGS="-j2" # for my old dual core macbook pro

CONF_LINE="${WXSRC}/configure CC=gcc-4.0 CXX=g++-4.0 LD=g++-4.0 --prefix=${PREFIX} \
  --with-macosx-sdk=/Developer/SDKs/MacOSX10.5.sdk \
  --with-macosx-version-min=10.4 --disable-compat26 \
  --with-expat=builtin --with-zlib=builtin --with-regex=builtin \
  --enable-universal_binary=yes --enable-unicode=yes"

echo "Configure command line:\n\n $CONF_LINE \n"

if [ -n "$1" ]
then
   #Shared Release
   rm -rf bld-ShRel
   mkdir bld-ShRel
   cd bld-ShRel
   ${CONF_LINE} --enable-static=no --enable-shared=yes --enable-debug=no
   make clean
   make
   make install
   cd ..

   #Static Release
   rm -rf bld-StRel
   mkdir bld-StRel
   cd bld-StRel
   ${CONF_LINE} --enable-static=yes --enable-shared=no --enable-debug=no
   make clean
   make
   make install
   cd ..

   #Shared Debug
   rm -rf bld-ShDeb
   mkdir bld-ShDeb
   cd bld-ShDeb
   ${CONF_LINE} --enable-static=no --enable-shared=yes --enable-debug=yes
   make clean
   make
   make install
   cd ..
fi

#Static Debug
rm -rf bld-StDeb
mkdir bld-StDeb
cd bld-StDeb
${CONF_LINE} --enable-static=yes --enable-shared=no --enable-debug=yes
make clean
make
make install
cd ..

