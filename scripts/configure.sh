#!/usr/bin/env bash
#
# configure.sh -- run CMake's configure step.
#
# Generates the build system into ${BUILD_DIR} (default: <repo>/build).
# Safe to re-run; CMake reuses its cache unless --fresh is passed.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/configure.sh [options]

Options:
  --build-type <type>   Debug | Release | RelWithDebInfo | MinSizeRel
                        (default: Debug)
  --no-werror           Do not treat compiler warnings as errors.
                        Warnings-as-errors is ON by default here, matching CI.
  --with-app            Also configure the Qt6 application layer
                        (requires Qt6; see docs/QT_INTEGRATION_CHECKLIST.md).
  --fresh               Delete the build directory first, then configure
                        from scratch. Use when CMake's cache is stale or
                        after switching compilers.
  --help                Show this message.

Environment:
  BUILD_DIR   Build directory (default: <repo>/build)
  CC / CXX    Compiler to use (default: whatever CMake picks)

Examples:
  ./scripts/configure.sh
  ./scripts/configure.sh --build-type Release
  CXX=g++-13 ./scripts/configure.sh --fresh
EOF
}

main() {
    local build_type="Debug"
    local werror="ON"
    local with_app="OFF"
    local fresh=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --build-type)
                [[ $# -ge 2 ]] || die "--build-type requires a value"
                build_type="$2"
                shift 2
                ;;
            --no-werror) werror="OFF"; shift ;;
            --with-app)  with_app="ON"; shift ;;
            --fresh)     fresh=1; shift ;;
            --help|-h)   usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    case "${build_type}" in
        Debug|Release|RelWithDebInfo|MinSizeRel) ;;
        *) die "Invalid --build-type '${build_type}' (expected Debug, Release, RelWithDebInfo, or MinSizeRel)" ;;
    esac

    require_repo_root
    require_command cmake "install it, e.g. 'sudo apt install cmake'"
    warn_if_windows_mount

    if (( fresh )) && [[ -d "${BUILD_DIR}" ]]; then
        log_info "Removing existing build directory (--fresh)"
        run rm -rf "${BUILD_DIR}"
    fi

    log_step "Configuring (${build_type}, warnings-as-errors=${werror})"

    local -a args=(
        -S "${REPO_ROOT}"
        -B "${BUILD_DIR}"
        "-DCMAKE_BUILD_TYPE=${build_type}"
        "-DOPENHOUSE_WARNINGS_AS_ERRORS=${werror}"
        "-DOPENHOUSE_BUILD_APP=${with_app}"
    )

    # Prefer Ninja when available: faster, and gives a single-config
    # generator so --config isn't needed later.
    if command -v ninja >/dev/null 2>&1; then
        args+=(-G Ninja)
    fi

    if ! run cmake "${args[@]}"; then
        log_error "CMake configure failed."
        if [[ "${REPO_ROOT}" == /mnt/[a-z]/* ]]; then
            log_error "The source tree is on a Windows mount -- that is the most"
            log_error "likely cause. Copy it into the Linux filesystem and retry."
        else
            log_error "Run ./scripts/doctor.sh to check the environment."
        fi
        exit 1
    fi

    log_success "Configured into ${BUILD_DIR}"
    printf '\nNext:  ./scripts/build.sh\n' >&2
}

main "$@"
