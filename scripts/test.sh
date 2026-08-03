#!/usr/bin/env bash
#
# test.sh -- run the test suite via CTest.
#
# Builds first if needed, so this can be run directly after a checkout.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/test.sh [options]

Options:
  --filter <regex>  Run only tests whose name matches (CTest -R)
  --jobs <n>        Run tests in parallel (default: number of CPUs)
  --no-build        Do not build first; fail if the build is missing
  --quiet           Only show failures and the final summary
  --help            Show this message

Environment:
  BUILD_DIR   Build directory (default: <repo>/build)

Examples:
  ./scripts/test.sh
  ./scripts/test.sh --filter Layer
  ./scripts/test.sh --filter 'Geometry|Bounds'
EOF
}

main() {
    local filter=""
    local jobs=""
    local do_build=1
    local quiet=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --filter|-R)
                [[ $# -ge 2 ]] || die "--filter requires a value"
                filter="$2"
                shift 2
                ;;
            --jobs|-j)
                [[ $# -ge 2 ]] || die "--jobs requires a value"
                [[ "$2" =~ ^[0-9]+$ ]] || die "--jobs expects a number, got '$2'"
                jobs="$2"
                shift 2
                ;;
            --no-build) do_build=0; shift ;;
            --quiet)    quiet=1; shift ;;
            --help|-h)  usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    require_repo_root
    require_command ctest "usually ships with cmake"

    if (( do_build )); then
        "${SCRIPTS_DIR}/build.sh"
    elif [[ ! -d "${BUILD_DIR}" ]]; then
        die "No build directory at ${BUILD_DIR} (and --no-build was given)"
    fi

    [[ -n "${jobs}" ]] || jobs="$(detect_jobs)"

    # --output-on-failure is deliberately always on: a test that fails
    # without showing why is close to useless.
    local -a args=(--test-dir "${BUILD_DIR}" --output-on-failure --parallel "${jobs}")
    [[ -n "${filter}" ]] && args+=(-R "${filter}")
    (( quiet )) && args+=(--quiet)

    if [[ -n "${filter}" ]]; then
        log_step "Running tests matching '${filter}'"
    else
        log_step "Running tests"
    fi

    if ! run ctest "${args[@]}"; then
        log_error "Tests failed."
        exit 1
    fi

    log_success "All tests passed"
}

main "$@"
