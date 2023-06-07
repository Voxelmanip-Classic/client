#!/bin/bash -e

# Linux build only
install_linux_deps() {
	local pkgs=(
		cmake
		libpng-dev libjpeg-dev libxi-dev libgl1-mesa-dev
		libogg-dev libgmp-dev libvorbis-dev
		libopenal-dev libzstd-dev
	)

	sudo apt-get update
	sudo apt-get install -y --no-install-recommends "${pkgs[@]}" "$@"
}
