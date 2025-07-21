package io.sifr.shaded.blurProcessor

import android.graphics.Bitmap

internal object BlurNative: BlurProcessor {
    init {
        System.loadLibrary("blur_renderer")
    }

    private external fun blurBitmap(bitmap: Bitmap, radius: Float): Bitmap
    private external fun blurBitmapUnbounded(bitmap: Bitmap, radius: Float): Bitmap

    override fun gaussianBlur(inputBitmap: Bitmap, radius: Float, blurEdgeTreatment: BlurEdgeTreatment): Bitmap {
        return when (blurEdgeTreatment) {
            BlurEdgeTreatment.RECTANGLE -> blurBitmap(inputBitmap, radius)
            BlurEdgeTreatment.UNBOUNDED -> blurBitmapUnbounded(inputBitmap, radius)
        }
    }
}