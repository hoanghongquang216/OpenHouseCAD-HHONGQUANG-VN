#!/usr/bin/env bash
# Shared helpers for OpenHouseCAD's scripts/.
#
# Sourced (not executed) by every script in scripts/. Provides logging,
# repository-root detection, and small guards. Keep this dependency-free
# beyond coreutils + bash 4+.

# Guard against double-sourcing (verify.sh sources other scripts that
# each source this).
if [[ -n "${OPENHOUSE_COMMON_SH_LOADED:-}" ]]; then
    return 0
fi
OPENHOUSE_COMMON_SH_LOADED=1

# --- Colors ----------------------------------------------------------------
# Disabled automatically when not writing to a terminal (so piping to a
# file or CI log doesn't get filled with escape codes), or when NO_COLOR
# is set (https://no-color.org/).
if [[ -t 1 && -z "${NO_COLOR:-}" ]]; then
    readonly C_RESET=$'\033[0m'
    readonly C_RED=$'\033[0;31m'
    readonly C_GREEN=$'\033[0;32m'
    readonly C_YELLOW=$'\033[0;33m'
    readonly C_BLUE=$'\033[0;34m'
    readonly C_BOLD=$'\033[1m'
else
    readonly C_RESET=""
    readonly C_RED=""
    readonly C_GREEN=""
    readonly C_YELLOW=""
    readonly C_BLUE=""
    readonly C_BOLD=""
fi

# --- Logging ---------------------------------------------------------------
# All logging goes to stderr, so a script's stdout stays clean for actual
# output that a caller might want to capture.

log_info()    { printf '%s==>%s %s\n'  "${C_BLUE}"   "${C_RESET}" "$*" >&2; }
log_success() { printf '%s OK %s %s\n' "${C_GREEN}"  "${C_RESET}" "$*" >&2; }
log_warn()    { printf '%swarn%s %s\n' "${C_YELLOW}" "${C_RESET}" "$*" >&2; }
log_error()   { printf '%sERROR%s %s\n' "${C_RED}"   "${C_RESET}" "$*" >&2; }

log_step() {
    printf '\n%s%s%s\n' "${C_BOLD}" "$*" "${C_RESET}" >&2
}

# Print an error and exit. Exit code defaults to 1; pass a different one
# as the first argument if a caller needs to distinguish failure modes.
die() {
    local code=1
    if [[ "$1" =~ ^[0-9]+$ ]]; then
        code="$1"
        shift
    fi
    log_error "$*"
    exit "${code}"
}

# --- Repository root -------------------------------------------------------
# Resolved from this file's own location (scripts/lib/common.sh -> ../..),
# NOT from the caller's working directory and NOT from `git rev-parse`.
# This means scripts work when invoked from anywhere, and also work in a
# plain (non-git) copy of the source tree -- e.g. an extracted release
# archive, where `git rev-parse --show-toplevel` would fail.
_common_sh_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
readonly REPO_ROOT="$(cd -- "${_common_sh_dir}/../.." && pwd)"
readonly SCRIPTS_DIR="${REPO_ROOT}/scripts"
unset _common_sh_dir

# Default build directory. Overridable via the environment so a caller can
# keep multiple build trees (e.g. BUILD_DIR=build-debug ./scripts/build.sh).
BUILD_DIR="${BUILD_DIR:-${REPO_ROOT}/build}"

# --- Guards ----------------------------------------------------------------

# Verify we're actually pointed at an OpenHouseCAD tree, not some
# unrelated directory -- catches the case where scripts/ was copied
# somewhere by mistake.
require_repo_root() {
    [[ -f "${REPO_ROOT}/CMakeLists.txt" ]] \
        || die "Not an OpenHouseCAD source tree (no CMakeLists.txt at ${REPO_ROOT})"
    [[ -d "${REPO_ROOT}/modules" ]] \
        || die "Not an OpenHouseCAD source tree (no modules/ at ${REPO_ROOT})"
}

require_command() {
    local cmd="$1"
    local hint="${2:-}"
    if ! command -v "${cmd}" >/dev/null 2>&1; then
        if [[ -n "${hint}" ]]; then
            die "Required command not found: ${cmd}  (${hint})"
        fi
        die "Required command not found: ${cmd}"
    fi
}

# Warn when the source tree lives on a Windows drive mounted into WSL.
# CMake's compiler checks fail there with "Operation not permitted" -- a
# real problem hit during this project's development, hence an explicit
# check rather than leaving the next person to rediscover it.
warn_if_windows_mount() {
    if [[ "${REPO_ROOT}" == /mnt/[a-z]/* ]]; then
        log_warn "Source tree is on a Windows mount (${REPO_ROOT})."
        log_warn "CMake may fail here with 'Operation not permitted'."
        log_warn "If it does, copy the tree into the Linux filesystem first:"
        log_warn "    cp -r \"${REPO_ROOT}\" \"\$HOME/\$(basename "${REPO_ROOT}")\""
        log_warn "    cd \"\$HOME/\$(basename "${REPO_ROOT}")\""
    fi
}

# --- Misc ------------------------------------------------------------------

# Number of parallel jobs. nproc isn't POSIX and is absent on some
# systems (notably macOS), so fall back rather than failing.
detect_jobs() {
    if command -v nproc >/dev/null 2>&1; then
        nproc
    elif command -v sysctl >/dev/null 2>&1 && sysctl -n hw.ncpu >/dev/null 2>&1; then
        sysctl -n hw.ncpu
    else
        echo 2
    fi
}

# Run a command, echoing it first so build logs show exactly what ran.
run() {
    printf '%s+%s %s\n' "${C_BLUE}" "${C_RESET}" "$*" >&2
    "$@"
}
