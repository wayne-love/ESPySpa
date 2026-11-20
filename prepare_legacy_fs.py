
Import("env")

import os
import shutil

project_dir = env.subst("$PROJECT_DIR")
environment = env.subst("$PIOENV")

# Only alter the filesystem source for the legacy environment.
if environment != "espa-v2-legacy":
    print("[legacy-fs] Skipping non-legacy environment")
else:
    source_dir = os.path.join(project_dir, "data")
    staging_dir = os.path.join(project_dir, ".pio", "legacy-data")

    if not os.path.isdir(source_dir):
        raise RuntimeError(
            "[legacy-fs] Source data directory not found: "
            + source_dir
        )

    # Rebuild the staging directory from scratch on every run.
    if os.path.exists(staging_dir):
        shutil.rmtree(staging_dir)

    shutil.copytree(source_dir, staging_dir)

    # Exclude vendor assets from the legacy filesystem image.
    vendor_dir = os.path.join(
        staging_dir, "www", "vendor"
    )

    if os.path.isdir(vendor_dir):
        shutil.rmtree(vendor_dir)
        print("[legacy-fs] Removed www/vendor from staged filesystem")

    # Point the ESP32 filesystem builder at the staged directory.
    env.Replace(PROJECT_DATA_DIR=staging_dir)

    print("[legacy-fs] Filesystem source: " + staging_dir)
