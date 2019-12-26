
#include "shareddate.h"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<jni.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include<signal.h>
#include <sys/mman.h>
#include<android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "native-activity", __VA_ARGS__))
#define LOGW(...) ((void)__android_log_print(ANDROID_LOG_WARN, "native-activity", __VA_ARGS__))
#define LOGE(...) ((void)__android_log_print(ANDROID_LOG_ERROR, "native-activity", __VA_ARGS__))
#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

static char CLASS_NAME[] = "com/jesson/mmap/ShareMemUtils";
static shared_use_st *use_st = NULL;
//JNI 免费的公开课
static jint throwException(JNIEnv* env, jobject clazz, const char *clsname, const char *msg)
{
    jclass cls;
    cls = env->FindClass(clsname);
    if(cls == NULL){
}
    env->ThrowNew(cls, msg);
    return -1;
}

static jint openMem(JNIEnv* env, jobject clazz, jstring name, jint length)
{
    shared_use_st *st = NULL;
    const char* namestr = (name ? env->GetStringUTFChars(name, NULL) : NULL);
    int result = -1;

    if(access(namestr, F_OK) == 0)
    {
        result = open(namestr, O_RDWR);
    }
    else
    {
        result = open(namestr, O_RDWR|O_CREAT);

        if(result >= 0)
        {
            st = (shared_use_st *)malloc(sizeof(shared_use_st));

            if(st == NULL)
            {
                LOGE("open malloc failed");

            }
            if(write(result, st, sizeof(shared_use_st)) < 0)
            {
                LOGE("open write failed");

            }

            free(st);
        }
    }

    env->ReleaseStringUTFChars(name, namestr);

    return result;

}
/***
 * 内存映射
 * @param env
 * @param clazz
 * @param fd
 * @return
 */
static jint getMem(JNIEnv* env, jobject clazz,int fd)
{
    struct stat sb;
    int result = -1;

    if(fd >= 0)
    {
        if((fstat(fd, &sb)) == -1)
        {
            LOGE("readMem size failed");
            return result;
        }

        if((use_st = (shared_use_st *)mmap(NULL, sb.st_size,
                                           PROT_READ|PROT_WRITE, MAP_SHARED,
                                           fd, 0)) == (void *)-1)
        {
            LOGE("readMem mmap failed");
            return result;
        }
        result = reinterpret_cast<jlong>(use_st->text);
        LOGE("memory address:%0x", result);

    }

    return result;
}

static jint writeMem(JNIEnv* env, jobject clazz, int fd, jbyteArray buffer, int size, jint address)
{
    int ret = 0;

    if(use_st != NULL && address == reinterpret_cast<jlong >(use_st->text))
    {
        env->GetByteArrayRegion(buffer, 0, size, (jbyte *)address);
        ret = size;
    }

    return ret;
}
static jbyteArray readMem(JNIEnv* env, jobject clazz, int fd, int address)
{


    if(fd >= 0)
    {
        if(use_st != NULL)
        {
            jbyteArray array = env->NewByteArray(TEXT_SZ);
            env->SetByteArrayRegion(array, 0, TEXT_SZ,
                                    reinterpret_cast<const jbyte *>((char *) address));
            return array;
        }
    }

    return NULL;
}

static void setModeMem(JNIEnv* env, jobject clazz, int mode)
{
    if(use_st != NULL)
    {
        use_st->written = mode;
    }
}

static int getModeMem(JNIEnv* env, jobject clazz)
{
    int nRet = -1;

    if(use_st != NULL)
    {
        nRet = use_st->written;
    }
    return nRet;
}

static  void closeMem(JNIEnv* env, jobject clazz, int fd)
{
    close(fd);

    if(use_st != NULL)
    {
        if ((munmap((void *)use_st,
                    sizeof(shared_use_st))) == -1)
        {
            LOGE("munmap failed");
        }
    }

}

static  jboolean flushMem(JNIEnv* env, jobject clazz)
{
    jboolean isFailed = 1;
    if(use_st != NULL)
    {
        if ((msync((void *)use_st, sizeof(shared_use_st), MS_SYNC)) == -1) 		{
            LOGE("msync failed");
        }
        else
        {
            isFailed = 0;
        }
    }

    return !isFailed;
}

static JNINativeMethod mehods[] = {

        { "open", "(Ljava/lang/String;I)I",
                (void *) openMem },
        { "read", "(II)[B",
                (void *) readMem },
        { "write", "(I[BII)I",
                (void *) writeMem },
        { "get", "(I)I",
                (void *) getMem },
        { "setMode", "(I)V",
                (void *) setModeMem },
        { "getMode", "()I",
                (void *) getModeMem },
        { "flush", "()Z",
                (void *) flushMem },
        { "close", "(I)V",
                (void *) closeMem }

};

//JNI的动态注册
static int registerNativeMethods(JNIEnv *env, const char* className,
                                 const JNINativeMethod* methods, int numMethods)
{
    int rc;
    jclass clazz;
    clazz = env->FindClass(className);
    if (clazz == NULL) {
        LOGE("Native registration unable to find class '%s'\n", className);
        return -1;
    }
    if (rc = (env->RegisterNatives(clazz, methods, numMethods)) < 0) {
        LOGE("RegisterNatives failed for '%s' %d\n", className, rc);
        return -1;
    }
    return 0;
}


static int register_jni(JNIEnv *env)
{
    return registerNativeMethods(env, CLASS_NAME, mehods, NELEM(mehods));
}
//JNI_OnLoad 函数 JVM 免费课1次 onload
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *reserved)
{
    JNIEnv* env = NULL;
    jint result = -1;

    //获取JNI版本
    if (vm->GetEnv((void**)&env, JNI_VERSION_1_4) != JNI_OK)
    {
        LOGE("GetEnv failed!");
        return result;
    }
    if (register_jni(env) < 0)
    {
        LOGE("register method failed!");
        return result;
    }
    return JNI_VERSION_1_4;
}