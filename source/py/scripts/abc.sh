VERSION=3.8
VER=38
NAME=python-min
MAC_DEP_TARGET=10.13

PREFIX=`pwd`/../${NAME}
LIB=${PREFIX}/lib/python${VERSION}


remove() {
	echo "removing $1"
	rm -rf $1
}

rm_lib() {
	echo "removing $1"
	rm -rf ${LIB}/$1
}


rm_ext() {
	echo "removing $LIB/lib-dynload/$1.cpython-${VER}m-darwin.so"
	rm -rf $LIB/lib-dynload/$1.cpython-${VER}-darwin.so
}

rm_bin() {
	echo "removing $PREFIX/bin/$1"
	rm -rf $PREFIX/bin/$1
}


clean_python_cruft() {
	find ${PREFIX} | grep -E "(__pycache__|\.pyc|\.pyo$)" | xargs rm -rf
	find ${PREFIX} | grep -E "(tests|test)" | xargs rm -rf
}


clean_python_site_packages() {
	echo "removing everything in $LIB/site-packages"
	rm -rf $LIB/site-packages/*
}

clean_python() {
	clean_python_cruft
	clean_python_site_packages

	#remove $LIB/distutils/command/*.exe
	remove $PREFIX/lib/pkgconfig
	remove $PREFIX/share

	rm_lib config-${VERSION}-darwin
	rm_lib idlelib
	rm_lib lib2to3
	rm_lib tkinter
	rm_lib turtledemo
	rm_lib turtle.py
	rm_lib ctypes
	rm_lib curses
	rm_lib ensurepip
	rm_lib venv

	rm_lib distutils
	rm_lib gettext.py
	rm_lib crypt.py
	rm_lib cgi.py
	rm_lib cgitb.py
	rm_lib pydoc
	rm_lib pydoc_data
	rm_lib sunau
	rm_lib sqlite3

	# rm_ext _tkinter
	# rm_ext _ctypes

	# rm_ext _multibytecodec
	# rm_ext _codecs_jp
	# rm_ext _codecs_hk
	# rm_ext _codecs_cn
	# rm_ext _codecs_kr
	# rm_ext _codecs_tw
	# rm_ext _codecs_iso2022
	# rm_ext _curses
	# rm_ext _curses_panel

	rm_bin 2to3-${VERSION}
	rm_bin idle${VERSION}
	rm_bin easy_install-${VERSION}
	rm_bin pip${VERSION}
	rm_bin python${VERSION}m
	rm_bin pyvenv-${VERSION}
	rm_bin pydoc${VERSION}
}

zip_python_library() {
	remove ${LIB}/site-packages
	mv $LIB/lib-dynload $PREFIX
	cp $LIB/os.py $PREFIX
	python -m zipfile -c $PREFIX/lib/python${VER}.zip $LIB/*
	remove $LIB
	mkdir -p $LIB
	mv $PREFIX/lib-dynload $LIB
	 # Need to have a copy of os.py to remain in site-packages
	 # or it will fail to pick up library.zip
	mv $PREFIX/os.py $LIB
	mkdir $LIB/site-packages
}

build_python() {
	MACOSX_DEPLOYMENT_TARGET=$MAC_DEP_TARGET ./configure \
	    --prefix=${PREFIX} \
	    --without-doc-strings \
	    --enable-ipv6 \
	    --without-ensurepip \
	    && make altinstall &> build.log
}

prepare_python() {
	remove $PREFIX
	build_python
	clean_python
	zip_python_library
}

prepare_python

