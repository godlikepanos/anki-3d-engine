#!/bin/bash
set -euo pipefail

# Fetches the MSVC CRT and the Windows SDK into ThirdParty/Xwin for Tools/Build/CrossCompileWindowsX64.cmake. Running it means accepting Microsoft's license

cd "$(dirname "$(readlink -f "$0")")/../.."

version=0.10.0
dir=ThirdParty/Xwin

mkdir -p $dir/bin
curl -sSfL "https://github.com/Jake-Shadle/xwin/releases/download/${version}/xwin-${version}-x86_64-unknown-linux-musl.tar.gz" \
	| tar xz -C $dir/bin --strip-components=1 "xwin-${version}-x86_64-unknown-linux-musl/xwin"

$dir/bin/xwin --accept-license --cache-dir $dir/cache --arch x86_64 --include-atl splat --use-winsysroot-style --preserve-ms-arch-notation --output $dir/sdk

rm -rf $dir/cache
