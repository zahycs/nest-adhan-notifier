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
os.environ["GIT_VERSION"] = revision
print("-DGIT_VERSION='\"%s\"'" % revision)
