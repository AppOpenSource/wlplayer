#include <jni.h>
#include <stddef.h>
#include "AndroidLog.h"
#include "WlJavaCall.h"
#include "WlFFmpeg.h"

_JavaVM *javaVM = NULL;
WlJavaCall *wlJavaCall = NULL;
WlFFmpeg *wlFFmpeg = NULL;

#define TEST_H264  1
#define TEST_HEVC  0

int main1() {
    AVCodec *pCodec;
    AVCodecContext *pCodecCtx = NULL;
    AVCodecParserContext *pCodecParserCtx = NULL;

    FILE *fp_in;
    FILE *fp_out;
    AVFrame *pFrame;

    const int in_buffer_size = 4096;
    uint8_t in_buffer[in_buffer_size + FF_INPUT_BUFFER_PADDING_SIZE] = {0};
    uint8_t *cur_ptr;
    int cur_size;
    AVPacket packet;
    int ret, got_picture;
    int y_size;


#if TEST_HEVC
    enum AVCodecID codec_id=AV_CODEC_ID_HEVC;
    char filepath_in[]="bigbuckbunny_480x272.hevc";
#elif TEST_H264
    AVCodecID codec_id=AV_CODEC_ID_H264;
    //char filepath_in[]="bigbuckbunny_480x272.h264";
    char filepath_in[]="/sdcard/aaa.264";
#else
    AVCodecID codec_id = AV_CODEC_ID_MPEG2VIDEO;
    char filepath_in[] = "bigbuckbunny_480x272.m2v";
#endif

    //char filepath_out[] = "bigbuckbunny_480x272.yuv";
    char filepath_out[]="/sdcard/aaa_out.yuv";

    int first_time = 1;

    //av_log_set_level(AV_LOG_DEBUG);

    avcodec_register_all();

    pCodec = avcodec_find_decoder(codec_id);
    if (!pCodec) {
        LOGD("Codec not found\n");
        return -1;
    }
    pCodecCtx = avcodec_alloc_context3(pCodec);
    if (!pCodecCtx) {
        LOGD("Could not allocate video codec context\n");
        return -1;
    }

    pCodecParserCtx = av_parser_init(codec_id);
    if (!pCodecParserCtx) {
        LOGD("Could not allocate video parser context\n");
        return -1;
    }

    //if(pCodec->capabilities&CODEC_CAP_TRUNCATED)
    //    pCodecCtx->flags|= CODEC_FLAG_TRUNCATED;

    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
        LOGD("Could not open codec\n");
        return -1;
    }
    //Input File
    fp_in = fopen(filepath_in, "rb");
    if (!fp_in) {
        LOGD("Could not open input stream, path = %s\n", filepath_in);
        return -1;
    }
    //Output File
    fp_out = fopen(filepath_out, "wb");
    if (!fp_out) {
        LOGD("Could not open output YUV file\n");
        return -1;
    }

    pFrame = av_frame_alloc();
    av_init_packet(&packet);

    while (1) {

        cur_size = fread(in_buffer, 1, in_buffer_size, fp_in);
        if (cur_size == 0)
            break;
        cur_ptr = in_buffer;

        while (cur_size > 0) {

            int len = av_parser_parse2(
                    pCodecParserCtx, pCodecCtx,
                    &packet.data, &packet.size,
                    cur_ptr, cur_size,
                    AV_NOPTS_VALUE, AV_NOPTS_VALUE, AV_NOPTS_VALUE);

            cur_ptr += len;
            cur_size -= len;

            if (packet.size == 0)
                continue;

            //Some Info from AVCodecParserContext
            LOGD("[Packet]Size:%6d\t", packet.size);
            switch (pCodecParserCtx->pict_type) {
                case AV_PICTURE_TYPE_I:
                    LOGD("Type:I\t");
                    break;
                case AV_PICTURE_TYPE_P:
                    LOGD("Type:P\t");
                    break;
                case AV_PICTURE_TYPE_B:
                    LOGD("Type:B\t");
                    break;
                default:
                    LOGD("Type:Other\t");
                    break;
            }
            LOGD("Number:%4d\n", pCodecParserCtx->output_picture_number);

            wlJavaCall->onDecMediacodec(WL_THREAD_CHILD, packet.size, packet.data, 0);

            av_usleep(1000 * 55);
#if 0
            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
            if (ret < 0) {
                LOGD("Decode Error.\n");
                return ret;
            }
            if (got_picture) {
                if (first_time) {
                    LOGD("\nCodec Full Name:%s\n", pCodecCtx->codec->long_name);
                    LOGD("width:%d\nheight:%d\n\n", pCodecCtx->width, pCodecCtx->height);
                    first_time = 0;
                }
                //Y, U, V
                for (int i = 0; i < pFrame->height; i++) {
                    fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
                }
                for (int i = 0; i < pFrame->height / 2; i++) {
                    fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
                }
                for (int i = 0; i < pFrame->height / 2; i++) {
                    fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
                }

                LOGD("Succeed to decode 1 frame!\n");
            }
#endif
        }

    }

    //Flush Decoder
    packet.data = NULL;
    packet.size = 0;
    while (0) {
        ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, &packet);
        if (ret < 0) {
            LOGD("Decode Error.\n");
            return ret;
        }
        if (!got_picture) {
            break;
        } else {
            //Y, U, V
            for (int i = 0; i < pFrame->height; i++) {
                fwrite(pFrame->data[0] + pFrame->linesize[0] * i, 1, pFrame->width, fp_out);
            }
            for (int i = 0; i < pFrame->height / 2; i++) {
                fwrite(pFrame->data[1] + pFrame->linesize[1] * i, 1, pFrame->width / 2, fp_out);
            }
            for (int i = 0; i < pFrame->height / 2; i++) {
                fwrite(pFrame->data[2] + pFrame->linesize[2] * i, 1, pFrame->width / 2, fp_out);
            }

            LOGD("Flush Decoder: Succeed to decode 1 frame!\n");
        }
    }

    fclose(fp_in);
    fclose(fp_out);


    av_parser_close(pCodecParserCtx);

    av_frame_free(&pFrame);
    avcodec_close(pCodecCtx);
    av_free(pCodecCtx);

    return 0;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_testH264(JNIEnv *env, jobject instance) {



}

extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlPrepared(JNIEnv *env, jobject instance, jstring url_,
                                              jboolean isOnlyMusic) {
    const char *url = env->GetStringUTFChars(url_, 0);
    if (wlJavaCall == NULL) {
        wlJavaCall = new WlJavaCall(javaVM, env, &instance);
    }
    if (wlFFmpeg == NULL) {
        wlFFmpeg = new WlFFmpeg(wlJavaCall, url, isOnlyMusic);
        wlJavaCall->onLoad(WL_THREAD_MAIN, true);
        wlFFmpeg->preparedFFmpeg();
    }
}


extern "C"
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved) {
    jint result = -1;
    javaVM = vm;
    JNIEnv *env;

    if (vm->GetEnv((void **) &env, JNI_VERSION_1_4) != JNI_OK) {
        if (LOG_SHOW) {
            LOGE("GetEnv failed!");
        }
        return result;
    }
    return JNI_VERSION_1_4;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlStart(JNIEnv *env, jobject instance) {
    if (wlFFmpeg != NULL) {
        //wlFFmpeg->start();
        main1();
    }
}

extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlStop(JNIEnv *env, jobject instance, bool exit) {
    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->exitByUser = true;
        wlFFmpeg->release();
        delete (wlFFmpeg);
        wlFFmpeg = NULL;
        if (wlJavaCall != NULL) {
            wlJavaCall->release();
            wlJavaCall = NULL;
        }
        if (!exit) {
            jclass jlz = env->GetObjectClass(instance);
            jmethodID jmid_stop = env->GetMethodID(jlz, "onStopComplete", "()V");
            env->CallVoidMethod(instance, jmid_stop);
        }
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlPause(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->pause();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlResume(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->resume();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlSeek(JNIEnv *env, jobject instance, jint secds) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->seek(secds);
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlGetDuration(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        return wlFFmpeg->getDuration();
    }
    return 0;

}extern "C"
JNIEXPORT jint JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlGetAudioChannels(JNIEnv *env, jobject instance) {

    if (wlFFmpeg != NULL) {
        return wlFFmpeg->getAudioChannels();
    }
    return 0;
}extern "C"
JNIEXPORT jint JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlGetVideoWidth(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->getVideoWidth();
    }

}extern "C"
JNIEXPORT jint JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlGetVideoHeidht(JNIEnv *env, jobject instance) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->getVideoHeight();
    }

}extern "C"
JNIEXPORT void JNICALL
Java_com_ywl5320_wlplayer_WlPlayer_wlSetAudioChannels(JNIEnv *env, jobject instance, jint index) {

    // TODO
    if (wlFFmpeg != NULL) {
        wlFFmpeg->setAudioChannel(index);
    }

}
