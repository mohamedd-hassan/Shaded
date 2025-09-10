package io.sifr.shaded.samples

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.BasicText
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.input.key.onKeyEvent
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.TextStyle
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import coil.compose.AsyncImage
import coil.request.ImageRequest
import io.sifr.shaded.R
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment
import io.sifr.shaded.modifiers.blur

@Composable
fun BlurSample() {
    Box(
        modifier = Modifier
            .size(200.dp, 100.dp)
            .background(
                brush = Brush.horizontalGradient(
                    colors = listOf(
                        Color.Blue,
                        Color.Cyan,
                        Color.Magenta
                    )
                ),
                shape = RoundedCornerShape(16.dp)
            )
            // Here we pass the radius as Float instead. it gives the same effect as the native one (30F will look the same here and in the native modifier)
            .blur(
                radius = 30F,
                edgeTreatment = BlurEdgeTreatment.UNBOUNDED
            ),
        contentAlignment = Alignment.Center
    ) {
        BasicText(
            text = "Test Content",
            color = { Color.White },
            style = TextStyle(
                fontWeight = FontWeight.Bold,
                fontSize = 24.sp
            )
        )
    }
}

@Composable
fun CoilBlurSample(){
    val context = LocalContext.current
    AsyncImage(
        model = ImageRequest
            .Builder(context)
            // Here for example we turn coil hardware acceleration off so the blur can be applied on old APIS
            .allowHardware(false)
            .data("IMAGE DATA")
            .build()
        ,
        contentDescription = null,
        modifier = Modifier
            .fillMaxWidth()
            .blur(
                5f,
                BlurEdgeTreatment.UNBOUNDED
            )
    )
}