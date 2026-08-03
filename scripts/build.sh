#!/usr/bin/env bash
#
# build.sh -- compile OpenHouseCAD.
#
# Configures first if the build directory doesn't exist yet, so a fresh
# checkout can go straight to ./scripts/build.sh without a separate
# configure step.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/build.sh [options]

Options:
  --jobs <n>       Parallel jobs (default: number of CPUs)
  --target <name>  Build only this target (e.g. OpenHouseLayerTests)
  --clean          Remove built artifacts first, then rebuild.
                   Keeps the CMake cache -- use configure.sh --fresh to
                   discard that too.
  --verbose        Show the full compiler command lines
  --help           Show this message

Environment:
  BUILD_DIR   Build directory (default: <repo>/build)

If the build directory does not exist, configure.sh runs automatically
with its defaults (Debug, warnings-as-errors ON).

Examples:
  ./scripts/build.sh
  ./scripts/build.sh --target OpenHouseDocumentTests
  ./scripts/build.sh --jobs 4 --verbose
EOF
}

main() {
    local jobs=""
    local target=""
    local clean=0
    local verbose=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --jobs|-j)
                [[ $# -ge 2 ]] || die "--jobs requires a value"
                [[ "$2" =~ ^[0-9]+$ ]] || die "--jobs expects a number, got '$2'"
                jobs="$2"
                shift 2
                ;;
            --target)
                [[ $# -ge 2 ]] || die "--target requires a value"
                target="$2"
                shift 2
                ;;
            --clean)   clean=1; shift ;;
            --verbose) verbose=1; shift ;;
            --help|-h) usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    require_repo_root
    require_command cmake "install it, e.g. 'sudo apt install cmake'"

    # Auto-configure on first use so build.sh works from a clean checkout.
    if [[ ! -d "${BUILD_DIR}" ]]; then
        log_info "No build directory yet -- configuring first"
        "${SCRIPTS_DIR}/configure.sh"
    fi

    [[ -n "${jobs}" ]] || jobs="$(detect_jobs)"

    local -a args=(--build "${BUILD_DIR}" --parallel "${jobs}")
    [[ -n "${target}" ]] && args+=(--target "${target}")
    (( clean )) && args+=(--clean-first)
    (( verbose )) && args+=(--verbose)

    if [[ -n "${target}" ]]; then
        log_step "Building target ${target} (${jobs} jobs)"
    else
        log_step "Building (${jobs} jobs)"
    fi

    if ! run cmake "${args[@]}"; then
        log_error "Build failed."
        log_error "If this is unexpected, try a clean configure:"
        log_error "    ./scripts/configure.sh --fresh && ./scripts/build.sh"
        exit 1
    fi

    log_success "Build complete"
    printf '\nNext:  ./scripts/test.sh\n' >&2
}

main "$@"
