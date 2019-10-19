package com.vegen.bankcardocr

import android.graphics.Bitmap
import android.Manifest
import android.graphics.BitmapFactory
import android.os.Build
import android.support.v7.app.AppCompatActivity
import android.os.Bundle
import android.widget.Toast
import com.tbruyelle.rxpermissions2.RxPermissions
import io.reactivex.Observable
import io.reactivex.android.schedulers.AndroidSchedulers
import io.reactivex.schedulers.Schedulers
import kotlinx.android.synthetic.main.activity_main.*

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            RxPermissions(this).request(Manifest.permission.READ_EXTERNAL_STORAGE, Manifest.permission.WRITE_EXTERNAL_STORAGE)
                .subscribe {
                    if (it) {
                        //Toast.makeText(this, "欢迎学习 OpenCV", Toast.LENGTH_SHORT).show()
                        var mCardBitmap = BitmapFactory.decodeResource(resources, R.mipmap.bank_card)
                        bandCardOcr(mCardBitmap)
                    } else {
                        Toast.makeText(this, "请赋予本项目必要的权限", Toast.LENGTH_LONG).show()
                        finish()
                    }
                }
        } else {
            //Toast.makeText(this, "欢迎学习 OpenCV", Toast.LENGTH_SHORT).show()
            var mCardBitmap = BitmapFactory.decodeResource(resources, R.mipmap.bank_card)
            bandCardOcr(mCardBitmap)
        }
    }

    private fun bandCardOcr(bitmap: Bitmap) {
        Toast.makeText(this, "开始识别图片", Toast.LENGTH_SHORT).show()
        Observable.just(bitmap)
            .map {
                BandCardOcrUtil.bandCardOcr(it)
                it
            }
            .subscribeOn(Schedulers.newThread())
            .observeOn(AndroidSchedulers.mainThread())
            .subscribe {
                Toast.makeText(this, "识别完成", Toast.LENGTH_SHORT).show()
                sample_img.setImageBitmap(it)
            }
    }
}
