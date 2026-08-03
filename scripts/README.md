# scripts/

Build, test, and release automation for OpenHouseCAD.

Every script takes `--help`, exits non-zero on failure, works when
invoked from any directory, and hard-codes no paths. Linux and WSL are
the supported environments.

## The short version

```bash
./scripts/verify.sh          # everything: configure, build, test, demos
./scripts/release.sh --tag <name>   # verify, tag, package
```

On a machine that has never built this project:

```bash
./scripts/bootstrap.sh       # install the toolchain
./scripts/doctor.sh          # confirm it's usable
./scripts/verify.sh
```

## Scripts

| Script | What it does |
|---|---|
| `bootstrap.sh` | Installs the toolchain (compiler, CMake, Ninja, Git) via apt/dnf/pacman |
| `doctor.sh` | Checks the environment and reports anything missing or risky. Changes nothing |
| `configure.sh` | Runs CMake's configure step |
| `build.sh` | Compiles. Configures first if needed |
| `test.sh` | Runs the CTest suite. Builds first if needed |
| `demo.sh` | Runs every demo and collects the `.svg` output |
| `verify.sh` | doctor → configure → build → test → demos. Stops at the first failure |
| `release.sh` | Verifies, tags, and writes a source archive to `dist/` |
| `clean.sh` | Removes build artifacts |

Most scripts chain: `test.sh` builds first, `build.sh` configures first.
So `./scripts/test.sh` on a clean checkout does the right thing without
a sequence of setup commands.

## Common tasks

```bash
# Work on one test while iterating
./scripts/test.sh --filter Layer

# Build a single target
./scripts/build.sh --target OpenHouseDocumentTests

# Release build, from a clean CMake cache
./scripts/verify.sh --fresh --build-type Release

# See what a release would do, without doing it
./scripts/release.sh --tag v0.1.0 --dry-run

# Look at the demo output
./scripts/demo.sh --output-dir /tmp/svg
```

## Environment variables

| Variable | Effect |
|---|---|
| `BUILD_DIR` | Build directory (default: `<repo>/build`) |
| `CC` / `CXX` | Compiler to use |
| `NO_COLOR` | Disable colored output (also auto-disabled when not a TTY) |

```bash
CXX=g++-13 ./scripts/verify.sh
BUILD_DIR=/tmp/ohc-build ./scripts/build.sh
```

## WSL: build outside `/mnt/c`

CMake's compiler check fails on Windows-mounted paths under WSL with
`Operation not permitted`. If the source tree lives on `/mnt/c/...`,
copy it into the Linux filesystem first:

```bash
cp -r /mnt/c/Users/<you>/path/to/OpenHouseCAD-HHONGQUANG-VN "$HOME/"
cd "$HOME/OpenHouseCAD-HHONGQUANG-VN"
./scripts/verify.sh
```

`doctor.sh` warns when it detects this, and `configure.sh` points at it
if the configure step fails there.

## Release safety

`release.sh` is deliberately hard to misuse:

- It runs the full verification pipeline first and stops if anything
  fails (override with `--skip-verify`, which you generally shouldn't).
- It refuses to release from a dirty working tree (`--allow-dirty` to
  override).
- It refuses to overwrite an existing tag.
- **It does not push.** The tag stays local unless you pass `--push`.
- If archiving fails after the tag was created, it deletes the tag again
  rather than leaving a half-finished release behind.

The archive is produced with `git archive`, so it contains exactly what
the tag contains: no build output, no untracked files, nothing ignored.

## Exit codes

`0` on success, `1` on failure, for every script. Safe to chain with
`&&` or use in CI.

## Adding a script

Source the shared helpers and follow the existing shape:

```bash
#!/usr/bin/env bash
set -euo pipefail
source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"
```

`lib/common.sh` provides `log_info` / `log_success` / `log_warn` /
`log_error` / `log_step`, `die`, `run` (echoes a command before running
it), `require_repo_root`, `require_command`, `detect_jobs`, and the
`REPO_ROOT` / `SCRIPTS_DIR` / `BUILD_DIR` variables.
