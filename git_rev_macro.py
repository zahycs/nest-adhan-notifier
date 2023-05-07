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
print(f"::set-env name=GIT_VERSION::{revision}")

