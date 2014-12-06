#! /usr/bin/env bash

TX_FFMPEG_UPSTREAM=https://github.com/yyl7886908/ffmpeg.git
TX_FFMPEG_FORK=https://github.com/yyl7886908/ffmpeg.git
TX_FFMPEG_LOCAL_REPO=extra/ffmpeg


set -e
TOOLS=tools

# echo "== pull gas-preprocessor base =="
# sh $TOOLS/pull-repo-base.sh git://git.libav.org/gas-preprocessor.git extra/gas-preprocessor

echo "== pull ffmpeg base =="
sh $TOOLS/pull-repo-base.sh $TX_FFMPEG_UPSTREAM $TX_FFMPEG_LOCAL_REPO

function pull_fork()
{
    echo "== pull ffmpeg fork $1 =="
    sh $TOOLS/pull-repo-ref.sh $TX_FFMPEG_FORK ios/ffmpeg-$1 ${TX_FFMPEG_LOCAL_REPO}
   
}

pull_fork "armv7"
pull_fork "armv7s"
pull_fork "arm64"
pull_fork "i386"
pull_fork "x86_64"
