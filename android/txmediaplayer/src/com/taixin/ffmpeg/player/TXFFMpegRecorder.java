package com.taixin.ffmpeg.player;

public class TXFFMpegRecorder implements ITxRecorder{


	private native void _startRecoderRTSPStream(String rtspStream, String filename,int time);

	private native void _stopRecordRTSPStream();

	static{
		System.loadLibrary("stlport_shared");
		System.loadLibrary("txffmpeg");
		System.loadLibrary("txutil");
		System.loadLibrary("txrecorder");
	}

	@Override
	public void startRecoderRTSPStream(String rtspStream, String filename,
			int time) {
		_startRecoderRTSPStream(rtspStream, filename,time);
	}

	@Override
	public void stopRecordRTSPStream() {
		_stopRecordRTSPStream();
	}
}
