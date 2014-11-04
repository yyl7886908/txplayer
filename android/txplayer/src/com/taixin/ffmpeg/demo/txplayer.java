package com.taixin.ffmpeg.demo;

import java.io.File;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;
import android.text.TextUtils;
import android.view.KeyEvent;
import android.view.View;

import com.taixin.ffmpeg.widget.MediaController;
import com.taixin.ffmpeg.widget.VideoView;

public class txplayer extends Activity
{
	private VideoView mVideoView;
	private View mBufferingIndicator;
	private MediaController mMediaController;

	private String mVideoPath;

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.main);

		//mVideoPath = "http://live.gslb.letv.com/gslb?stream_id=btv6_800&tag=live&ext=m3u8";
		//mVideoPath ="http://192.168.1.113:8080/video/liyue720p.mp4";
		//mVideoPath ="http://192.168.1.113:8080/video/bingheshiji202.mp4";
		//mVideoPath ="rtsp://admin:12345@192.168.1.106:554/Streaming/Channels/1?transportmode=unicast&profile=Profile_1";
		
		mVideoPath = "rtsp://admin:12345@192.168.1.105:554/Streaming/Channels/1?transportmode=unicast&profile=Profile_1";
		Intent intent = getIntent();
		String intentAction = intent.getAction();
		if (!TextUtils.isEmpty(intentAction)
				&& intentAction.equals(Intent.ACTION_VIEW)) {
			mVideoPath = intent.getDataString();
		}

		if (TextUtils.isEmpty(mVideoPath)) {
			mVideoPath = new File(Environment.getExternalStorageDirectory(),
					"download/test.mp4").getAbsolutePath();
		}

		mBufferingIndicator = findViewById(R.id.buffering_indicator);
		mMediaController = new MediaController(this);

		mVideoView = (VideoView) findViewById(R.id.video_view);
		mVideoView.setMediaController(mMediaController);
		mVideoView.setMediaBufferingIndicator(mBufferingIndicator);
		mVideoView.setVideoPath(mVideoPath);
		mVideoView.requestFocus();
		mVideoView.start();
	}

	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		 if (keyCode == KeyEvent.KEYCODE_DPAD_UP) {
			 System.out.println("向上建按住------");
			 mVideoView.photoImage("/storage/external_storage/sda1/photoTest");
		 }
		return super.onKeyDown(keyCode, event);
	}
	
}
