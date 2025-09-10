package io.sifr.shaded.modifiers

import android.graphics.Picture
import android.os.Build
import androidx.compose.runtime.NonRestartableComposable
import androidx.compose.runtime.remember
import androidx.compose.ui.Modifier
import androidx.compose.ui.composed
import androidx.compose.ui.draw.blur
import androidx.compose.ui.draw.drawWithCache
import androidx.compose.ui.graphics.drawscope.drawIntoCanvas
import androidx.compose.ui.graphics.nativeCanvas
import androidx.compose.ui.unit.dp
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment
import io.sifr.shaded.blurProcessor.BlurNative
import io.sifr.shaded.util.recordComposable
import io.sifr.shaded.util.toBlurredEdgeTreatment
import io.sifr.shaded.samples.BlurSample
import io.sifr.shaded.samples.CoilBlurSample


/**
 * Draw content blurred with specified radii. This effect can be used all the way down to Api 24.
 * If used on Android 12 and above it will use the native AndroidX Blur modifier
 *
 * NOTE: This currently does not support hardware accelerated Composables. Any use of this modifier on a hardware accelerated composable
 * will result in a blank composable.
 *
 * @param radius Radius of the blur modifier
 * @param edgeTreatment Strategy used to render pixels outside of bounds of the original input
 *
 * @sample BlurSample
 * @sample CoilBlurSample
 */
fun Modifier.blur(
    radius: Float,
    edgeTreatment: BlurEdgeTreatment = BlurEdgeTreatment.RECTANGLE,
): Modifier = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
    val nativeEdgeTreatment = edgeTreatment.toBlurredEdgeTreatment()
    this.blur(radius.dp, nativeEdgeTreatment)
} else if (radius == 0f) {
    this
} else {
    composed {
        val picture = remember { Picture() }

        val blurPadding = if (edgeTreatment == BlurEdgeTreatment.UNBOUNDED) {
            (radius * 4f).toInt()
        } else {
            0
        }

        this.drawWithCache {
            val originalWidth = this.size.width.toInt()
            val originalHeight = this.size.height.toInt()

            onDrawWithContent {
                if (originalWidth > 0 && originalHeight > 0) {

                    val originalBitmap = recordComposable(picture, this@drawWithCache)

                    val blurredBitmap =
                        BlurNative.blurBitmap(originalBitmap, radius * 4f, edgeTreatment)

                    drawIntoCanvas { canvas ->
                        when (edgeTreatment) {
                            BlurEdgeTreatment.RECTANGLE -> {
                                canvas.nativeCanvas.clipRect(
                                    0f, 0f, originalWidth.toFloat(), originalHeight.toFloat()
                                )
                                canvas.nativeCanvas.drawBitmap(
                                    blurredBitmap, 0f, 0f, null
                                )
                            }

                            BlurEdgeTreatment.UNBOUNDED -> {
                                canvas.nativeCanvas.drawBitmap(
                                    blurredBitmap,
                                    -blurPadding.toFloat(),
                                    -blurPadding.toFloat(),
                                    null
                                )
                            }
                        }
                    }

                    if (blurredBitmap != originalBitmap) {
                        blurredBitmap.recycle()
                    }
                    originalBitmap.recycle()
                }
            }
        }
    }
}
