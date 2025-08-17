import subprocess
from datetime import datetime
from SCons.Script import Import

Import("env")

def get_git_version():
    try:
        # Check if the current commit is exactly on a tag
        version = subprocess.check_output(
            ["git", "describe", "--tags", "--exact-match"],
            stderr=subprocess.DEVNULL
        ).strip().decode("utf-8")
    except subprocess.CalledProcessError:
        try:
            # If no exact match, get the latest tag with commit info
            version = subprocess.check_output(
                ["git", "describe", "--tags", "--always"],
                stderr=subprocess.DEVNULL
            ).strip().decode("utf-8")
            # Append "-nightly" and date to indicate it's not an exact tag
            version += f"-nightly-{datetime.now().strftime('%Y-%m-%d')}"
        except subprocess.CalledProcessError:
            try:
                # If no tags exist, get the short commit hash
                version = subprocess.check_output(
                    ["git", "rev-parse", "--short", "HEAD"],
                    stderr=subprocess.DEVNULL
                ).strip().decode("utf-8")
                version += f"-nightly-{datetime.now().strftime('%Y-%m-%d')}"
            except subprocess.CalledProcessError:
                version = f"unknown-{datetime.now().strftime('%Y-%m-%d')}"  # fallback if git fails entirely

    return version

build_info = get_git_version()
# Append the version to build flags
env.Append(CPPDEFINES=[("BUILD_INFO", f'"{build_info}"')])
env.Append(CPPDEFINES=[("PIOENV", f'"{env["PIOENV"]}"')])

# Output the values to the console
print(f"BUILD_INFO: {build_info}")
print(f"PIOENV: {env['PIOENV']}")
