package com.vegen.bankcardocr

import android.graphics.Bitmap

object BandCardOcrUtil {
    init {
        System.loadLibrary("native-lib")
    }

    /**
     * 三种滤波模糊
     */
    external fun test(bitmap: Bitmap): Int

}
