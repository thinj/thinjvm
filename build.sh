#!/bin/sh
DESTINATION=/tools/thinj/devel
LIBDIR=$DESTINATION/lib
BINDIR=$DESTINATION/bin
INCDIR=$DESTINATION/inc

mkdir -p $DESTINATION || exit 1
mkdir -p $LIBDIR || exit 1
mkdir -p $INCDIR || exit 1

make clean || exit 1
make || exit 1

cp libthinjvm.a $LIBDIR || exit 1

cp constantpool.h operandstack.h jni.h instructions.h types.h config.h console.h frame.h heap.h $INCDIR

exit 0

