package com.taixin.ffmpeg.player;

public interface ITxRecorder {

	public void startRecoderRTSPStream(String rtspStream, String filename,int time);

	public void stopRecordRTSPStream();
}
