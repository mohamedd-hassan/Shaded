package io.sifr.shaded.util

import android.graphics.Bitmap
import android.graphics.Picture
import androidx.compose.ui.draw.CacheDrawScope
import androidx.compose.ui.graphics.Canvas
import androidx.compose.ui.graphics.drawscope.ContentDrawScope
import androidx.compose.ui.graphics.drawscope.draw

internal fun ContentDrawScope.recordComposable(picture: Picture, drawScope: CacheDrawScope): Bitmap {
    val pictureCanvas = Canvas(
        picture.beginRecording(drawScope.size.width.toInt(), drawScope.size.height.toInt())
    )

    draw(drawScope, drawScope.layoutDirection, pictureCanvas, drawScope.size) {
        this@recordComposable.drawContent()
    }
    picture.endRecording()

    return createBitmapFromPicture(picture)
}