#!/usr/bin/env bash
#
# doctor.sh -- check that this machine can build OpenHouseCAD.
#
# Reports what's present, what's missing, and what's likely to cause
# trouble, then exits non-zero if anything would actually block a build.
# Safe to run before anything else -- it changes nothing.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/doctor.sh [--help]

Checks the local environment for everything needed to build and test
OpenHouseCAD, and reports anything that would get in the way.

Exit codes:
  0  Ready to build.
  1  Something required is missing or misconfigured.

Changes nothing on disk; safe to run at any time.
EOF
}

# --- Result tracking -------------------------------------------------------
# Warnings don't fail the run; only genuine blockers do.
problems=0
warnings=0

check_pass() { printf '  %s[ ok ]%s %s\n' "${C_GREEN}" "${C_RESET}" "$*" >&2; }
check_warn() { printf '  %s[warn]%s %s\n' "${C_YELLOW}" "${C_RESET}" "$*" >&2; warnings=$((warnings + 1)); }
check_fail() { printf '  %s[FAIL]%s %s\n' "${C_RED}" "${C_RESET}" "$*" >&2; problems=$((problems + 1)); }

# --- Individual checks -----------------------------------------------------

check_source_tree() {
    log_step "Source tree"

    if [[ -f "${REPO_ROOT}/CMakeLists.txt" && -d "${REPO_ROOT}/modules" ]]; then
        check_pass "OpenHouseCAD source tree at ${REPO_ROOT}"
    else
        check_fail "Not an OpenHouseCAD source tree: ${REPO_ROOT}"
        return
    fi

    # The Windows-mount trap: CMake's compiler probe fails on /mnt/<drive>
    # under WSL with "Operation not permitted". This is a hard blocker in
    # practice, but reported as a warning because it depends on the WSL
    # version and mount options -- some setups do work.
    if [[ "${REPO_ROOT}" == /mnt/[a-z]/* ]]; then
        check_warn "Source tree is on a Windows mount (${REPO_ROOT})"
        printf '         %sCMake often fails here with "Operation not permitted".%s\n' \
            "${C_YELLOW}" "${C_RESET}" >&2
        printf '         Copy into the Linux filesystem and build there:\n' >&2
        printf '             cp -r "%s" "$HOME/%s"\n' "${REPO_ROOT}" "$(basename "${REPO_ROOT}")" >&2
        printf '             cd "$HOME/%s"\n' "$(basename "${REPO_ROOT}")" >&2
    else
        check_pass "Source tree is on a native filesystem"
    fi
}

check_compiler() {
    log_step "C++ compiler"

    local cxx="${CXX:-}"
    if [[ -n "${cxx}" ]]; then
        if command -v "${cxx}" >/dev/null 2>&1; then
            check_pass "CXX=${cxx} ($(${cxx} --version | head -1))"
        else
            check_fail "CXX is set to '${cxx}' but that command was not found"
        fi
        return
    fi

    # No CXX set -- look for something suitable. C++23 support is the
    # real requirement; GCC 13+ / Clang 17+ are the practical baselines.
    local found=""
    local candidate
    for candidate in g++-14 g++-13 g++ clang++-18 clang++-17 clang++; do
        if command -v "${candidate}" >/dev/null 2>&1; then
            found="${candidate}"
            break
        fi
    done

    if [[ -z "${found}" ]]; then
        check_fail "No C++ compiler found (looked for g++/clang++)"
        printf '         Install one, e.g.:  sudo apt install build-essential g++-13\n' >&2
        return
    fi

    check_pass "Found ${found} ($(${found} --version | head -1))"

    # Actually compile a C++23 snippet rather than parsing version
    # numbers -- version strings vary and a "new enough" compiler can
    # still be missing library pieces this project uses.
    local probe
    probe="$(mktemp -t openhouse-probe-XXXXXX.cpp)"
    cat >"${probe}" <<'PROBE'
#include <expected>
#include <format>
#include <print>
#include <utility>
int main() {
    std::expected<int, int> e{1};
    auto s = std::format("{}", *e);
    return s.empty() ? 1 : 0;
}
PROBE
    if "${found}" -std=c++23 -fsyntax-only "${probe}" 2>/dev/null; then
        check_pass "Compiles C++23 (std::expected, std::format, std::print)"
    else
        # Retry without <print>, which arrived later than the others.
        sed -i '/#include <print>/d' "${probe}"
        if "${found}" -std=c++23 -fsyntax-only "${probe}" 2>/dev/null; then
            check_pass "Compiles C++23 (std::expected, std::format)"
            check_warn "<print> unavailable -- fine for now; nothing in-tree uses it"
        else
            check_fail "${found} cannot compile the C++23 features this project needs"
            printf '         Need GCC 13+ or Clang 17+ with a matching standard library.\n' >&2
        fi
    fi
    rm -f "${probe}"
}

check_cmake() {
    log_step "CMake"

    if ! command -v cmake >/dev/null 2>&1; then
        check_fail "cmake not found"
        printf '         Install it, e.g.:  sudo apt install cmake\n' >&2
        return
    fi

    local version
    version="$(cmake --version | head -1 | awk '{print $3}')"
    check_pass "cmake ${version}"

    # The project's CMakeLists.txt files require 3.25.
    local major minor
    major="${version%%.*}"
    minor="${version#*.}"
    minor="${minor%%.*}"
    if (( major > 3 )) || { (( major == 3 )) && (( minor >= 25 )); }; then
        check_pass "Meets the required minimum (3.25)"
    else
        check_fail "CMake ${version} is older than the required 3.25"
    fi
}

check_ctest() {
    log_step "CTest"
    if command -v ctest >/dev/null 2>&1; then
        check_pass "ctest $(ctest --version | head -1 | awk '{print $3}')"
    else
        check_fail "ctest not found (usually ships with cmake)"
    fi
}

check_build_tool() {
    log_step "Build tool"
    if command -v ninja >/dev/null 2>&1; then
        check_pass "ninja $(ninja --version) (preferred)"
    elif command -v make >/dev/null 2>&1; then
        check_pass "make $(make --version | head -1 | awk '{print $3}')"
        check_warn "ninja not installed -- builds work, just slower"
    else
        check_fail "Neither ninja nor make found"
        printf '         Install one, e.g.:  sudo apt install ninja-build\n' >&2
    fi
}

check_git() {
    log_step "Git"
    if ! command -v git >/dev/null 2>&1; then
        check_warn "git not found -- build and test work; release.sh will not"
        return
    fi
    check_pass "git $(git --version | awk '{print $3}')"

    if git -C "${REPO_ROOT}" rev-parse --git-dir >/dev/null 2>&1; then
        check_pass "Source tree is a git repository"
    else
        check_warn "Source tree is not a git repository -- release.sh will not work"
    fi
}

check_disk_space() {
    log_step "Disk space"
    if ! command -v df >/dev/null 2>&1; then
        check_warn "df not available -- skipping disk space check"
        return
    fi
    local avail_kb
    avail_kb="$(df -Pk "${REPO_ROOT}" | awk 'NR==2 {print $4}')"
    local avail_mb=$(( avail_kb / 1024 ))
    if (( avail_mb < 200 )); then
        check_warn "Only ${avail_mb} MB free on the volume holding the source tree"
    else
        check_pass "${avail_mb} MB free"
    fi
}

main() {
    if [[ "${1:-}" == "--help" || "${1:-}" == "-h" ]]; then
        usage
        exit 0
    fi
    if [[ $# -gt 0 ]]; then
        log_error "Unknown argument: $1"
        usage >&2
        exit 1
    fi

    printf '%sOpenHouseCAD environment check%s\n' "${C_BOLD}" "${C_RESET}" >&2

    check_source_tree
    check_compiler
    check_cmake
    check_ctest
    check_build_tool
    check_git
    check_disk_space

    log_step "Summary"
    if (( problems > 0 )); then
        log_error "${problems} problem(s) found -- fix these before building."
        (( warnings > 0 )) && log_warn "${warnings} warning(s) as well."
        exit 1
    fi

    if (( warnings > 0 )); then
        log_success "Ready to build (with ${warnings} warning(s) -- see above)."
    else
        log_success "Ready to build."
    fi
    printf '\nNext:  ./scripts/verify.sh\n' >&2
}

main "$@"
