#!/bin/bash
# Copyright (c) 2023 Google LLC.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# Linux Build Script.

# Fail on any error.
set -e
# Display commands being run.
set -x

function die {
  echo "$@"
  exit 1
}


# This is required to run any git command in the docker since owner will
# have changed between the clone environment, and the docker container.
# Marking the root of the repo as safe for ownership changes.
git config --global --add safe.directory $ROOT_DIR

. /bin/using.sh # Declare the bash `using` function for configuring toolchains.

using python-3.12

case $CONFIG in
  DEBUG|RELEASE)
    ;;
  *)
    die expected CONFIG to be DEBUG or RELEASE
    ;;
esac

case $COMPILER in
  clang)
    using clang-13.0.1
    ;;
  gcc)
    using gcc-13
    ;;
  *)
    die expected COMPILER to be clang or gcc
    ;;
esac

case $TOOL in
  cmake|bazel)
    ;;
  *)
    die expected TOOL to be cmake or bazel
    ;;
esac

cd $ROOT_DIR

function clean_dir() {
  dir=$1
  if [[ -d "$dir" ]]; then
    rm -fr "$dir"
  fi
  mkdir "$dir"
}

# Get source for dependencies, as specified in the DEPS file
/usr/bin/python3 utils/git-sync-deps

if [ $TOOL = "cmake" ]; then
  using cmake-3.31.2
  using ninja-1.10.0

  BUILD_TYPE="Debug"
  if [ $CONFIG = "RELEASE" ]; then
    BUILD_TYPE="RelWithDebInfo"
  fi

  clean_dir "$ROOT_DIR/build"
  cd "$ROOT_DIR/build"

  # Invoke the build.
  BUILD_SHA=${KOKORO_GITHUB_COMMIT:-$KOKORO_GITHUB_PULL_REQUEST_COMMIT}
  echo $(date): Starting build...
  cmake -DPYTHON_EXECUTABLE:FILEPATH=/usr/bin/python3 -GNinja -DCMAKE_INSTALL_PREFIX=$KOKORO_ARTIFACTS_DIR/install -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DRE2_BUILD_TESTING=OFF ..

  echo $(date): Build everything...
  ninja
  echo $(date): Build completed.

  echo $(date): Starting ctest...
  ctest --output-on-failure --timeout 300
  echo $(date): ctest completed.
elif [ $TOOL = "bazel" ]; then
  using bazel-7.0.2

  echo $(date): Build everything...
  bazel build --cxxopt=-std=c++17 :all
  echo $(date): Build completed.

  echo $(date): Starting bazel test...
  bazel test --cxxopt=-std=c++17 :all
  echo $(date): Bazel test completed.
fi
