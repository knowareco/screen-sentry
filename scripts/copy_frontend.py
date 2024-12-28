Import("env")
import subprocess
import shutil
import os

def build_and_copy_frontend(source, target, env):
    # Run npm build
    subprocess.run(["npm", "install"], cwd="frontend", check=True)
    subprocess.run(["npm", "run", "build"], cwd="frontend", check=True)

    # Copy build files to data folder
    dist_folder = os.path.join("frontend", "build")
    data_folder = "data"

    if os.path.exists(data_folder):
        shutil.rmtree(data_folder)

    shutil.copytree(dist_folder, data_folder)

env.AddPreAction("upload", build_and_copy_frontend)
