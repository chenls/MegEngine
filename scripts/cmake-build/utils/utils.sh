#!/usr/bin/env bash
set -e

OS=$(uname -s)
NINJA_CMD=""
NINJA_BASE="ninja"

if [[ $OS =~ "NT" ]]; then
    echo "BUILD in NT ..."
    NINJA_BASE="Ninja"
fi

READLINK=readlink
if [ $OS = "Darwin" ];then
    READLINK=greadlink
fi

function cd_real_build_dir() {
    REAL_DIR=$($READLINK -f $1)
    echo "may alias dir: $1"
    echo "cd real build dir: ${REAL_DIR}"
    cd ${REAL_DIR}
}

function build_flatc() {
    BUILD_DIR=$1/build_dir/host_flatc/build
    INSTALL_DIR=$BUILD_DIR/../install
    REMOVE_OLD_BUILD=$2

    if [ $REMOVE_OLD_BUILD = "true" ]; then
        echo "remove old build/install dir"
        rm -rf $INSTALL_DIR
        rm -rf $BUILD_DIR
    else
        echo "strip remove old build"
    fi

    mkdir -p $BUILD_DIR
    mkdir -p $INSTALL_DIR

    cd_real_build_dir $BUILD_DIR
    cmake -G Ninja \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
        -DFLATBUFFERS_BUILD_TESTS=OFF \
        -DFLATBUFFERS_BUILD_FLATHASH=OFF \
        -DFLATBUFFERS_BUILD_FLATLIB=OFF \
        -DFLATBUFFERS_LIBCXX_WITH_CLANG=OFF \
        $SRC_DIR/third_party/flatbuffers

    ${NINJA_BASE} all
    ${NINJA_BASE} install/strip
}

function try_remove_old_build() {
    REMOVE_OLD_BUILD=$1
    echo $REMOVE_OLD_BUILD
    BUILD_DIR=$2
    INSTALL_DIR=$3

    if [ $REMOVE_OLD_BUILD = "true" ]; then
        echo "remove old build/install dir"
        rm -rf ${BUILD_DIR}
        rm -rf ${INSTALL_DIR}
    else
        echo "strip remove old build"
    fi
}

function config_ninja_target_cmd() {
    NINJA_CMD="${NINJA_BASE} all"
    if [ $# -eq 4 ]; then
        _NINJA_VERBOSE=$1
        _BUILD_DEVELOP=$2
        _INSTALL_ALL_TARGET=$3
        _NINJA_DRY_RUN=$4
    else
        echo "err call config_ninja_target_cmd"
        exit -1
    fi

    if [ ${_NINJA_DRY_RUN} = "ON" ]; then
        NINJA_CMD="${NINJA_CMD} -d explain -n"
    else
        if [ ${_NINJA_VERBOSE} = "ON" ]; then
            NINJA_CMD="${NINJA_CMD} -d explain -v"
        fi
        if [ ${_BUILD_DEVELOP} = "ON" ]; then
            echo "add develop target"
            NINJA_CMD="${NINJA_CMD} && ${NINJA_BASE} develop"
        fi
        if [ -n "${_INSTALL_ALL_TARGET}" ]; then
            NINJA_CMD="${NINJA_CMD} && ${NINJA_BASE} ${_INSTALL_ALL_TARGET}"
        fi
    fi

    echo "build ${NINJA_BASE} target command: ${NINJA_CMD}"
}
