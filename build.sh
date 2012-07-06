#!/bin/sh

# Setup environment:
.  ../thinj/config.sh || exit 1


make clean || exit 1
make || exit 1

cp libthinjvm.a $LIBDIR || exit 1

cp thinjvm.h constantpool.h operandstack.h jni.h instructions.h types.h config.h console.h frame.h heap.h $INCDIR

exit 0

