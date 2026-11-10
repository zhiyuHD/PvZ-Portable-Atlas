/*
 * Copyright (C) 2026 PvZ-Portable contributors
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * Jetpack Compose Material3 theme for the PvZ-Portable Android launcher.
 * Dark green palette matching the classic PvZ lawn aesthetic.
 */

package io.github.wszqkzqk.pvzportable

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable
import androidx.compose.ui.graphics.Color

// PvZ palette
val PvZGreenBg = Color(0xFF1B5E20)       // deep lawn green (page background)
val PvZGreenDark = Color(0xFF0F3D13)     // darker green (disabled / depth)
val PvZGreenCard = Color(0xFF2E7D32)     // card surface green
val PvZGreenLight = Color(0xFF81C784)    // light accent green (section titles)
val PvZSunYellow = Color(0xFFFFD54F)     // sun/sunflower yellow (primary actions)

private val PvZColorScheme = darkColorScheme(
    primary = PvZSunYellow,
    onPrimary = Color(0xFF2E1F00),
    primaryContainer = PvZGreenCard,
    onPrimaryContainer = Color.White,
    secondary = PvZGreenLight,
    onSecondary = Color(0xFF0F3D13),
    background = PvZGreenBg,
    onBackground = Color.White,
    surface = PvZGreenBg,
    onSurface = Color.White,
    surfaceVariant = PvZGreenCard,
    onSurfaceVariant = Color.White.copy(alpha = 0.85f),
    outline = Color.White.copy(alpha = 0.55f),
    error = Color(0xFFE53935),
)

@Composable
fun PvZTheme(content: @Composable () -> Unit) {
    MaterialTheme(
        colorScheme = PvZColorScheme,
        content = content
    )
}
