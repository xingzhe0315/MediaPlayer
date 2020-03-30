#include <jni.h>
#include <string>
#include "MediaPlayer.h"
#include "log_define.h"

#define TAG "Media_Player_JNI"

JavaVM *javaVm;

struct AudioPlayerJava {
    jmethodID postEvent;
    jmethodID updateTime;
    jmethodID yuvCallbackMid;
} audioPlayerJava;

void callJavaNotify(JNIEnv *env, jobject target, int what) {
    env->CallVoidMethod(target, audioPlayerJava.postEvent, what);
}

class JNIAudioPlayerListener : public MediaPlayerListener {
public:
    JNIAudioPlayerListener(JNIEnv *env, jobject obj);

    ~JNIAudioPlayerListener();

    void notify(int what, int thread) override;

    void updateTime(int duration, double progress, int thread) override;

    void onYUVCallback(int width, int height, uint8_t *yBuffer, uint8_t *uBuffer,
                       uint8_t *vBuffer) override;

private:
    jobject mTarget;
};

JNIAudioPlayerListener::JNIAudioPlayerListener(JNIEnv *env, jobject obj) {
    mTarget = env->NewGlobalRef(obj);
}

JNIAudioPlayerListener::~JNIAudioPlayerListener() {
    JNIEnv *env;
    javaVm->AttachCurrentThread(&env, nullptr);
    env->DeleteGlobalRef(mTarget);
}

void JNIAudioPlayerListener::notify(int what, int thread) {
    JNIEnv *env;
    switch (thread) {
        case THREAD_WORK:
            javaVm->AttachCurrentThread(&env, nullptr);
            callJavaNotify(env, mTarget, what);
            javaVm->DetachCurrentThread();
            break;
        case THREAD_MAIN:
            javaVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
            callJavaNotify(env, mTarget, what);
            break;
        default:
            break;
    }
}

void JNIAudioPlayerListener::updateTime(int duration, double progress, int thread) {
    JNIEnv *env;
    switch (thread) {
        case THREAD_WORK:
            javaVm->AttachCurrentThread(&env, nullptr);
            env->CallVoidMethod(mTarget, audioPlayerJava.updateTime, duration, progress);
            javaVm->DetachCurrentThread();
            break;
        case THREAD_MAIN:
            javaVm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
            env->CallVoidMethod(mTarget, audioPlayerJava.updateTime, duration, progress);
            break;
        default:
            break;
    }
}

void
JNIAudioPlayerListener::onYUVCallback(int width, int height, uint8_t *yBuffer, uint8_t *uBuffer,
                                      uint8_t *vBuffer) {
    JNIEnv *env;
    javaVm->AttachCurrentThread(&env, nullptr);

    jbyteArray yArray = env->NewByteArray(width * height);
    env->SetByteArrayRegion(yArray, 0, width * height, reinterpret_cast<const jbyte *>(yBuffer));

    jbyteArray uArray = env->NewByteArray(width * height / 4);
    env->SetByteArrayRegion(uArray, 0, width * height / 4,
                            reinterpret_cast<const jbyte *>(uBuffer));

    jbyteArray vArray = env->NewByteArray(width * height / 4);
    env->SetByteArrayRegion(vArray, 0, width * height / 4,
                            reinterpret_cast<const jbyte *>(vBuffer));

    env->CallVoidMethod(mTarget, audioPlayerJava.yuvCallbackMid, width, height, yArray, uArray,
                        vArray);
    javaVm->DetachCurrentThread();
}

MediaPlayer *getMediaPlayer(jlong nativePtr) {
    return reinterpret_cast<MediaPlayer *>(nativePtr);
}

extern "C" JNIEXPORT void JNICALL
setDataSource(JNIEnv *env, jobject thiz, jstring dataSource, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->setDataSource(env->GetStringUTFChars(dataSource, 0));
}
extern "C"
JNIEXPORT void JNICALL
prepare(JNIEnv *env, jobject thiz, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->prepare();
}

extern "C" JNIEXPORT void JNICALL
prepareAsync(JNIEnv *env, jobject thiz, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->prepareAsync();
}
extern "C"
JNIEXPORT void JNICALL
start(JNIEnv *env, jobject thiz, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->start();
}
extern "C"
JNIEXPORT void JNICALL
stop(JNIEnv *env, jobject thiz, jlong nativePtr) {
    LOGE(TAG, "############ begin jni stop ############");
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->release();
}
extern "C"
JNIEXPORT void JNICALL
pause(JNIEnv *env, jobject thiz, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->pause();
}

extern "C"
JNIEXPORT void JNICALL
resume(JNIEnv *env, jobject thiz, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->resume();
}
extern "C"
JNIEXPORT void JNICALL
release(JNIEnv *env, jobject thiz, jlong nativePtr) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->release();
    delete (mediaPlayer);
}

extern "C"
JNIEXPORT void JNICALL
seekTo(JNIEnv *env, jobject thiz, jlong nativePtr, jint time) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->seekTo(time);
}

extern "C"
JNIEXPORT void JNICALL
setVolume(JNIEnv *env, jobject obj, jlong nativePtr, jint volume) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->setVolume(volume);
}

extern "C"
JNIEXPORT void JNICALL
setSoundChannel(JNIEnv *env, jobject obj, jlong nativePtr, jint channel) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->setSoundChannel(channel);
}

extern "C"
JNIEXPORT void JNICALL
setPitch(JNIEnv *env, jobject obj, jlong nativePtr, jfloat pitch) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->setPitch(pitch);
}

extern "C"
JNIEXPORT void JNICALL
setSpeed(JNIEnv *env, jobject obj, jlong nativePtr, jfloat speed) {
    MediaPlayer *mediaPlayer = getMediaPlayer(nativePtr);
    mediaPlayer->setSpeed(speed);
}

extern "C"
JNIEXPORT void JNICALL
native_init(JNIEnv *env, jobject thiz) {
    auto *mediaPlayer = new MediaPlayer();
    auto *listener = new JNIAudioPlayerListener(env, thiz);
    mediaPlayer->setListener(listener);
    jclass clazz = env->GetObjectClass(thiz);
    if (clazz != nullptr) {
        jfieldID nativePtrFieldId = env->GetFieldID(clazz, "mNativePtr", "J");
        if (nativePtrFieldId == nullptr) {
            return;
        }
        env->SetLongField(thiz, nativePtrFieldId, reinterpret_cast<jlong>(mediaPlayer));
    }
}
const JNINativeMethod methods[] = {
        {"_native_init",     "()V",                    (void *) native_init},
        {"_setDataSource",   "(Ljava/lang/String;J)V", (void *) setDataSource},
        {"_prepare",         "(J)V",                   (void *) prepare},
        {"_prepareAsync",    "(J)V",                   (void *) prepareAsync},
        {"_start",           "(J)V",                   (void *) start},
        {"_pause",           "(J)V",                   (void *) pause},
        {"_resume",          "(J)V",                   (void *) resume},
        {"_stop",            "(J)V",                   (void *) stop},
        {"_release",         "(J)V",                   (void *) release},
        {"_seekTo",          "(JI)V",                  (void *) seekTo},
        {"_setVolume",       "(JI)V",                  (void *) setVolume},
        {"_setSoundChannel", "(JI)V",                  (void *) setSoundChannel},
        {"_setPitch",        "(JF)V",                  (void *) setPitch},
        {"_setSpeed",        "(JF)V",                  (void *) setSpeed}
};

extern "C"
JNIEXPORT int JNICALL JNI_OnLoad(JavaVM *vm, void *unused) {
    javaVm = vm;
    JNIEnv *env;
    vm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);

    const char *className = "com/xingzhe/player/MediaPlayer";
    jclass clazz = env->FindClass(className);
    if (clazz == nullptr) {
        return -1;
    }
    audioPlayerJava.postEvent = env->GetMethodID(clazz, "postEventFromNative", "(I)V");
    audioPlayerJava.updateTime = env->GetMethodID(clazz, "onTimeUpdate", "(ID)V");
    audioPlayerJava.yuvCallbackMid = env->GetMethodID(clazz, "onYUVCallback", "(II[B[B[B)V");
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) < 0) {
        return -1;
    }
    return JNI_VERSION_1_6;
}
