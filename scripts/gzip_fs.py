Import("env")

import gzip
import os

# Text-based assets that usually compress well.
GZIP_EXTENSIONS = {
    ".htm",
    ".html",
    ".css",
    ".js",
    ".json",
    ".xml",
    ".svg",
    ".txt",
}

def gzip_filesystem_assets(source, target, env, **kwargs):
    data_dir = env.subst("$PROJECT_DATA_DIR")

    print("\n[gzip-fs] Generating compressed web assets")
    print(f"[gzip-fs] Source directory: {data_dir}")

    if not os.path.isdir(data_dir):
        raise RuntimeError(
            f"[gzip-fs] Filesystem directory not found: {data_dir}"
        )

    generated = 0
    skipped = 0

    for root, _, files in os.walk(data_dir):
        for filename in files:
            source_path = os.path.join(root, filename)

            # Avoid compressing generated files again.
            if filename.endswith(".gz"):
                continue

            _, extension = os.path.splitext(filename)
            if extension.lower() not in GZIP_EXTENSIONS:
                continue

            gzip_path = source_path + ".gz"

            # Skip if the compressed copy is newer than its source.
            if (
                os.path.isfile(gzip_path)
                and os.path.getmtime(gzip_path) >= os.path.getmtime(source_path)
            ):
                skipped += 1
                continue

            source_size = os.path.getsize(source_path)

            with open(source_path, "rb") as src:
                with open(gzip_path, "wb") as raw_dst:
                    with gzip.GzipFile(
                        filename="",
                        mode="wb",
                        fileobj=raw_dst,
                        mtime=0,
                    ) as dst:
                        while True:
                            chunk = src.read(8192)
                            if not chunk:
                                break
                            dst.write(chunk)

            compressed_size = os.path.getsize(gzip_path)
            saved = 100 * (1 - compressed_size / source_size) if source_size else 0

            print(
                f"[gzip-fs] {filename}: "
                f"{source_size} -> {compressed_size} bytes "
                f"({saved:.1f}% smaller)"
            )
            generated += 1

    print(
        f"[gzip-fs] Done: {generated} generated, "
        f"{skipped} already up to date"
    )

# Run before building the filesystem image.
env.AddPreAction("$BUILD_DIR/littlefs.bin", gzip_filesystem_assets)
