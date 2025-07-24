package io.sifr.shaded.util

import android.graphics.Bitmap
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment
import io.sifr.shaded.blurProcessor.BlurNative

fun blurBitmap(inputBitmap: Bitmap, radius: Float, blurEdgeTreatment: BlurEdgeTreatment): Bitmap{
    return BlurNative.blurBitmap(inputBitmap, radius, blurEdgeTreatment)
}