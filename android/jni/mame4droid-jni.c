/*
 * This file is part of MAME4droid.
 *
 * Copyright (C) 2011 David Valdeita (Seleuco)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * In addition, as a special exception, Seleuco
 * gives permission to link the code of this program with
 * the MAME library (or with modified versions of MAME that use the
 * same license as MAME), and distribute linked combinations including
 * the two.  You must obey the GNU General Public License in all
 * respects for all of the code used other than MAME.  If you modify
 * this file, you may extend this exception to your version of the
 * file, but you are not obligated to do so.  If you do not wish to
 * do so, delete this exception statement from your version.
 */

#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include <math.h>

#include <pthread.h>

#include "com_seleuco_mame4droid_Emulator.h"

#define DEBUG 1

//mame4droid funtions
int  (*android_main)(int argc, char **argv)=NULL;
void (*setAudioCallbacks)(void *func1,void *func2,void *func3)= NULL;
void (*setVideoCallbacks)(void *func1,void *func2,void *func3) = NULL;
void (*setPadStatus)(int i, unsigned long pad_status) = NULL;
void (*setGlobalPath)(const char *path) = NULL;

void  (*setMyValue)(int key,int i, int value)=NULL;
int  (*getMyValue)(int key, int i)=NULL;
void  (*setMyValueStr)(int key, int i,const char *value)=NULL;
char *(*getMyValueStr)(int key,int i)=NULL;

void  (*setMyAnalogData)(int i, float v1,float v2)=NULL;

void  (*droid_video_thread)()=NULL;

int (*netplayInit)(const char *value, int i, int j)=NULL;
void (*setNetplayCallbacks)(void *func1) = NULL;

/* Callbacks to Android */
jmethodID android_dumpVideo;
jmethodID android_changeVideo;
jmethodID android_openAudio;
jmethodID android_dumpAudio;
jmethodID android_closeAudio;
jmethodID android_netplayWarn;

static JavaVM *jVM = NULL;
static void *libdl = NULL;
static jclass cEmulator = NULL;

static jobject videoBuffer=NULL;//es un ByteBuffer wrappeando el buffer de video en la libreria 

static jbyteArray jbaAudioBuffer = NULL;

static jobject audioBuffer=NULL;
static unsigned char audioByteBuffer[882 * 2 * 2 * 10];

static pthread_t main_tid;


static void load_lib(const char *str)
{

    char str2[256];
    
    strcpy(str2,str);
    strcpy(str2+strlen(str),"/libMAME4droid.so");

#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "Attempting to load %s\n", str2);
#endif

    if(libdl!=NULL)
        return;

    libdl = dlopen(str2, RTLD_NOW);
    if(!libdl)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Unable to load libMAME4droid.so: %s\n", dlerror());
        return;
    }

    android_main = dlsym(libdl, "android_main");
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","android_main %d\n", android_main!=NULL);

    setVideoCallbacks = dlsym(libdl, "setVideoCallbacks");
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setVideoCallbacks %d\n", setVideoCallbacks!=NULL);

    setAudioCallbacks = dlsym(libdl, "setAudioCallbacks");    
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setAudioCallbacks %d\n", setAudioCallbacks!=NULL);

    setPadStatus = dlsym(libdl, "setPadStatus");
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setPadStatus %d\n", setPadStatus!=NULL);

    setGlobalPath = dlsym(libdl, "setGlobalPath"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setGlobalPath %d\n", setGlobalPath!=NULL);

    setMyValue = dlsym(libdl, "setMyValue"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setMyValue %d\n",setMyValue!=NULL);

    getMyValue = dlsym(libdl, "getMyValue"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","getMyValue %d\n", getMyValue!=NULL);

    setMyValueStr = dlsym(libdl, "setMyValueStr"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setMyValueStr %d\n",setMyValueStr!=NULL);

    getMyValueStr = dlsym(libdl, "getMyValueStr"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","getMyValueStr %d\n", getMyValueStr!=NULL);

    setMyAnalogData = dlsym(libdl, "setMyAnalogData"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setMyAnalogData %d\n", setMyAnalogData!=NULL);
    
    droid_video_thread = dlsym(libdl, "droid_ios_video_thread"); 
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","droid_ios_video_thread %d\n", droid_video_thread!=NULL);

    netplayInit = dlsym(libdl, "netplayInit"); 
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","netplayInit %d\n", netplayInit!=NULL);
    
    setNetplayCallbacks = dlsym(libdl, "setNetplayCallbacks");
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni","setNetplayCallbacks %d\n", setNetplayCallbacks!=NULL);
}

void myJNI_initVideo(void *buffer, int width, int height, int pitch)
{
    JNIEnv *env;
    jobject tmp;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);
#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "initVideo");
#endif
    tmp = (*env)->NewDirectByteBuffer(env, buffer, width * height * pitch);

    videoBuffer = (jobject)(*env)->NewGlobalRef(env, tmp);

    if(!videoBuffer) __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "yikes, unable to initialize video buffer");

}

void myJNI_dumpVideo()
{
    JNIEnv *env;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);

#ifdef DEBUG
    //__android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "dumpVideo");
#endif

   (*env)->CallStaticVoidMethod(env, cEmulator,  android_dumpVideo, videoBuffer);
}

void myJNI_changeVideo(int newWidth, int newHeight,int newVisWidth, int newVisHeight)
{
    JNIEnv *env;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);

#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "changeVideo");
#endif

    (*env)->CallStaticVoidMethod(env, cEmulator, android_changeVideo, (jint)newWidth,(jint)newHeight,(jint)newVisWidth,(jint)newVisHeight);
}

void myJNI_closeAudio()
{
    JNIEnv *env;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);

#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "closeAudio");
#endif

    (*env)->CallStaticVoidMethod(env, cEmulator, android_closeAudio);
}

void myJNI_openAudio(int rate, int stereo)
{
    JNIEnv *env;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);

#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "openAudio");
#endif


    (*env)->CallStaticVoidMethod(env, cEmulator, android_openAudio, (jint)rate,(jboolean)stereo);
}

void myJNI_dumpAudio(void *buffer, int size)
{
    JNIEnv *env;
    jobject tmp;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);

#ifdef DEBUG
    //__android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "dumpAudio %ld %d",buffer, size);
#endif

    if(jbaAudioBuffer==NULL)
    {
        jbaAudioBuffer=(*env)->NewByteArray(env, 882*2*2*10);
        tmp = jbaAudioBuffer;
        jbaAudioBuffer=(jbyteArray)(*env)->NewGlobalRef(env, jbaAudioBuffer);
        (*env)->DeleteLocalRef(env, tmp);
    }    

    (*env)->SetByteArrayRegion(env, jbaAudioBuffer, 0, size, (jbyte *)buffer);
   
    (*env)->CallStaticVoidMethod(env, cEmulator, android_dumpAudio,jbaAudioBuffer,(jint)size);
}

void myJNI_dumpAudio2(void *buffer, int size)
{
    JNIEnv *env;
    jobject tmp;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);

#ifdef DEBUG
    //__android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "dumpAudio %ld %d",buffer, size);
#endif

    if(audioBuffer==NULL)
    {
       tmp = (*env)->NewDirectByteBuffer(env, audioByteBuffer, 882*2*2*10);
       audioBuffer = (jobject)(*env)->NewGlobalRef(env, tmp);
    }
    
    memcpy(audioByteBuffer,buffer,size);

    (*env)->CallStaticVoidMethod(env, cEmulator, android_dumpAudio, audioBuffer,(jint)size);

}
/*
The global/local distinction affects both lifetime and scope. A global is usable from any thread, using that thread’s JNIEnv*, and is valid until an explicit call to DeleteGlobalRef(). A local is only usable from the thread it was originally handed to, and is valid until either an explicit call to DeleteLocalRef() or, more commonly, until you return from your native method. When a native method returns, all local references are automatically deleted.
*/

void myJNI_netplayWarn(char *msg)
{
    JNIEnv *env;
    (*jVM)->GetEnv(jVM, (void**) &env, JNI_VERSION_1_4);
    int attached  = 0;

#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "netplayWarn");
#endif
   if(msg!=NULL)
   {
     __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "netplayWarn %s\n",msg);
     if(env==NULL)
     {
        attached  = 1;
        (*jVM)->AttachCurrentThread(jVM,(void *) &env, NULL);
     }

     jstring jstrBuf = (*env)->NewStringUTF(env, msg);
     (*env)->CallStaticVoidMethod(env, cEmulator, android_netplayWarn, jstrBuf);

     if(attached)
        (*jVM)->DetachCurrentThread(jVM);

   }
   //(*env)->DeleteLocalRef(env, jstrBuf);
}

int JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    jVM = vm;

#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "JNI_OnLoad called");
#endif

    if((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to get the environment using GetEnv()");
        return -1;
    }
    
    cEmulator = (*env)->FindClass (env, "com/seleuco/mame4droid/Emulator"); 

    if(cEmulator==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find class com.seleuco.mame4droid.Emulator");
        return -1;
    } 

    cEmulator = (jclass) (*env)->NewGlobalRef(env,cEmulator );

    android_dumpVideo = (*env)->GetStaticMethodID(env,cEmulator,"bitblt","(Ljava/nio/ByteBuffer;)V");
    
    if(android_dumpVideo==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find method bitblt");
        return -1;
    }

    android_changeVideo= (*env)->GetStaticMethodID(env,cEmulator,"changeVideo","(IIII)V");
    
    if(android_changeVideo==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find method changeVideo");
        return -1;
    }

    //android_dumpAudio = (*env)->GetStaticMethodID(env,cEmulator,"writeAudio","(Ljava/nio/ByteBuffer;I)V");
    android_dumpAudio = (*env)->GetStaticMethodID(env,cEmulator,"writeAudio","([BI)V");

    if(android_dumpAudio==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find method writeAudio");
        return -1;
    }

    android_openAudio = (*env)->GetStaticMethodID(env,cEmulator,"initAudio","(IZ)V");

    if(android_openAudio==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find method openAudio");
        return -1;
    }

    android_closeAudio = (*env)->GetStaticMethodID(env,cEmulator,"endAudio","()V");

    if(android_closeAudio==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find method closeAudio");
        return -1;
    }

    android_netplayWarn = (*env)->GetStaticMethodID(env,cEmulator,"netplayWarn","(Ljava/lang/String;)V");

    if(android_netplayWarn==NULL)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Failed to find method netplayWarn");
        return -1;
    }
   
    return JNI_VERSION_1_4;
}


void* app_Thread_Start(void* args)
{
    android_main(0, NULL); 
    return NULL;
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_init
  (JNIEnv *env, jclass c,  jstring s1, jstring s2)
{
    __android_log_print(ANDROID_LOG_INFO, "mame4droid-jni", "init");

    const char *str1 = (*env)->GetStringUTFChars(env, s1, 0);

    load_lib(str1);

    (*env)->ReleaseStringUTFChars(env, s1, str1);
    
    __android_log_print(ANDROID_LOG_INFO, "mame4droid-jni","calling setVideoCallbacks");
    if(setVideoCallbacks!=NULL)
        setVideoCallbacks(&myJNI_initVideo,&myJNI_dumpVideo,&myJNI_changeVideo);   

    __android_log_print(ANDROID_LOG_INFO, "mame4droid-jni","calling setAudioCallbacks");
    if(setAudioCallbacks!=NULL)
       setAudioCallbacks(&myJNI_openAudio,&myJNI_dumpAudio,&myJNI_closeAudio);

    __android_log_print(ANDROID_LOG_INFO, "mame4droid-jni","calling setNetplayCallbacks");
    if(setNetplayCallbacks!=NULL)
       setNetplayCallbacks(&myJNI_netplayWarn);

    const char *str2 = (*env)->GetStringUTFChars(env, s2, 0);

    __android_log_print(ANDROID_LOG_INFO, "mame4droid-jni", "path %s",str2);

    setGlobalPath(str2);

    (*env)->ReleaseStringUTFChars(env, s2, str2);

    //int i = pthread_create(&main_tid, NULL, app_Thread_Start, NULL);

    //if(i!=0)__android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Error setting creating pthread %d",i);	
    //struct sched_param    param;
    //param.sched_priority = 63;
    //param.sched_priority = 46;
    //param.sched_priority = 100;
    /*  
    if(pthread_setschedparam(main_tid, SCHED_RR, &param) != 0)
    {
        __android_log_print(ANDROID_LOG_ERROR, "mame4droid-jni", "Error setting pthread priority");
        return;
    }
    */  
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_setPadData
  (JNIEnv *env, jclass c, jint i,  jlong jl)
{
    //long 	jlong 	signed 64 bits ??? valdria con un jint
    //__android_log_print(ANDROID_LOG_INFO, "mame4droid-jni", "setPadData");

    unsigned long l = (unsigned long)jl;

    if(setPadStatus!=NULL)
       setPadStatus(i,l);
    else
      __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no setPadStatus!");
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_setAnalogData
  (JNIEnv *env, jclass c, jint i, jfloat v1, jfloat v2)
{
    if(setMyAnalogData!=NULL)
       setMyAnalogData(i,v1,v2);
    else
      __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no setMyAnalogData!");
}

JNIEXPORT jint JNICALL Java_com_seleuco_mame4droid_Emulator_getValue
  (JNIEnv *env, jclass c, jint key, jint i)
{
#ifdef DEBUG
   // __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "getValue %d",key);
#endif
      if(getMyValue!=NULL)
         return getMyValue(key,i);
      else 
      {
         __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no getMyValue!");
         return -1;
      }
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_setValue
  (JNIEnv *env, jclass c, jint key, jint i, jint value)
{
#ifdef DEBUG
    //__android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "setValue %d,%d=%d",key,i,value);
#endif
    if(setMyValue!=NULL)
      setMyValue(key,i,value);
    else
      __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no setMyValue!");
}

JNIEXPORT jstring JNICALL Java_com_seleuco_mame4droid_Emulator_getValueStr
  (JNIEnv *env, jclass c, jint key, jint i)
{
#ifdef DEBUG
   // __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "getValueStr %d",key);
#endif
      if(getMyValueStr!=NULL)
      {
         const char * r =  getMyValueStr(key,i);
         return (*env)->NewStringUTF(env,r);
      }
      else 
      {
         __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no getMyValueStr!");
         return NULL;
      }
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_setValueStr
  (JNIEnv *env, jclass c, jint key, jint i, jstring s1)
{
    if(setMyValueStr!=NULL)
    {
       const char *value = (*env)->GetStringUTFChars(env, s1, 0);
#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "setValueStr %d,%d=%s",key,i,value);
#endif
       setMyValueStr(key,i,value);
       (*env)->ReleaseStringUTFChars(env, s1, value);
    }
    else
      __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no setMyValueStr!");
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_runT
  (JNIEnv *env, jclass c){
#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "runThread");
#endif
    if(android_main!=NULL)
       android_main(0, NULL);  
    else
       __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no android main!");
}

JNIEXPORT void JNICALL Java_com_seleuco_mame4droid_Emulator_runVideoT
  (JNIEnv *env, jclass c){
#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "runVideoThread");
#endif
    if(droid_video_thread!=NULL)        
       droid_video_thread();
    else
      __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no droid_video_thread!");
}

JNIEXPORT jint JNICALL Java_com_seleuco_mame4droid_Emulator_netplayInit
  (JNIEnv *env, jclass c, jstring addr, jint port, jint join){
#ifdef DEBUG
    __android_log_print(ANDROID_LOG_DEBUG, "mame4droid-jni", "netplayInit");
#endif
    if(droid_video_thread!=NULL)    
    {     
        const char *str = NULL;
        if(addr!=NULL)
           str = (*env)->GetStringUTFChars(env, addr, 0);
        netplayInit(str,port,join);
        if(addr!=NULL)
           (*env)->ReleaseStringUTFChars(env, addr, str);
    }
    else
      __android_log_print(ANDROID_LOG_WARN, "mame4droid-jni", "error no netplayInit!");
}






