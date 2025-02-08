#!/usr/bin/python3

# Adds PlatformIO post-processing to merge all the ESP flash images into a single image.

import os
import csv

Import("env", "projenv")

board_config = env.BoardConfig()
firmware_bin = "${BUILD_DIR}/${PROGNAME}.bin"
merged_bin = os.environ.get("MERGED_BIN_PATH", "${BUILD_DIR}/${PROGNAME}_factory.bin")


def merge_bin_action(source, target, env):
    flash_images = [
        *env.Flatten(env.get("FLASH_EXTRA_IMAGES", [])),
        "$ESP32_APP_OFFSET",
        source[0].get_abspath(),
    ]
    if board_config.get("build.mcu", "") == 'esp8266':
        return

    partition_csv = env.get("PARTITIONS_TABLE_CSV")
    fs_image_name = env.get("ESP32_FS_IMAGE_NAME")
    offset = 0
    print("partition_csv: ", partition_csv)
    print("fs_image_name: ", fs_image_name)

    if partition_csv and fs_image_name:
        with open(partition_csv, "r") as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                if row["# Name"] == fs_image_name:
                    offset = row[" Offset"]
                    break

    if offset:
        flash_images.append(offset)
        flash_images.append("${BUILD_DIR}/%s.bin" % fs_image_name)

    merge_cmd = " ".join(
        [
            '"$PYTHONEXE"',
            '"$OBJCOPY"',
            "--chip",
            board_config.get("build.mcu", "esp32"),
            "merge_bin",
            "-o",
            merged_bin,
            "--flash_mode",
            "${__get_board_flash_mode(__env__)}",
            "--flash_freq",
            "${__get_board_f_flash(__env__)}",
            "--flash_size",
            board_config.get("upload.flash_size", "4MB"),
            *flash_images,
        ]
    )
    env.Execute(merge_cmd)


env.AddCustomTarget(
    name="mergebin",
    dependencies=firmware_bin,
    actions=merge_bin_action,
    title="Merge binary",
    description="Build combined image",
    always_build=True,
)