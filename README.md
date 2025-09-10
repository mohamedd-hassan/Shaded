
# Shaded

Shaded is a jetpack compose library for Android allowing blurring on lower Android versions


## Screenshots

<img width="357" height="720" alt="Unbounded Blur Comparison" src="https://github.com/user-attachments/assets/864e02f6-a22b-4e33-8e1b-a4dc4a5ef5d7" />
<img width="355" height="720" alt="Bounded Blur Comparison" src="https://github.com/user-attachments/assets/b1596273-588c-4aaa-9de7-5dc3779ba58e" />

# Installation

## Method 1: Direct Dependency

### Step 1: Add JitPack Repository

Add the JitPack repository to your **root** `settings.gradle` file:

```kotlin
dependencyResolutionManagement {
    repositoriesMode.set(RepositoriesMode.FAIL_ON_PROJECT_REPOS)
    repositories {
        mavenCentral()
        maven { url = uri("https://jitpack.io") }
    }
}
```

### Step 2: Add Dependency

Add the dependency to your **module** `build.gradle.kts`:

```kotlin
dependencies {
    implementation("com.github.mohamedd-hassan:Shaded:0.2.3-alpha")
}
```

---

## Method 2: Version Catalog (Recommended)

### Step 1: Add to Version Catalog

Add the dependency to your `libs.versions.toml`:

```toml
[versions]
shaded = "0.2.3-alpha"

[libraries]
sifr-shaded = { module = "com.github.mohamedd-hassan:Shaded", version.ref = "shaded" }
```

### Step 2: Add to Build Script

Add it to your **module** `build.gradle.kts`:

```kotlin
dependencies {
    implementation(libs.sifr.shaded)
}
```
## Features

- Blurring composables on low Android versions (API 24+)
- Unbounded and Bounded blurring just like native blurring
- If on Android 12 and above it automatically uses native blur for performance
- High performance while keeping the same look and effect


## Roadmap

[ ] Better OpenGL context management for improved performance

[ ] Vulkan support for reduced 

[ ] Hardware Accelerated composable support

[ ] Support for custom shader integrations

[ ] Better interoperability with the Android renderer for improved performance

## Usage/Examples

You would use it just like the normal blur modifier that comes with androidx

```
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
```
here while using it with coil we had to disable hardware bitmaps because the library currently does not support hardware accelerated composables

```
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
```

