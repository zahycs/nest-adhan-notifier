import subprocess
import os
revision = ""
try:
    revision = (
        subprocess.check_output(["git", "describe", "--tags", "--always"], stderr=subprocess.DEVNULL)
        .strip()
        .decode("utf-8")
    )
except:
    pass 

print("-DGIT_VERSION='\"%s\"'" % revision)
# Set the GIT_VERSION environment variable
# Set the GIT_VERSION environment variable
with open(os.environ['GITHUB_ENV'], 'a') as f:
    f.write(f"GIT_VERSION={revision}\n")

