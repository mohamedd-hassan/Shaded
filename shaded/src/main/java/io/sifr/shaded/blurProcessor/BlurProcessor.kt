package io.sifr.shaded.blurProcessor

import android.graphics.Bitmap
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment

internal interface BlurProcessor {

    fun gaussianBlur(inputBitmap: Bitmap, radius: Float, blurEdgeTreatment: BlurEdgeTreatment): Bitmap

}