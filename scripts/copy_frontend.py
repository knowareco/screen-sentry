Import("env")
import subprocess
import shutil
import os

def build_and_copy_frontend(source, target, env):
    # Ensure frontend directory exists
    if not os.path.exists("frontend"):
        print("Error: frontend directory not found")
        return

    try:
        # Run npm build
        print("Installing frontend dependencies...")
        subprocess.run(["npm", "install"], cwd="frontend", check=True)
        
        print("Building frontend...")
        subprocess.run(["npm", "run", "build"], cwd="frontend", check=True)

        # Copy build files to data folder
        dist_folder = os.path.join("frontend", "dist")  # Vite uses 'dist' not 'build'
        data_folder = "data"

        print(f"Copying frontend files to {data_folder}...")
        if os.path.exists(data_folder):
            shutil.rmtree(data_folder)
        shutil.copytree(dist_folder, data_folder)
        
        print("Frontend build and copy completed successfully")

    except subprocess.CalledProcessError as e:
        print(f"Error during frontend build: {e}")
        env.Exit(1)
    except Exception as e:
        print(f"Error: {e}")
        env.Exit(1)

env.AddPreAction("upload", build_and_copy_frontend)