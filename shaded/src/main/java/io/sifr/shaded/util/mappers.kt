package io.sifr.shaded.util

import androidx.compose.ui.draw.BlurredEdgeTreatment
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment

internal fun BlurEdgeTreatment.toBlurredEdgeTreatment(): BlurredEdgeTreatment =
    if (this == BlurEdgeTreatment.RECTANGLE) BlurredEdgeTreatment.Rectangle else BlurredEdgeTreatment.Unbounded