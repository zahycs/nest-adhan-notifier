import subprocess
import os
import re

# Get the git revision
revision = ""
try:
    revision = (
        subprocess.check_output(["git", "describe", "--tags", "--always"], stderr=subprocess.DEVNULL)
        .strip()
        .decode("utf-8")
    )

    # Extract the semantic version from the git revision
    match = re.match(r"v?(\d+\.\d+\.\d+)", revision)
    if match:
        revision = match.group(1)
    else:
        revision = ""

except:
    pass

print("-DGIT_VERSION='\"%s\"'" % revision)

# Check if the script is running on GitHub Actions
if 'GITHUB_ENV' in os.environ:
    # Set the GIT_VERSION environment variable for GitHub Actions
    with open(os.environ['GITHUB_ENV'], 'a') as f:
        f.write(f"GIT_VERSION={revision}\n")
else:
    # Set the GIT_VERSION environment variable for local builds
    with open('.env', 'a') as f:
        f.write(f"GIT_VERSION={revision}\n")
