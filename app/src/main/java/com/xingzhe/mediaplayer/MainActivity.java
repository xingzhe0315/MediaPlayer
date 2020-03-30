package com.xingzhe.mediaplayer;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ProgressBar;
import android.widget.SeekBar;
import android.widget.TextView;

import com.xingzhe.player.MediaPlayer;
import com.xingzhe.player.enums.SoundChannel;
import com.xingzhe.player.view.MediaPlayerView;

import java.io.File;

public class MainActivity extends AppCompatActivity {
    private MediaPlayer mediaPlayer;

    private TextView mSongNameTv;
    private CheckBox mPlayCb;
    private SeekBar mPlaySeekBar;
    private TextView mProgressTv;
    private TextView mTotalTimeTv;
    private SeekBar mVolumeSeekBar;
    private ProgressBar mLoadingBar;
    private int mProgress;
    private boolean isSeeking;
    private MediaPlayerView mPlayView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        mediaPlayer = new MediaPlayer();
        mediaPlayer.setOnStateChangedListener(new MediaPlayer.OnStateChangeListener() {
            @Override
            public void onPrepared() {
                Log.e("MainActivity", "----onPrepared");
                mediaPlayer.start();
                mPlayCb.setChecked(true);
            }

            @Override
            public void onLoading() {
                Log.e("MainActivity", "----onLoading");
            }

            @Override
            public void onPlaying() {
                Log.e("MainActivity", "----onPlaying");
            }

            @Override
            public void onFinished() {
                Log.e("MainActivity", "----onFinished");
            }
        });
        String path = new File(getExternalFilesDir(null), "第一滴血4.mp4").getAbsolutePath();
        path = new File(getExternalFilesDir(null), "20200105_160921.mp4").getAbsolutePath();
//        path = "http://mpge.5nd.com/2015/2015-11-26/69708/1.mp3";
        mediaPlayer.setDataSource(path);

        initViews();
    }

    private void initViews() {
        mPlayView = findViewById(R.id.player_view);
        mediaPlayer.setPlayerView(mPlayView);
        mLoadingBar = findViewById(R.id.loading_bar);
        mPlaySeekBar = findViewById(R.id.play_progress_seekbar);
        mProgressTv = findViewById(R.id.progress_tv);
        mTotalTimeTv = findViewById(R.id.total_time_tv);
        mVolumeSeekBar = findViewById(R.id.volume_seekbar);
        mVolumeSeekBar.setProgress(50);
        mPlayCb = findViewById(R.id.play_checkbox);
        mPlaySeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                if (fromUser) {
                    mProgress = progress;
                }
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {
                isSeeking = true;
            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                mediaPlayer.seekTo(mProgress);
                isSeeking = false;
            }
        });
        mVolumeSeekBar.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {
                mediaPlayer.setVolume(progress);
            }

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {

            }

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {

            }
        });

        mPlayCb.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    mediaPlayer.resume();
                } else {
                    mediaPlayer.pause();
                }
            }
        });
    }

    public void start(View view) {
        mediaPlayer.prepareAsync();
    }

    public void next(View view) {
    }

    public void stop(View view) {
        mediaPlayer.stop();
    }

    public void last(View view) {
    }

    public void setRightMute(View view) {
        mediaPlayer.setSoundChannel(SoundChannel.RIGHT);
    }

    public void setCenterMute(View view) {
        mediaPlayer.setSoundChannel(SoundChannel.CENTER);
    }

    public void setLeftMute(View view) {
        mediaPlayer.setSoundChannel(SoundChannel.LEFT);
    }
}
