package io.sifr.samples.ui

import android.os.Build
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.FilterChip
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Slider
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableFloatStateOf
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.remember
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Brush
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import io.sifr.shaded.blurProcessor.BlurEdgeTreatment
import io.sifr.shaded.modifiers.blur

@Composable
fun CustomBlurComparisonTest() {
    var blurRadius by remember { mutableFloatStateOf(0f) }
    var edgeTreatment by remember { mutableStateOf(BlurEdgeTreatment.RECTANGLE) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .statusBarsPadding()
            .navigationBarsPadding()
            .padding(16.dp)
            .verticalScroll(rememberScrollState()),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Card(
            modifier = Modifier.fillMaxWidth(),
            elevation = CardDefaults.cardElevation(defaultElevation = 4.dp)
        ) {
            Column(
                modifier = Modifier.padding(16.dp),
                verticalArrangement = Arrangement.spacedBy(8.dp)
            ) {
                Text(
                    text = "Blur Controls",
                    style = MaterialTheme.typography.headlineSmall
                )

                Text(text = "Radius: ${blurRadius.toInt()}dp")
                Slider(
                    value = blurRadius,
                    onValueChange = { blurRadius = it },
                    valueRange = 0f..25f,
                    modifier = Modifier.fillMaxWidth()
                )

                Text(text = "Edge Treatment")
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.spacedBy(8.dp)
                ) {
                    FilterChip(
                        selected = edgeTreatment == BlurEdgeTreatment.RECTANGLE,
                        onClick = { edgeTreatment = BlurEdgeTreatment.RECTANGLE },
                        label = { Text("Rectangle") }
                    )
                    FilterChip(
                        selected = edgeTreatment == BlurEdgeTreatment.UNBOUNDED,
                        onClick = { edgeTreatment = BlurEdgeTreatment.UNBOUNDED },
                        label = { Text("Unbounded") }
                    )
                }
            }
        }

        // Test content for blurring
        @Composable
        fun TestContent(
            modifier: Modifier
        ) {
            Box(
                modifier = modifier
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
                    ),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "Test Content",
                    color = Color.White,
                    style = MaterialTheme.typography.headlineSmall,
                    fontWeight = FontWeight.Bold
                )
            }
        }

        // Native blur comparison (API 31+)
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            Text(
                text = "Native Blur (API 31+)",
                style = MaterialTheme.typography.headlineSmall,
                color = Color.Green
            )

            Box(
                modifier = Modifier
                    .fillMaxWidth()
                    .background(Color.LightGray.copy(alpha = 0.3f))
                    .padding(32.dp),
                contentAlignment = Alignment.Center
            ) {
                Row(
                    modifier = Modifier.fillMaxWidth(),
                    horizontalArrangement = Arrangement.Center
                ) {
                    Box(
                        modifier = Modifier
                            .size(100.dp, 200.dp)
                            .background(Color.Red.copy(alpha = 0.3f))
                    )
                    TestContent(
                        modifier = Modifier.blur(blurRadius, edgeTreatment)
                    )
                }
            }
        }

        // Your custom blur
        Text(
            text = "Custom OpenGL Blur",
            style = MaterialTheme.typography.headlineSmall,
            color = Color.Blue
        )

        Box(
            modifier = Modifier
                .fillMaxWidth()
                .background(Color.LightGray.copy(alpha = 0.3f))
                .padding(32.dp),
            contentAlignment = Alignment.Center
        ) {
            Row(
                modifier = Modifier.fillMaxWidth(),
                horizontalArrangement = Arrangement.Center
            ) {
                Box(
                    modifier = Modifier
                        .size(100.dp, 200.dp)
                        .background(Color.Red.copy(alpha = 0.3f))
                )
                TestContent(
                    modifier = Modifier.blur(blurRadius, edgeTreatment)
                )
            }
        }

        // Additional test - Text with different backgrounds
        Text(
            text = "Text Blur Test",
            style = MaterialTheme.typography.headlineSmall
        )

        Row(
            modifier = Modifier.fillMaxWidth(),
            horizontalArrangement = Arrangement.spacedBy(16.dp)
        ) {
            // Native (if available)
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
                Column(
                    modifier = Modifier.weight(1f),
                    horizontalAlignment = Alignment.CenterHorizontally
                ) {
                    Text("Native", style = MaterialTheme.typography.bodySmall)
                    Box(
                        modifier = Modifier
                            .size(120.dp)
                            .background(
                                brush = Brush.radialGradient(
                                    colors = listOf(Color.Red, Color.Yellow, Color.Green)
                                )
                            ),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = "BLUR",
                            color = Color.White,
                            style = MaterialTheme.typography.headlineMedium,
                            fontWeight = FontWeight.Bold,
                            modifier = Modifier.blur(blurRadius, edgeTreatment)
                        )
                    }
                }
            }

            // Custom
            Column(
                modifier = Modifier.weight(1f),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                Text("Custom", style = MaterialTheme.typography.bodySmall)
                Box(
                    modifier = Modifier
                        .size(120.dp)
                        .background(
                            brush = Brush.radialGradient(
                                colors = listOf(Color.Red, Color.Yellow, Color.Green)
                            )
                        ),
                    contentAlignment = Alignment.Center
                ) {
                    Text(
                        text = "BLUR",
                        color = Color.White,
                        style = MaterialTheme.typography.headlineMedium,
                        fontWeight = FontWeight.Bold,
                        modifier = Modifier.blur(blurRadius, edgeTreatment)
                    )
                }
            }
        }

        Text(
            text = "Multiple Elements Test",
            style = MaterialTheme.typography.headlineSmall
        )

        Box(
            modifier = Modifier
                .fillMaxWidth()
                .height(200.dp)
                .background(Color.Black.copy(alpha = 0.1f))
        ) {

            Box(
                modifier = Modifier
                    .size(80.dp)
                    .align(Alignment.TopStart)
                    .background(Color.Red, CircleShape)
            )

            Box(
                modifier = Modifier
                    .size(80.dp)
                    .align(Alignment.TopEnd)
                    .background(Color.Blue, CircleShape)
            )

            Box(
                modifier = Modifier
                    .size(80.dp)
                    .align(Alignment.BottomStart)
                    .background(Color.Green, CircleShape)
            )

            Box(
                modifier = Modifier
                    .size(80.dp)
                    .align(Alignment.BottomEnd)
                    .background(Color.Yellow, CircleShape)
            )

            // Blurred center element
            Box(
                modifier = Modifier
                    .size(100.dp)
                    .align(Alignment.Center)
                    .background(Color.Magenta, RoundedCornerShape(16.dp))
                    .blur(blurRadius, edgeTreatment),
                contentAlignment = Alignment.Center
            ) {
                Text(
                    text = "CENTER",
                    color = Color.White,
                    fontWeight = FontWeight.Bold
                )
            }
        }
    }
}