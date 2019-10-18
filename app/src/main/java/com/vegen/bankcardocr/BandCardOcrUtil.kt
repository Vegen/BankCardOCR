package com.vegen.bankcardocr

import android.graphics.Bitmap

object BandCardOcrUtil {
    init {
        System.loadLibrary("native-lib")
    }

    /**
     * 银行卡识别
     */
    external fun bandCardOcr(bitmap: Bitmap): Int

}
