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
        # Use a placeholder version if a valid semantic version is not available
        revision = "0.0.0"

except:
    # Use a placeholder version if an error occurs
    revision = "0.0.0"

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
