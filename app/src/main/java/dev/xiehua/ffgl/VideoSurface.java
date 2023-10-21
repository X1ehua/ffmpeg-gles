package dev.xiehua.ffgl;

import android.content.Context;
import android.util.Log;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceView;

public class VideoSurface extends SurfaceView implements SurfaceHolder.Callback {
	private static final String TAG = "VideoSurface";

	static {
		try {
			//System.loadLibrary("ffmpeg");
			System.loadLibrary("videosurface");
			Log.i(TAG, "native libVideoSurface.so loaded");
		}
		catch (Exception e) {
			Log.e(TAG, "load native .so failed: " + e.toString());
		}
	}

	public VideoSurface(Context context) {
		super(context);
		Log.v(TAG, "VideoSurface");
		getHolder().addCallback(this);
	}

	@Override
	public void surfaceChanged(SurfaceHolder holder, int format, int width,
			int height) {
		Log.v(TAG, "surfaceChanged, format is " + format + ", width is "
				+ width + ", height is" + height);
		//nativeStopPlayer();
		//setSurface(holder.getSurface());
	}

	@Override
	public void surfaceCreated(SurfaceHolder holder) {
		Log.v(TAG, "surfaceCreated");
		setSurface(holder.getSurface(), "http://mozicode.com/10s.mp4");
	}

	@Override
	public void surfaceDestroyed(SurfaceHolder holder) {
		Log.v(TAG, "surfaceDestroyed");
		//nativeStopPlayer();
	}

	public int pausePlayer() {
		return nativePausePlayer();
	}

	public int resumePlayer() {
		return nativeResumePlayer();
	}

	public int stopPlayer() {
		return nativeStopPlayer();
	}

	public native int setSurface(Surface view, String videoUri);

	public native int nativePausePlayer();

	public native int nativeResumePlayer();

	public native int nativeStopPlayer();
}
