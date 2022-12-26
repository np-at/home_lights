import json
import shutil
import subprocess
import warnings

Import("env")

import os

pioenv = env["PIOENV"]
print(f"pio env is {pioenv}")

secrets_file = os.environ.get("PIO_SECRETS_FILE_PATH")
secrets: dict
if not secrets_file:
    op_executable = shutil.which("op")
    if op_executable is not None:
        try:
            result = subprocess.run(
                args=[op_executable, "read", "op://private/pio_secrets/contents"],
                capture_output=True,
                text=True,
            )
            result.check_returncode()
            secrets = json.loads(result.stdout).get(pioenv)
        except Exception as ex:
            warnings.warn(
                f"error while attempting to get secrets from 1Password CLI: {ex}"
            )

        warnings.warn(
            f"PIO_SECRETS_FILE_PATH environmental var not set and unable to get secret from 1password cli, falling back to secrets.json"
        )
        secrets_file = "secrets.json"
    try:
        if not secrets_file:
            pass
        else:
            with open(secrets_file, "r", encoding="utf-8") as f:
                secrets = json.load(f).get(pioenv)
    except Exception as e:
        print(e)
        raise

if not secrets:
    raise Exception("secrets not found")


def format_json_prop(inp: str | int):
    if type(inp) is str:
        return f'"{inp}"'
    elif type(inp) is int:
        return inp
    else:
        print(f"type of {inp} is {type(inp)}")
        raise Exception(f"unknown input type for {inp}")


build_flags = list(
    map(lambda x: f"-D{x[0]}={format_json_prop(x[1])}", secrets["BUILD_FLAGS"].items())
)
print(f"build flags {build_flags}")
env["BUILD_FLAGS"].extend(build_flags)

upload_flags = list(
    map(lambda x: f"--{x[0]}={format_json_prop(x[1])}", secrets["UPLOAD_FLAGS"].items())
)

if env.get("UPLOAD_FLAGS") is not None:
    env["UPLOAD_FLAGS"].extend(upload_flags)
else:
    env["UPLOAD_FLAGS"] = upload_flags
if upload_port := secrets["UPLOAD_PORT"]:
    env["UPLOAD_PORT"] = upload_port
if upload_protocol := secrets.get("UPLOAD_PROTOCOL"):
    env["UPLOAD_PROTOCOL"] = upload_protocol

# # add macro definition for code to use in projenv (visible to src/ files)
# projenv.Append(CPPDEFINES=[
#     ("SECRET_VALUE_MACRO", secret)
# ])
