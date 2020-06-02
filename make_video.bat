@ffmpeg -r 60 -y -f image2 -i frames\%%06d.bmp -s 1920x1080 -b:v 32000k video_noaudio.mp4
@ffmpeg -y -i video_noaudio.mp4 -i lug00ber-wavetracker.mp3 -codec copy video_audio.mp4