"""Build the experimental legacy source without a Rust toolchain."""
import json
from pathlib import Path
import secrets
import subprocess
import zipfile

root = Path(__file__).resolve().parent
subprocess.run(
    [
        "clang", "--target=wasm32", "-Wall", "-Wextra", "-Werror",
        "-O2", "-nostdlib", "-fno-builtin",
        "-mno-bulk-memory", f"-DSESSION_SEED={secrets.randbits(32) or 1}u",
        "-Wl,--no-entry", "-Wl,--export-memory", "-Wl,-z,stack-size=131072",
        "-Wl,--initial-memory=23068672", "-Wl,--max-memory=33554432",
        str(root / "source.c"), "-o", str(root / "legacy.wasm"),
    ],
    check=True,
)
manifest = {
    "info": {
        "id": "pt.tomato",
        "name": "Tomato (experimental)",
        "version": 1,
        "lang": "pt-BR",
        "url": "https://tomatoanimes.com",
    }
}
(root / "source.json").write_text(json.dumps(manifest, indent=2) + "\n")
output = root / "pt.tomato-legacy-experimental.aix"
with zipfile.ZipFile(output, "w", zipfile.ZIP_DEFLATED) as archive:
    archive.writestr("Payload/source.json", json.dumps(manifest))
    archive.writestr("Payload/settings.json", json.dumps([{"type":"text","placeholder":"Sessao Tomato","key":"token","default":""}]))
    archive.write(root / "legacy.wasm", "Payload/main.wasm")
with zipfile.ZipFile(output) as archive:
    assert archive.testzip() is None
print(output)
