package io.sifr.shaded.util

import android.graphics.Bitmap
import android.graphics.Color
import android.graphics.Picture
import android.graphics.PorterDuff
import androidx.core.graphics.createBitmap

internal fun createBitmapFromPicture(picture: Picture): Bitmap {
    return createBitmap(picture.width, picture.height).apply {
        val canvas = android.graphics.Canvas(this)
        canvas.drawColor(Color.TRANSPARENT, PorterDuff.Mode.CLEAR)
        canvas.drawPicture(picture)
    }
}