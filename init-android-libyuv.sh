#! /usr/bin/env bash

TX_LIBYUV_UPSTREAM=https://github.com/yyl7886908/libyuv.git
TX_LIBYUV_FORK=https://github.com/yyl7886908/libyuv.git
TX_LIBYUV_COMMIT=master
TX_LIBYUV_LOCAL_REPO=extra/libyuv

set -e
TOOLS=tools

echo "== pull libyuv base =="
sh $TOOLS/pull-repo-base.sh $TX_LIBYUV_UPSTREAM $TX_LIBYUV_LOCAL_REPO

echo "== pull libyuv fork =="
sh $TOOLS/pull-repo-ref.sh $TX_LIBYUV_FORK txmedia/txyuv ${TX_LIBYUV_LOCAL_REPO}
cd txmedia/txyuv
git checkout ${TX_LIBYUV_COMMIT}
cd -
