#!/bin/bash
rm -rf  extra/
cd txmedia/
rm -rf txyuv/
cd -

cd android/

cd txmediaplayer/jni/

ndk-build clean

cd -

rm -rf build/
rm -rf ffmpeg*

cd ../


