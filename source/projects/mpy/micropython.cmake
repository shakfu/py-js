include(FetchContent)

set(MP_INSTALL_DIR "${CMAKE_BINARY_DIR}/_deps")
set(MP_EMBEDDING "${MP_INSTALL_DIR}/micropython-src/examples/embedding")
set(EMBED_DIR "${MP_EMBEDDING}/micropython_embed")

FetchContent_Declare(
	micropython
	GIT_REPOSITORY https://github.com/micropython/micropython.git
	GIT_TAG b525f1c9ec8ffa9009754578932f3fad5f63026b
	GIT_SUBMODULES "docs" # hack to prevent recursive clone
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(
    micropython
)

if(NOT EXISTS ${EMBED_DIR})
	execute_process(
		COMMAND make -f micropython_embed.mk
		WORKING_DIRECTORY ${MP_EMBEDDING}
	)
endif()


set(MPY_EXE "${PROJECT_NAME}_exe")
set(MPY_LIB "${PROJECT_NAME}_lib")

set(MPY_COMPILE_OPTIONS
	-Wall 
	-Og
)

set(MPY_INCLUDE_DIRECTORIES
	${CMAKE_CURRENT_SOURCE_DIR}
	${MP_EMBEDDING}
	${EMBED_DIR}
	${EMBED_DIR}/extmod
	${EMBED_DIR}/genhdr
	${EMBED_DIR}/port
	${EMBED_DIR}/py
	${EMBED_DIR}/shared
)


set(MPY_SOURCE_FILES
	${EMBED_DIR}/port/embed_util.c
	${EMBED_DIR}/port/mphalport.c
	${EMBED_DIR}/py/argcheck.c
	${EMBED_DIR}/py/asmarm.c
	${EMBED_DIR}/py/asmbase.c
	${EMBED_DIR}/py/asmthumb.c
	${EMBED_DIR}/py/asmx64.c
	${EMBED_DIR}/py/asmx86.c
	${EMBED_DIR}/py/asmxtensa.c
	${EMBED_DIR}/py/bc.c
	${EMBED_DIR}/py/binary.c
	${EMBED_DIR}/py/builtinevex.c
	${EMBED_DIR}/py/builtinhelp.c
	${EMBED_DIR}/py/builtinimport.c
	${EMBED_DIR}/py/compile.c
	${EMBED_DIR}/py/emitbc.c
	${EMBED_DIR}/py/emitcommon.c
	${EMBED_DIR}/py/emitglue.c
	${EMBED_DIR}/py/emitinlinethumb.c
	${EMBED_DIR}/py/emitinlinextensa.c
	${EMBED_DIR}/py/emitnarm.c
	${EMBED_DIR}/py/emitnative.c
	${EMBED_DIR}/py/emitnthumb.c
	${EMBED_DIR}/py/emitnx64.c
	${EMBED_DIR}/py/emitnx86.c
	${EMBED_DIR}/py/emitnxtensa.c
	${EMBED_DIR}/py/emitnxtensawin.c
	${EMBED_DIR}/py/formatfloat.c
	${EMBED_DIR}/py/frozenmod.c
	${EMBED_DIR}/py/gc.c
	${EMBED_DIR}/py/lexer.c
	${EMBED_DIR}/py/malloc.c
	${EMBED_DIR}/py/map.c
	${EMBED_DIR}/py/modarray.c
	${EMBED_DIR}/py/modbuiltins.c
	${EMBED_DIR}/py/modcmath.c
	${EMBED_DIR}/py/modcollections.c
	${EMBED_DIR}/py/modgc.c
	${EMBED_DIR}/py/modio.c
	${EMBED_DIR}/py/modmath.c
	${EMBED_DIR}/py/modmicropython.c
	${EMBED_DIR}/py/modstruct.c
	${EMBED_DIR}/py/modsys.c
	${EMBED_DIR}/py/modthread.c
	${EMBED_DIR}/py/moduerrno.c
	${EMBED_DIR}/py/mpprint.c
	${EMBED_DIR}/py/mpstate.c
	${EMBED_DIR}/py/mpz.c
	${EMBED_DIR}/py/nativeglue.c
	${EMBED_DIR}/py/nlr.c
	${EMBED_DIR}/py/nlraarch64.c
	${EMBED_DIR}/py/nlrmips.c
	${EMBED_DIR}/py/nlrpowerpc.c
	${EMBED_DIR}/py/nlrsetjmp.c
	${EMBED_DIR}/py/nlrthumb.c
	${EMBED_DIR}/py/nlrx64.c
	${EMBED_DIR}/py/nlrx86.c
	${EMBED_DIR}/py/nlrxtensa.c
	${EMBED_DIR}/py/obj.c
	${EMBED_DIR}/py/objarray.c
	${EMBED_DIR}/py/objattrtuple.c
	${EMBED_DIR}/py/objbool.c
	${EMBED_DIR}/py/objboundmeth.c
	${EMBED_DIR}/py/objcell.c
	${EMBED_DIR}/py/objclosure.c
	${EMBED_DIR}/py/objcomplex.c
	${EMBED_DIR}/py/objdeque.c
	${EMBED_DIR}/py/objdict.c
	${EMBED_DIR}/py/objenumerate.c
	${EMBED_DIR}/py/objexcept.c
	${EMBED_DIR}/py/objfilter.c
	${EMBED_DIR}/py/objfloat.c
	${EMBED_DIR}/py/objfun.c
	${EMBED_DIR}/py/objgenerator.c
	${EMBED_DIR}/py/objgetitemiter.c
	${EMBED_DIR}/py/objint.c
	${EMBED_DIR}/py/objint_longlong.c
	${EMBED_DIR}/py/objint_mpz.c
	${EMBED_DIR}/py/objlist.c
	${EMBED_DIR}/py/objmap.c
	${EMBED_DIR}/py/objmodule.c
	${EMBED_DIR}/py/objnamedtuple.c
	${EMBED_DIR}/py/objnone.c
	${EMBED_DIR}/py/objobject.c
	${EMBED_DIR}/py/objpolyiter.c
	${EMBED_DIR}/py/objproperty.c
	${EMBED_DIR}/py/objrange.c
	${EMBED_DIR}/py/objreversed.c
	${EMBED_DIR}/py/objset.c
	${EMBED_DIR}/py/objsingleton.c
	${EMBED_DIR}/py/objslice.c
	${EMBED_DIR}/py/objstr.c
	${EMBED_DIR}/py/objstringio.c
	${EMBED_DIR}/py/objstrunicode.c
	${EMBED_DIR}/py/objtuple.c
	${EMBED_DIR}/py/objtype.c
	${EMBED_DIR}/py/objzip.c
	${EMBED_DIR}/py/opmethods.c
	${EMBED_DIR}/py/pairheap.c
	${EMBED_DIR}/py/parse.c
	${EMBED_DIR}/py/parsenum.c
	${EMBED_DIR}/py/parsenumbase.c
	${EMBED_DIR}/py/persistentcode.c
	${EMBED_DIR}/py/profile.c
	${EMBED_DIR}/py/pystack.c
	${EMBED_DIR}/py/qstr.c
	${EMBED_DIR}/py/reader.c
	${EMBED_DIR}/py/repl.c
	${EMBED_DIR}/py/ringbuf.c
	${EMBED_DIR}/py/runtime.c
	${EMBED_DIR}/py/runtime_utils.c
	${EMBED_DIR}/py/scheduler.c
	${EMBED_DIR}/py/scope.c
	${EMBED_DIR}/py/sequence.c
	${EMBED_DIR}/py/showbc.c
	${EMBED_DIR}/py/smallint.c
	${EMBED_DIR}/py/stackctrl.c
	${EMBED_DIR}/py/stream.c
	${EMBED_DIR}/py/unicode.c
	${EMBED_DIR}/py/vm.c
	${EMBED_DIR}/py/vstr.c
	${EMBED_DIR}/py/warning.c
	${EMBED_DIR}/port/embed_util.c
	${EMBED_DIR}/port/mphalport.c
	${EMBED_DIR}/py/argcheck.c
	${EMBED_DIR}/py/asmarm.c
	${EMBED_DIR}/py/asmbase.c
	${EMBED_DIR}/py/asmthumb.c
	${EMBED_DIR}/py/asmx64.c
	${EMBED_DIR}/py/asmx86.c
	${EMBED_DIR}/py/asmxtensa.c
	${EMBED_DIR}/py/bc.c
	${EMBED_DIR}/py/binary.c
	${EMBED_DIR}/py/builtinevex.c
	${EMBED_DIR}/py/builtinhelp.c
	${EMBED_DIR}/py/builtinimport.c
	${EMBED_DIR}/py/compile.c
	${EMBED_DIR}/py/emitbc.c
	${EMBED_DIR}/py/emitcommon.c
	${EMBED_DIR}/py/emitglue.c
	${EMBED_DIR}/py/emitinlinethumb.c
	${EMBED_DIR}/py/emitinlinextensa.c
	${EMBED_DIR}/py/emitnarm.c
	${EMBED_DIR}/py/emitnative.c
	${EMBED_DIR}/py/emitnthumb.c
	${EMBED_DIR}/py/emitnx64.c
	${EMBED_DIR}/py/emitnx86.c
	${EMBED_DIR}/py/emitnxtensa.c
	${EMBED_DIR}/py/emitnxtensawin.c
	${EMBED_DIR}/py/formatfloat.c
	${EMBED_DIR}/py/frozenmod.c
	${EMBED_DIR}/py/gc.c
	${EMBED_DIR}/py/lexer.c
	${EMBED_DIR}/py/malloc.c
	${EMBED_DIR}/py/map.c
	${EMBED_DIR}/py/modarray.c
	${EMBED_DIR}/py/modbuiltins.c
	${EMBED_DIR}/py/modcmath.c
	${EMBED_DIR}/py/modcollections.c
	${EMBED_DIR}/py/modgc.c
	${EMBED_DIR}/py/modio.c
	${EMBED_DIR}/py/modmath.c
	${EMBED_DIR}/py/modmicropython.c
	${EMBED_DIR}/py/modstruct.c
	${EMBED_DIR}/py/modsys.c
	${EMBED_DIR}/py/modthread.c
	${EMBED_DIR}/py/moduerrno.c
	${EMBED_DIR}/py/mpprint.c
	${EMBED_DIR}/py/mpstate.c
	${EMBED_DIR}/py/mpz.c
	${EMBED_DIR}/py/nativeglue.c
	${EMBED_DIR}/py/nlr.c
	${EMBED_DIR}/py/nlraarch64.c
	${EMBED_DIR}/py/nlrmips.c
	${EMBED_DIR}/py/nlrpowerpc.c
	${EMBED_DIR}/py/nlrsetjmp.c
	${EMBED_DIR}/py/nlrthumb.c
	${EMBED_DIR}/py/nlrx64.c
	${EMBED_DIR}/py/nlrx86.c
	${EMBED_DIR}/py/nlrxtensa.c
	${EMBED_DIR}/py/obj.c
	${EMBED_DIR}/py/objarray.c
	${EMBED_DIR}/py/objattrtuple.c
	${EMBED_DIR}/py/objbool.c
	${EMBED_DIR}/py/objboundmeth.c
	${EMBED_DIR}/py/objcell.c
	${EMBED_DIR}/py/objclosure.c
	${EMBED_DIR}/py/objcomplex.c
	${EMBED_DIR}/py/objdeque.c
	${EMBED_DIR}/py/objdict.c
	${EMBED_DIR}/py/objenumerate.c
	${EMBED_DIR}/py/objexcept.c
	${EMBED_DIR}/py/objfilter.c
	${EMBED_DIR}/py/objfloat.c
	${EMBED_DIR}/py/objfun.c
	${EMBED_DIR}/py/objgenerator.c
	${EMBED_DIR}/py/objgetitemiter.c
	${EMBED_DIR}/py/objint.c
	${EMBED_DIR}/py/objint_longlong.c
	${EMBED_DIR}/py/objint_mpz.c
	${EMBED_DIR}/py/objlist.c
	${EMBED_DIR}/py/objmap.c
	${EMBED_DIR}/py/objmodule.c
	${EMBED_DIR}/py/objnamedtuple.c
	${EMBED_DIR}/py/objnone.c
	${EMBED_DIR}/py/objobject.c
	${EMBED_DIR}/py/objpolyiter.c
	${EMBED_DIR}/py/objproperty.c
	${EMBED_DIR}/py/objrange.c
	${EMBED_DIR}/py/objreversed.c
	${EMBED_DIR}/py/objset.c
	${EMBED_DIR}/py/objsingleton.c
	${EMBED_DIR}/py/objslice.c
	${EMBED_DIR}/py/objstr.c
	${EMBED_DIR}/py/objstringio.c
	${EMBED_DIR}/py/objstrunicode.c
	${EMBED_DIR}/py/objtuple.c
	${EMBED_DIR}/py/objtype.c
	${EMBED_DIR}/py/objzip.c
	${EMBED_DIR}/py/opmethods.c
	${EMBED_DIR}/py/pairheap.c
	${EMBED_DIR}/py/parse.c
	${EMBED_DIR}/py/parsenum.c
	${EMBED_DIR}/py/parsenumbase.c
	${EMBED_DIR}/py/persistentcode.c
	${EMBED_DIR}/py/profile.c
	${EMBED_DIR}/py/pystack.c
	${EMBED_DIR}/py/qstr.c
	${EMBED_DIR}/py/reader.c
	${EMBED_DIR}/py/repl.c
	${EMBED_DIR}/py/ringbuf.c
	${EMBED_DIR}/py/runtime.c
	${EMBED_DIR}/py/runtime_utils.c
	${EMBED_DIR}/py/scheduler.c
	${EMBED_DIR}/py/scope.c
	${EMBED_DIR}/py/sequence.c
	${EMBED_DIR}/py/showbc.c
	${EMBED_DIR}/py/smallint.c
	${EMBED_DIR}/py/stackctrl.c
	${EMBED_DIR}/py/stream.c
	${EMBED_DIR}/py/unicode.c
	${EMBED_DIR}/py/vm.c
	${EMBED_DIR}/py/vstr.c
	${EMBED_DIR}/py/warning.c
	${EMBED_DIR}/shared/runtime/gchelper_generic.c
)


# defaults to building static-libs
# can build shared if specified here
# or -DBUILD_SHARED_LIBS option is set
add_library(${MPY_LIB} # SHARED
	${MPY_SOURCE_FILES}
)


target_include_directories(${MPY_LIB}
	PUBLIC
	${MPY_INCLUDE_DIRECTORIES}
)

target_compile_options(${MPY_LIB}
	PUBLIC
	${MPY_COMPILE_OPTIONS}
)
