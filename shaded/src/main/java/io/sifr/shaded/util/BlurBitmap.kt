package io.sifr.shaded.util

import android.graphics.Bitmap
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment
import io.sifr.shaded.blurProcessor.BlurNative

fun blurBitmap(inputBitmap: Bitmap, radius: Float): Bitmap{
    return BlurNative.blurBitmap(inputBitmap, radius, BlurEdgeTreatment.RECTANGLE)
}