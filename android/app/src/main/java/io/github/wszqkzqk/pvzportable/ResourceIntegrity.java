package io.github.wszqkzqk.pvzportable;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

/** Checks the installed resource files against the manifest bundled with this app. */
public final class ResourceIntegrity {
    public static final String MANIFEST_ASSET = "resource-hashes.sha256";

    public enum Status { MISSING, OUTDATED, READY }

    private ResourceIntegrity() {}

    public static Status verify(File directory, InputStream manifest) throws IOException {
        try (BufferedReader reader = new BufferedReader(
                new InputStreamReader(manifest, StandardCharsets.UTF_8))) {
            if (directory == null || !new File(directory, "main.pak").isFile()
                    || !new File(directory, "properties").isDirectory()) {
                return Status.MISSING;
            }
            String root = directory.getCanonicalPath() + File.separator;
            boolean checkedPak = false;
            boolean checkedProperties = false;
            String line;
            while ((line = reader.readLine()) != null) {
                if (line.isEmpty() || line.startsWith("#")) continue;
                if (!line.matches("[0-9a-f]{64}  .+")) {
                    throw new IOException("Invalid resource hash manifest");
                }
                String expected = line.substring(0, 64);
                String path = line.substring(66);
                File resource = new File(directory, path);
                if (!resource.getCanonicalPath().startsWith(root)) {
                    throw new IOException("Invalid resource path in manifest");
                }
                if (!resource.isFile() || !expected.equals(sha256(resource))) {
                    return Status.OUTDATED;
                }
                checkedPak |= path.equals("main.pak");
                checkedProperties |= path.startsWith("properties/");
            }
            if (!checkedPak || !checkedProperties) {
                throw new IOException("Incomplete resource hash manifest");
            }
            return Status.READY;
        }
    }

    private static String sha256(File file) throws IOException {
        final MessageDigest digest;
        try {
            digest = MessageDigest.getInstance("SHA-256");
        } catch (NoSuchAlgorithmException e) {
            throw new IOException("SHA-256 is unavailable", e);
        }
        try (InputStream input = new FileInputStream(file)) {
            byte[] buffer = new byte[65536];
            int count;
            while ((count = input.read(buffer)) != -1) digest.update(buffer, 0, count);
        }
        char[] hex = "0123456789abcdef".toCharArray();
        StringBuilder result = new StringBuilder(64);
        for (byte value : digest.digest()) {
            result.append(hex[(value & 255) >>> 4]);
            result.append(hex[value & 15]);
        }
        return result.toString();
    }
}
