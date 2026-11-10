/*
 * Copyright (C) 2026 Zhou Qiankang <wszqkzqk@qq.com>
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 *
 * This file is part of PvZ-Portable.
 *
 * PvZ-Portable is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PvZ-Portable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PvZ-Portable. If not, see <https://www.gnu.org/licenses/>.
 */

package io.github.wszqkzqk.pvzportable

import android.content.Intent
import android.net.Uri
import android.os.Bundle
import android.util.Log
import android.widget.Toast
import androidx.activity.compose.setContent
import androidx.activity.result.ActivityResultLauncher
import androidx.activity.result.contract.ActivityResultContracts
import androidx.appcompat.app.AppCompatActivity
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.safeDrawingPadding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.CircleShape
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.Card
import androidx.compose.material3.CardDefaults
import androidx.compose.material3.CircularProgressIndicator
import androidx.compose.material3.LinearProgressIndicator
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.OutlinedButton
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.documentfile.provider.DocumentFile

import java.io.BufferedOutputStream
import java.io.File
import java.io.FileOutputStream
import java.io.IOException
import java.io.InputStream
import java.io.OutputStream
import java.util.zip.ZipEntry
import java.util.zip.ZipInputStream

/**
 * Resource import and save data management via SAF.
 * No runtime storage permissions required.
 * Jetpack Compose UI (PvZ dark-green theme).
 */
class ResourceImportActivity : AppCompatActivity() {
    private companion object {
        const val TAG = "ResImport"
        const val BUFFER_SIZE = 8192
        // Cloudreve share (pan.huang1111.cn) — public download link, no CF challenge, no 123pan CDN limit
        const val CR_BASE = "https://pan.huang1111.cn"
        const val CR_SHARE_KEY = "E7EaLHb"
        const val CR_PATH = "/pvz-portable-resources.zip"
        const val CR_REFERER = "$CR_BASE/s/$CR_SHARE_KEY"
        const val DL_UA = "Mozilla/5.0 (Linux; Android 13; Pixel 7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36"
    }

    private var gameDir: File? = null

    // Compose-observable UI state (Activity-scoped snapshot state)
    private var working by mutableStateOf(false)
    private var ready by mutableStateOf(false)
    private var checkingResources by mutableStateOf(false)
    private var resourcesNeedUpdate by mutableStateOf(false)
    private var statusGeneration = 0
    private var hasSave by mutableStateOf(false)
    private var downloadProgress by mutableStateOf<Int?>(null)

    private val zipPicker: ActivityResultLauncher<Array<String>> =
        registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
            if (uri != null) importFromZip(uri)
        }

    private val dirPicker: ActivityResultLauncher<Uri?> =
        registerForActivityResult(ActivityResultContracts.OpenDocumentTree()) { uri ->
            if (uri != null) importFromDirectory(uri)
        }

    private val saveExporter: ActivityResultLauncher<String> =
        registerForActivityResult(ActivityResultContracts.CreateDocument("application/zip")) { uri ->
            if (uri != null) exportSaveData(uri)
        }

    private val saveZipImporter: ActivityResultLauncher<Array<String>> =
        registerForActivityResult(ActivityResultContracts.OpenDocument()) { uri ->
            if (uri != null) importSaveDataFromZip(uri)
        }

    private val saveDirImporter: ActivityResultLauncher<Uri?> =
        registerForActivityResult(ActivityResultContracts.OpenDocumentTree()) { uri ->
            if (uri != null) importSaveDataFromDir(uri)
        }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        gameDir = getExternalFilesDir(null)
        gameDir?.let { if (!it.exists()) it.mkdirs() }

        refreshStatus()

        setContent {
            PvZTheme {
                LauncherScreen()
            }
        }
    }

    // ------------------------------------------------------------------
    // UI
    // ------------------------------------------------------------------

    @Composable
    private fun LauncherScreen() {
        Surface(
            modifier = Modifier.fillMaxSize(),
            color = PvZGreenBg
        ) {
            Column(
                modifier = Modifier
                    .fillMaxSize()
                    .safeDrawingPadding()
                    .verticalScroll(rememberScrollState())
                    .padding(horizontal = 24.dp, vertical = 32.dp),
                horizontalAlignment = Alignment.CenterHorizontally
            ) {
                // Title
                Text(
                    text = getString(R.string.app_name),
                    fontSize = 30.sp,
                    fontWeight = FontWeight.Bold,
                    color = Color.White,
                    modifier = Modifier.padding(bottom = 4.dp)
                )
                Text(
                    text = getString(R.string.tagline),
                    fontSize = 13.sp,
                    color = PvZGreenLight.copy(alpha = 0.8f),
                    modifier = Modifier.padding(bottom = 20.dp)
                )

                // Status card
                StatusCard()

                Spacer(Modifier.height(20.dp))

                // Import game resources section
                SectionTitle(getString(R.string.section_import_resources))
                Spacer(Modifier.height(6.dp))
                Text(
                    text = getString(R.string.import_help),
                    fontSize = 13.sp,
                    color = Color.White.copy(alpha = 0.7f),
                    modifier = Modifier.padding(bottom = 12.dp)
                )
                // Auto-download from OpenList
                Button(
                    onClick = { autoDownloadResources() },
                    enabled = !working && !ready,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(48.dp),
                    shape = RoundedCornerShape(14.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = PvZGreenLight,
                        contentColor = Color(0xFF0F3D13),
                        disabledContainerColor = PvZGreenDark,
                        disabledContentColor = Color.White.copy(alpha = 0.35f)
                    )
                ) {
                    Text(getString(if (resourcesNeedUpdate) R.string.btn_update_resources else R.string.btn_auto_download), fontSize = 15.sp, fontWeight = FontWeight.SemiBold)
                }
                if (downloadProgress != null) {
                    Spacer(Modifier.height(10.dp))
                    Text(
                        text = getString(R.string.download_progress, downloadProgress!!),
                        fontSize = 13.sp,
                        color = Color.White.copy(alpha = 0.85f),
                        modifier = Modifier.fillMaxWidth()
                    )
                    Spacer(Modifier.height(6.dp))
                    LinearProgressIndicator(
                        progress = { downloadProgress!! / 100f },
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(6.dp),
                        color = PvZSunYellow,
                        trackColor = PvZGreenDark
                    )
                    Spacer(Modifier.height(12.dp))
                }
                Row(horizontalArrangement = Arrangement.spacedBy(12.dp)) {
                    OutlinedButton(
                        onClick = {
                            zipPicker.launch(arrayOf("application/zip", "application/x-zip-compressed"))
                        },
                        enabled = !working,
                        modifier = Modifier.weight(1f),
                        colors = ButtonDefaults.outlinedButtonColors(contentColor = Color.White)
                    ) {
                        Text(getString(R.string.btn_pick_zip), fontSize = 14.sp)
                    }
                    OutlinedButton(
                        onClick = { dirPicker.launch(null) },
                        enabled = !working,
                        modifier = Modifier.weight(1f),
                        colors = ButtonDefaults.outlinedButtonColors(contentColor = Color.White)
                    ) {
                        Text(getString(R.string.btn_pick_dir), fontSize = 14.sp)
                    }
                }

                Spacer(Modifier.height(24.dp))

                // Save data section
                SectionTitle(getString(R.string.section_save_data))
                Spacer(Modifier.height(10.dp))
                Row(horizontalArrangement = Arrangement.spacedBy(8.dp)) {
                    ActionButton(
                        text = getString(R.string.btn_export_save),
                        enabled = !working && hasSave,
                        onClick = { saveExporter.launch("pvz-portable-savedata.zip") },
                        modifier = Modifier.weight(1f)
                    )
                    ActionButton(
                        text = getString(R.string.btn_import_save_zip),
                        enabled = !working,
                        onClick = {
                            saveZipImporter.launch(arrayOf("application/zip", "application/x-zip-compressed"))
                        },
                        modifier = Modifier.weight(1f)
                    )
                }
                Spacer(Modifier.height(8.dp))
                ActionButton(
                    text = getString(R.string.btn_import_save_dir),
                    enabled = !working,
                    onClick = { saveDirImporter.launch(null) },
                    modifier = Modifier.fillMaxWidth()
                )

                Spacer(Modifier.height(28.dp))

                // Progress
                if (working) {
                    CircularProgressIndicator(
                        color = PvZSunYellow,
                        strokeWidth = 3.dp,
                        modifier = Modifier
                            .size(36.dp)
                            .padding(bottom = 20.dp)
                    )
                    Spacer(Modifier.height(16.dp))
                }

                // Launch button
                Button(
                    onClick = { launchGame() },
                    enabled = ready && !working,
                    modifier = Modifier
                        .fillMaxWidth()
                        .height(56.dp),
                    shape = RoundedCornerShape(16.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = PvZSunYellow,
                        contentColor = Color.Black,
                        disabledContainerColor = PvZGreenDark,
                        disabledContentColor = Color.White.copy(alpha = 0.4f)
                    )
                ) {
                    Text(
                        text = getString(R.string.btn_launch_game),
                        fontSize = 18.sp,
                        fontWeight = FontWeight.Bold
                    )
                }
            }
        }
    }

    @Composable
    private fun StatusCard() {
        Card(
            modifier = Modifier.fillMaxWidth(),
            shape = RoundedCornerShape(16.dp),
            colors = CardDefaults.cardColors(containerColor = PvZGreenCard)
        ) {
            Row(
                modifier = Modifier
                    .fillMaxWidth()
                    .padding(16.dp),
                verticalAlignment = Alignment.CenterVertically
            ) {
                Box(
                    modifier = Modifier
                        .size(12.dp)
                        .background(
                            color = if (ready) PvZSunYellow else Color(0xFFE53935),
                            shape = CircleShape
                        )
                )
                Spacer(Modifier.width(12.dp))
                Text(
                    text = getString(when {
                        checkingResources -> R.string.status_checking_resources
                        ready -> R.string.status_ready
                        resourcesNeedUpdate -> R.string.status_resources_outdated
                        else -> R.string.status_missing
                    }),
                    fontSize = 14.sp,
                    color = Color.White,
                    fontWeight = if (ready) FontWeight.SemiBold else FontWeight.Normal
                )
            }
        }
    }

    @Composable
    private fun SectionTitle(text: String) {
        Text(
            text = text,
            fontSize = 16.sp,
            fontWeight = FontWeight.SemiBold,
            color = PvZGreenLight,
            modifier = Modifier
                .fillMaxWidth()
                .padding(bottom = 2.dp)
        )
    }

    @Composable
    private fun ActionButton(
        text: String,
        enabled: Boolean,
        onClick: () -> Unit,
        modifier: Modifier = Modifier
    ) {
        OutlinedButton(
            onClick = onClick,
            enabled = enabled,
            modifier = modifier,
            colors = ButtonDefaults.outlinedButtonColors(
                contentColor = Color.White,
                disabledContentColor = Color.White.copy(alpha = 0.35f)
            )
        ) {
            Text(text, fontSize = 13.sp)
        }
    }

    // ------------------------------------------------------------------
    // Status
    // ------------------------------------------------------------------

    private fun hasSaveData(): Boolean {
        val dir = gameDir ?: return false
        val save = File(dir, "userdata")
        return save.isDirectory() && save.list() != null && save.list()!!.isNotEmpty()
    }

    private fun refreshStatus() {
        hasSave = hasSaveData()
        ready = false
        checkingResources = true
        val generation = ++statusGeneration
        val directory = gameDir
        Thread {
            val status = try {
                assets.open(ResourceIntegrity.MANIFEST_ASSET).use { manifest ->
                    ResourceIntegrity.verify(directory, manifest)
                }
            } catch (e: IOException) {
                Log.e(TAG, "Resource verification failed", e)
                ResourceIntegrity.Status.OUTDATED
            }
            runOnUiThread {
                if (generation != statusGeneration || isFinishing || isDestroyed) return@runOnUiThread
                val wasOutdated = resourcesNeedUpdate
                resourcesNeedUpdate = status == ResourceIntegrity.Status.OUTDATED
                ready = status == ResourceIntegrity.Status.READY
                checkingResources = false
                if (resourcesNeedUpdate && !wasOutdated) {
                    Toast.makeText(this, R.string.resource_update_required, Toast.LENGTH_LONG).show()
                }
            }
        }.start()
    }

    private fun launchGame() {
        if (!ready || working || checkingResources) return
        val intent = Intent(this, PvZPortableActivity::class.java)
        startActivity(intent)
        finish()
    }

    // ------------------------------------------------------------------
    // Auto-download game resources from the Cloudreve share
    // ------------------------------------------------------------------

    private fun autoDownloadResources() {
        if (gameDir == null) return
        updateWorking(true)
        downloadProgress = 0
        Thread {
            try {
                // 1. Fetch download info from public API (raw_url CDN link preferred)
                val info = fetchDownloadInfo()
                // 2. Download to cache
                val tmpZip = File(cacheDir, "pvz-resources.zip")
                downloadFile(info, tmpZip)
                // 3. Extract into game dir
                runOnUiThread { downloadProgress = 100 }
                extractZipToGameDir(tmpZip)
                tmpZip.delete()
                runOnUiThread {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show()
                    downloadProgress = null
                }
            } catch (e: Exception) {
                Log.e(TAG, "Auto download failed", e)
                runOnUiThread {
                    Toast.makeText(this, getString(R.string.import_failed, e.message), Toast.LENGTH_LONG).show()
                    downloadProgress = null
                }
            } finally {
                runOnUiThread { updateWorking(false) }
            }
        }.start()
    }

    private data class DownloadInfo(val url: String, val sign: String)

    /**
     * Resolve the actual download URL from a Cloudreve share link.
     * Flow: PUT /api/v3/share/download/{key}?path={file} -> JSON {"data": "<direct url>"}
     */
    private fun fetchDownloadInfo(): DownloadInfo {
        val url = java.net.URL("$CR_BASE/api/v3/share/download/$CR_SHARE_KEY?path=" +
                java.net.URLEncoder.encode(CR_PATH, "UTF-8"))
        val conn = url.openConnection() as java.net.HttpURLConnection
        try {
            conn.requestMethod = "PUT"
            conn.connectTimeout = 15000
            conn.readTimeout = 15000
            conn.setRequestProperty("Accept", "application/json, text/plain, */*")
            conn.setRequestProperty("Referer", CR_REFERER)
            conn.setRequestProperty("User-Agent", DL_UA)
            val code = conn.responseCode
            if (code != 200) {
                val err = conn.errorStream?.bufferedReader(Charsets.UTF_8)?.use { it.readText() } ?: ""
                throw IOException("HTTP $code: ${err.take(120)}")
            }
            val resp = conn.inputStream.bufferedReader(Charsets.UTF_8).use { it.readText() }
            val obj = org.json.JSONObject(resp)
            if (obj.optInt("code") != 0) {
                throw IOException("Cloudreve error: ${obj.optString("msg")}")
            }
            val dl = obj.optString("data")
            if (dl.isEmpty()) throw IOException("Cloudreve returned no download link")
            // Direct URL may be plain http (Android blocks cleartext by default) — force https
            val safeUrl = if (dl.startsWith("http://")) "https://" + dl.substring(7) else dl
            return DownloadInfo(safeUrl, "")
        } finally {
            conn.disconnect()
        }
    }

    private fun downloadFile(info: DownloadInfo, dest: File) {
        val url = java.net.URL(info.url)
        val conn = url.openConnection() as java.net.HttpURLConnection
        try {
            conn.requestMethod = "GET"
            conn.connectTimeout = 15000
            conn.readTimeout = 60000
            conn.setRequestProperty("User-Agent", DL_UA)
            conn.setRequestProperty("Accept", "*/*")
            conn.setRequestProperty("Referer", CR_REFERER)
            conn.instanceFollowRedirects = true
            val code = conn.responseCode
            if (code != 200) {
                val err = conn.errorStream?.bufferedReader(Charsets.UTF_8)?.use { it.readText() } ?: ""
                throw IOException("Download HTTP $code: ${err.take(120)}")
            }
            val total = conn.contentLengthLong
            var downloaded = 0L
            conn.inputStream.use { input ->
                dest.outputStream().use { output ->
                    val buf = ByteArray(8192)
                    var lastReport = 0
                    while (true) {
                        val n = input.read(buf)
                        if (n < 0) break
                        output.write(buf, 0, n)
                        downloaded += n
                        if (total > 0) {
                            val pct = ((downloaded * 100) / total).toInt()
                            if (pct != lastReport) {
                                lastReport = pct
                                runOnUiThread { downloadProgress = pct }
                            }
                        }
                    }
                }
            }
            if (total > 0 && downloaded < total) throw IOException("Incomplete download")
        } finally {
            conn.disconnect()
        }
    }

    private fun extractZipToGameDir(zipFile: File) {
        ZipInputStream(zipFile.inputStream()).use { zis ->
            var entry: ZipEntry?
            while (zis.nextEntry.also { entry = it } != null) {
                val e = entry!!
                if (!e.isDirectory) {
                    val name = stripCommonPrefix(e.name)
                    if (name != null) {
                        val outFile = File(gameDir, name)
                        val parent = outFile.parentFile
                        if (parent != null && !parent.exists()) parent.mkdirs()
                        BufferedOutputStream(FileOutputStream(outFile), BUFFER_SIZE).use { os ->
                            val buf = ByteArray(BUFFER_SIZE)
                            var len: Int
                            while (zis.read(buf).also { len = it } > 0) os.write(buf, 0, len)
                        }
                    }
                }
                zis.closeEntry()
            }
        }
    }

    // ------------------------------------------------------------------
    // Import game resources
    // ------------------------------------------------------------------

    private fun importFromZip(uri: Uri) {
        updateWorking(true)
        Thread {
            try {
                getContentResolver().openInputStream(uri).use { is0 ->
                    ZipInputStream(is0).use { zis ->
                        var entry: ZipEntry?
                        while (zis.nextEntry.also { entry = it } != null) {
                            val e = entry!!
                            if (!e.isDirectory) {
                                val name = stripCommonPrefix(e.name)
                                if (name != null) {
                                    val outFile = File(gameDir!!, name)
                                    val parent = outFile.parentFile
                                    if (parent != null && !parent.exists()) parent.mkdirs()
                                    BufferedOutputStream(FileOutputStream(outFile), BUFFER_SIZE).use { os ->
                                        val buf = ByteArray(BUFFER_SIZE)
                                        var len: Int
                                        while (zis.read(buf).also { len = it } > 0) os.write(buf, 0, len)
                                    }
                                }
                            }
                            zis.closeEntry()
                        }
                    }
                }
                runOnUiThread {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show()
                }
            } catch (e: IOException) {
                Log.e(TAG, "ZIP import failed", e)
                runOnUiThread {
                    Toast.makeText(this, getString(R.string.import_failed, e.message), Toast.LENGTH_LONG).show()
                }
            } finally {
                runOnUiThread { updateWorking(false) }
            }
        }.start()
    }

    /**
     * Strips a single wrapper directory from zip entry paths when the entry
     * doesn't start with a known top-level name (e.g. "PvZ/main.pak" -> "main.pak").
     */
    private fun stripCommonPrefix(name: String): String? {
        var n = name.replace('\\', '/').replace(Regex("^/+"), "")
        if (isKnownTopLevel(n)) return n
        val slash = n.indexOf('/')
        if (slash > 0 && slash < n.length - 1) {
            return n.substring(slash + 1)
        }
        return null
    }

    private fun isKnownTopLevel(name: String): Boolean {
        return name.startsWith("main.pak") || name.startsWith("properties/") ||
                name.startsWith("Properties/") || name.startsWith("data/") ||
                name.startsWith("images/") || name.startsWith("particles/") ||
                name.startsWith("reanim/") || name.startsWith("sounds/") ||
                name.startsWith("compiled/")
    }

    private fun importFromDirectory(treeUri: Uri) {
        updateWorking(true)
        Thread {
            try {
                val root = DocumentFile.fromTreeUri(this, treeUri)
                    ?: throw IOException("Cannot open directory")

                var sourceDir = root
                if (root.findFile("main.pak") == null) {
                    for (child in root.listFiles()) {
                        if (child.isDirectory) {
                            if (child.findFile("main.pak") != null) {
                                sourceDir = child
                                break
                            }
                        }
                    }
                }
                copyDocumentTree(sourceDir, gameDir!!)
                runOnUiThread {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show()
                }
            } catch (e: IOException) {
                Log.e(TAG, "Directory import failed", e)
                runOnUiThread {
                    Toast.makeText(this, getString(R.string.import_failed, e.message), Toast.LENGTH_LONG).show()
                }
            } finally {
                runOnUiThread { updateWorking(false) }
            }
        }.start()
    }

    private fun copyDocumentTree(src: DocumentFile, destDir: File) {
        if (!destDir.exists()) destDir.mkdirs()
        for (child in src.listFiles()) {
            if (child.isDirectory) {
                val name = child.name ?: continue
                copyDocumentTree(child, File(destDir, name))
            } else {
                val name = child.name ?: continue
                val outFile = File(destDir, name)
                val is0 = getContentResolver().openInputStream(child.uri) ?: continue
                is0.use {
                    BufferedOutputStream(FileOutputStream(outFile), BUFFER_SIZE).use { os ->
                        val buf = ByteArray(BUFFER_SIZE)
                        var len: Int
                        while (it.read(buf).also { len = it } > 0) os.write(buf, 0, len)
                    }
                }
            }
        }
    }

    // ------------------------------------------------------------------
    // Save data export / import
    // ------------------------------------------------------------------

    private fun exportSaveData(destUri: Uri) {
        updateWorking(true)
        Thread {
            val saveDir = File(gameDir!!, "userdata")
            if (!saveDir.isDirectory()) {
                runOnUiThread {
                    Toast.makeText(this, R.string.no_save_data, Toast.LENGTH_SHORT).show()
                    updateWorking(false)
                }
                return@Thread
            }
            try {
                getContentResolver().openOutputStream(destUri).use { os ->
                    java.util.zip.ZipOutputStream(os).use { zos ->
                        addDirToZip(zos, saveDir, "userdata/")
                    }
                }
                runOnUiThread {
                    Toast.makeText(this, R.string.export_success, Toast.LENGTH_SHORT).show()
                }
            } catch (e: IOException) {
                Log.e(TAG, "Save export failed", e)
                runOnUiThread {
                    Toast.makeText(this, getString(R.string.export_failed, e.message), Toast.LENGTH_LONG).show()
                }
            } finally {
                runOnUiThread { updateWorking(false) }
            }
        }.start()
    }

    private fun addDirToZip(zos: java.util.zip.ZipOutputStream, dir: File, prefix: String) {
        val files = dir.listFiles() ?: return
        for (f in files) {
            if (f.isDirectory) {
                addDirToZip(zos, f, prefix + f.name + "/")
            } else {
                zos.putNextEntry(ZipEntry(prefix + f.name))
                java.io.FileInputStream(f).use { is0 ->
                    val buf = ByteArray(BUFFER_SIZE)
                    var len: Int
                    while (is0.read(buf).also { len = it } > 0) zos.write(buf, 0, len)
                }
                zos.closeEntry()
            }
        }
    }

    private fun importSaveDataFromDir(treeUri: Uri) {
        updateWorking(true)
        Thread {
            try {
                val root = DocumentFile.fromTreeUri(this, treeUri)
                    ?: throw IOException("Cannot open directory")
                var sourceDir = root
                val destDir = File(gameDir!!, "userdata")
                if ("userdata" != root.name) {
                    val nested = root.findFile("userdata")
                    if (nested != null && nested.isDirectory) {
                        sourceDir = nested
                    }
                }
                copyDocumentTree(sourceDir, destDir)
                runOnUiThread {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show()
                }
            } catch (e: IOException) {
                Log.e(TAG, "Save directory import failed", e)
                runOnUiThread {
                    Toast.makeText(this, getString(R.string.import_failed, e.message), Toast.LENGTH_LONG).show()
                }
            } finally {
                runOnUiThread { updateWorking(false) }
            }
        }.start()
    }

    private fun importSaveDataFromZip(uri: Uri) {
        updateWorking(true)
        Thread {
            try {
                getContentResolver().openInputStream(uri).use { is0 ->
                    ZipInputStream(is0).use { zis ->
                        var entry: ZipEntry?
                        while (zis.nextEntry.also { entry = it } != null) {
                            val e = entry!!
                            if (!e.isDirectory) {
                                var name = e.name
                                if (!name.startsWith("userdata/")) name = "userdata/" + name
                                val outFile = File(gameDir!!, name)
                                val parent = outFile.parentFile
                                if (parent != null && !parent.exists()) parent.mkdirs()
                                BufferedOutputStream(FileOutputStream(outFile), BUFFER_SIZE).use { os ->
                                    val buf = ByteArray(BUFFER_SIZE)
                                    var len: Int
                                    while (zis.read(buf).also { len = it } > 0) os.write(buf, 0, len)
                                }
                            }
                            zis.closeEntry()
                        }
                    }
                }
                runOnUiThread {
                    Toast.makeText(this, R.string.import_success, Toast.LENGTH_SHORT).show()
                }
            } catch (e: IOException) {
                Log.e(TAG, "Save import failed", e)
                runOnUiThread {
                    Toast.makeText(this, getString(R.string.import_failed, e.message), Toast.LENGTH_LONG).show()
                }
            } finally {
                runOnUiThread { updateWorking(false) }
            }
        }.start()
    }

    private fun updateWorking(working: Boolean) {
        this.working = working
        if (working) {
            // Discard any check begun before an import starts changing the files.
            ++statusGeneration
            ready = false
            checkingResources = false
        } else {
            refreshStatus()
        }
    }
}
