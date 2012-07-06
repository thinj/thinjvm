LIBNAME=thinjvm

IDIR =.
#CFLAGS=-I$(IDIR) -Wall -pg
#CFLAGS=-I$(IDIR) -Wall

ODIR=obj
LDIR =../lib

LIBS=-lm

_DEPS1=config.h console.h constantpool.h debugger.h disassembler.h frame.h heap.h instructions.h
_DEPS2=jni.h operandstack.h trace.h types.h xyprintf.h

_DEPS = $(_DEPS1) $(_DEPS2)
DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ1=console.o constantpool.o debugger.o disassembler.o frame.o
_OBJ2=heap.o instruction1.o jni.o $(NOSTDLIB) operandstack.o thinjvm.o trace.o xyprintf.o
_OBJ3=Java_java_lang_Object.o Java_java_lang_Throwable.o
_OBJ4=Java_thinj_regression_ReverseNativeTest.o
_OBJ5=Java_java_io_PrintStream.o Java_java_lang_Class.o

_OBJ = $(_OBJ1) $(_OBJ2) $(_OBJ3) $(_OBJ4) $(_OBJ5) 
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: %.c $(DEPS)
	@mkdir -p $(dir $@)
	@$(GCC) -o $@ $< $(CFLAGS)
#	@$(CC) -c -o $@ $< $(CFLAGS)

#	$(CC) $(CCOPTS) $< -o $@


lib: $(OBJ)
	@$(AR) crs lib$(LIBNAME).a $(OBJ)

.PHONY: clean

clean:
	@rm -f $(ODIR)/*.o lib*.a 
