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
with open(os.environ['GITHUB_OUTPUT'], 'a') as f:
    f.write(f"GIT_VERSION={revision}\n")

