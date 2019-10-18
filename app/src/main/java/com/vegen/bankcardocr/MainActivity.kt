package com.vegen.bankcardocr

import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        var mCardBitmap = BitmapFactory.decodeResource(resources, R.mipmap.timg)
        BandCardOcrUtil.bandCardOcr(mCardBitmap)
    }
}
