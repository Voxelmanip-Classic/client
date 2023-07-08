CORE_GIT=https://github.com/rollerozxa/rollertest
CORE_BRANCH=master
CORE_NAME=minetest

libpng_version=1.6.39
ogg_version=1.3.5
openal_version=1.23.0
vorbis_version=1.3.7
freetype_version=2.12.1
luajit_version=20230221
zlib_version=1.2.13
zstd_version=1.5.5

download () {
	local url=$1
	local filename=$2
	[ -z "$filename" ] && filename=${url##*/}
	local foldername=${filename%%[.-]*}
	local extract=${3:-unzip}

	[ -d "./$foldername" ] && return 0
	wget "$url" -c -O "./$filename"
	if [ "$extract" = "unzip" ]; then
		unzip -o "$filename" -d "$foldername"
	elif [ "$extract" = "unzip_nofolder" ]; then
		unzip -o "$filename"
	else
		return 1
	fi
}

# sets $sourcedir
get_sources () {
	if [ -n "$EXISTING_MINETEST_DIR" ]; then
		sourcedir="$( cd "$EXISTING_MINETEST_DIR" && pwd )"
		return
	fi
	cd $builddir
	sourcedir=$PWD/$CORE_NAME
	[ -d $CORE_NAME ] && { pushd $CORE_NAME; git pull --ff-only; popd; } || \
		git clone -b $CORE_BRANCH $CORE_GIT $CORE_NAME
}

# sets $runtime_dlls
find_runtime_dlls () {
	local triple=$1
	# Try to find runtime DLLs in various paths (varies by distribution, sigh)
	local tmp=$(dirname "$(command -v $compiler)")/..
	runtime_dlls=
	for name in lib{gcc_,stdc++-,winpthread-}'*'.dll; do
		for dir in $tmp/$triple/{bin,lib} $tmp/lib/gcc/$triple/*; do
			[ -d "$dir" ] || continue
			local file=$(echo $dir/$name)
			[ -f "$file" ] && { runtime_dlls+="$file;"; break; }
		done
	done
	if [ -z "$runtime_dlls" ]; then
		echo "The compiler runtime DLLs could not be found, they might be missing in the final package."
	else
		echo "Found DLLs: $runtime_dlls"
	fi
}

add_cmake_libs () {
	local irr_dlls=$(echo $libdir/irrlicht/lib/*.dll | tr ' ' ';')
	local vorbis_dlls=$(echo $libdir/libvorbis/bin/libvorbis{,file}-*.dll | tr ' ' ';')
	local gettext_dlls=$(echo $libdir/gettext/bin/lib{intl,iconv}-*.dll | tr ' ' ';')

	cmake_args+=(
		-DPNG_LIBRARY=$libdir/libpng/lib/libpng.dll.a
		-DPNG_PNG_INCLUDE_DIR=$libdir/libpng/include
		-DPNG_DLL=$libdir/libpng/bin/libpng16.dll

		-DZLIB_INCLUDE_DIR=$libdir/zlib/include
		-DZLIB_LIBRARY=$libdir/zlib/lib/libz.dll.a
		-DZLIB_DLL=$libdir/zlib/bin/zlib1.dll

		-DZSTD_INCLUDE_DIR=$libdir/zstd/include
		-DZSTD_LIBRARY=$libdir/zstd/lib/libzstd.dll.a
		-DZSTD_DLL=$libdir/zstd/bin/libzstd.dll

		-DLUA_INCLUDE_DIR=$libdir/luajit/include
		-DLUA_LIBRARY=$libdir/luajit/libluajit.a

		-DOGG_INCLUDE_DIR=$libdir/libogg/include
		-DOGG_LIBRARY=$libdir/libogg/lib/libogg.dll.a
		-DOGG_DLL=$libdir/libogg/bin/libogg-0.dll

		-DVORBIS_INCLUDE_DIR=$libdir/libvorbis/include
		-DVORBIS_LIBRARY=$libdir/libvorbis/lib/libvorbis.dll.a
		-DVORBIS_DLL="$vorbis_dlls"
		-DVORBISFILE_LIBRARY=$libdir/libvorbis/lib/libvorbisfile.dll.a

		-DOPENAL_INCLUDE_DIR=$libdir/openal/include/AL
		-DOPENAL_LIBRARY=$libdir/openal/lib/libOpenAL32.dll.a
		-DOPENAL_DLL=$libdir/openal/bin/OpenAL32.dll

		-DFREETYPE_INCLUDE_DIR_freetype2=$libdir/freetype/include/freetype2
		-DFREETYPE_INCLUDE_DIR_ft2build=$libdir/freetype/include/freetype2
		-DFREETYPE_LIBRARY=$libdir/freetype/lib/libfreetype.dll.a
		-DFREETYPE_DLL=$libdir/freetype/bin/libfreetype-6.dll
	)
}
