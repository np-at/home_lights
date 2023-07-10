import json
import shutil
import subprocess
import warnings
from time import sleep

Import("env")


def format_json_prop(inp):
    if type(inp) is str:
        return f'"{inp}"'
    elif type(inp) is int:
        return inp
    else:
        raise Exception(f"unknown input type for {inp}")


pioenv = env["PIOENV"]
print(f"pio env is {pioenv}")


# secrets_file = os.environ.get("PIO_SECRETS_FILE_PATH")
secrets_file = "secrets.json"
secrets: dict
if secrets_file:
    try:
        with open(secrets_file, "r", encoding="utf-8") as f:
            secrets = json.load(f).get(pioenv)
        print("secrets loaded from file")
    except Exception as e:
        print("error while attempting to get secrets from file")
        print(e)
        raise
else:
    op_executable = shutil.which("op")
    if op_executable is not None:
        try:
            result = subprocess.run(
                args=[op_executable, "read", "op://private/pio_secrets/contents"],
                capture_output=True,
                text=True,
            )
            result.check_returncode()
            secrets = json.loads(result.stdout.replace("\n", "")).get(pioenv)
        except Exception as ex:
            print(ex)
            sleep(5)
            warnings.warn(
                f"error while attempting to get secrets from 1Password CLI: {ex}"
            )

            warnings.warn(
                "PIO_SECRETS_FILE_PATH environmental var not set and unable to get secret from 1password cli, falling back to secrets.json"
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
print(f"secrets: {secrets}")


print("creating build flags")
build_flags = list(
    map(lambda x: f"-D{x[0]}={format_json_prop(x[1])}", secrets["BUILD_FLAGS"].items())
)

env["BUILD_FLAGS"].extend(build_flags)
if secrets.get("UPLOAD_FLAGS") is not None:
    upload_flags = list(
        map(
            lambda x: f"--{x[0]}={format_json_prop(x[1])}",
            secrets["UPLOAD_FLAGS"].items(),
        )
    )
else:
    upload_flags = list()

if env.get("UPLOAD_FLAGS") is not None:
    env["UPLOAD_FLAGS"].extend(upload_flags)
elif upload_flags is not None:
    env["UPLOAD_FLAGS"] = upload_flags

if secrets.get("UPLOAD_PORT") is not None:
    if upload_port := secrets["UPLOAD_PORT"]:
        env["UPLOAD_PORT"] = upload_port
    if upload_protocol := secrets.get("UPLOAD_PROTOCOL"):
        env["UPLOAD_PROTOCOL"] = upload_protocol

# # add macro definition for code to use in projenv (visible to src/ files)
# projenv.Append(CPPDEFINES=[
#     ("SECRET_VALUE_MACRO", secret)
# ])
