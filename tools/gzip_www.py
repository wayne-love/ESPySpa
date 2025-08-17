# tools/gzip_assets.py
Import("env")
import gzip, shutil, os

def gzip_assets(source, target, env):
    data_dir = os.path.join(env["PROJECT_DIR"], "data", "www", "assets")
    if not os.path.isdir(data_dir):
        print("No /data/www/assets folder found, skipping gzip")
        return

    print("Gzipping files in:", data_dir)
    for root, _, files in os.walk(data_dir):
        for f in files:
            if f.endswith(".gz"):
                continue  # skip already gzipped files

            filepath = os.path.join(root, f)
            gz_path = filepath + ".gz"

            if f.endswith((".html", ".css", ".js", ".ico", ".svg", ".json")):
                with open(filepath, "rb") as fin, gzip.open(gz_path, "wb") as fout:
                    shutil.copyfileobj(fin, fout)

                # Remove original file
                os.remove(filepath)
                print(f" → {f} → {f}.gz (original deleted)")

# Hook to the buildfs target
env.AddPreAction("buildfs", gzip_assets)
env.AddPreAction("uploadfs", gzip_assets)
