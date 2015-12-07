#include "io_github_melvincabatuan_nativebitmap_MainActivity.h"

#include <android/bitmap.h>
#include <android/log.h>
#include <stdlib.h>
#include <sys/time.h>

#define  LOG_TAG    "NativeBitmap"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

#define toInt(pValue) \
 (0xff & (int32_t) pValue)

#define max(pValue1, pValue2) \
 (pValue1<pValue2) ? pValue2 : pValue1

#define clamp(pValue, pLowest, pHighest) \
 ((pValue < 0) ? pLowest : (pValue > pHighest) ? pHighest: pValue)

#define color(pColorR, pColorG, pColorB) \
           (0xFF000000 | ((pColorB << 6)  & 0x00FF0000) \
                       | ((pColorG >> 2)  & 0x0000FF00) \
                       | ((pColorR >> 10) & 0x000000FF))


/*
 * Class:     io_github_melvincabatuan_nativebitmap_MainActivity
 * Method:    decode
 * Signature: (Landroid/graphics/Bitmap;[BI)V
 */


JNIEXPORT void JNICALL Java_io_github_melvincabatuan_nativebitmap_MainActivity_decode
  (JNIEnv * pEnv, jobject pClass, jobject pTarget, jbyteArray pSource, jint pFilter){

   AndroidBitmapInfo bitmapInfo;
   uint32_t* bitmapContent;

   if(AndroidBitmap_getInfo(pEnv, pTarget, &bitmapInfo) < 0) abort();
   if(bitmapInfo.format != ANDROID_BITMAP_FORMAT_RGBA_8888) abort();
   if(AndroidBitmap_lockPixels(pEnv, pTarget, (void**)&bitmapContent) < 0) abort();

   /// Access source array data
   jbyte* source = (*pEnv)->GetPrimitiveArrayCritical(pEnv, pSource, 0);
   if (source == NULL) abort();

   /// Decode raw video into output bitmap
   int32_t frameSize = bitmapInfo.width * bitmapInfo.height;
   int32_t yIndex, uvIndex, x, y;
   int32_t colorY, colorU, colorV;
   int32_t colorR, colorG, colorB;
   int32_t y1192;
 
   
   /// Convert YUV to RGB
   for (y = 0, yIndex = 0; y < bitmapInfo.height; ++y){
        colorU = 0; colorV = 0;

        uvIndex = frameSize + (y >> 1) * bitmapInfo.width;

        for(x = 0; x < bitmapInfo.width; ++x, ++yIndex ){
            
            colorY = max(toInt(source[yIndex]) - 16, 0);
            if(!(x % 2)){
                colorV = toInt(source[uvIndex++]) -128;
                colorU = toInt(source[uvIndex++]) -128;
            }

            /// Compute RGB from YUV
            y1192 = 1192 * colorY;
            colorR = (y1192 + 1634 * colorV);
            colorG = (y1192 - 833 * colorV - 400 * colorU);
            colorB = (y1192 + 2066 * colorU);

            colorR = clamp(colorR, 0, 262143);
            colorG = clamp(colorG, 0, 262143);
            colorB = clamp(colorB, 0, 262143);


            /// Combine R, G, B, and A into the final pixel color
 
            bitmapContent[yIndex] = color(colorR, colorG, colorB);
            bitmapContent[yIndex] &= pFilter;
        }
   }


   /// Release Java byte buffer and unlock backing bitmap
   (*pEnv)-> ReleasePrimitiveArrayCritical(pEnv,pSource,source,0);
   if (AndroidBitmap_unlockPixels(pEnv, pTarget) < 0) abort();

}
