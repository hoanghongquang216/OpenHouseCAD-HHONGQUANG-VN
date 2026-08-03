#!/usr/bin/env bash
#
# verify.sh -- the one command that checks everything.
#
# Runs the full gate: environment check, configure, build, test, demos.
# If this passes, the tree is in a state worth committing or releasing.
# Intended to be the last thing run before pushing, and what release.sh
# insists on before it will do anything.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/verify.sh [options]

Runs the complete verification pipeline:

  1. doctor.sh     -- environment is usable
  2. configure.sh  -- CMake configure (warnings-as-errors ON)
  3. build.sh      -- compile everything
  4. test.sh       -- full CTest suite
  5. demo.sh       -- every demo runs and produces output

Stops at the first failure.

Options:
  --build-type <type>  Debug | Release | RelWithDebInfo | MinSizeRel
                       (default: Debug)
  --fresh              Configure from scratch (discards the CMake cache)
  --skip-demos         Skip step 5
  --help               Show this message

Environment:
  BUILD_DIR   Build directory (default: <repo>/build)

Exit codes:
  0  Everything passed.
  1  Something failed -- see the output for which step.

Examples:
  ./scripts/verify.sh
  ./scripts/verify.sh --fresh --build-type Release
EOF
}

main() {
    local build_type="Debug"
    local fresh=0
    local skip_demos=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --build-type)
                [[ $# -ge 2 ]] || die "--build-type requires a value"
                build_type="$2"
                shift 2
                ;;
            --fresh)      fresh=1; shift ;;
            --skip-demos) skip_demos=1; shift ;;
            --help|-h)    usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    require_repo_root

    local started_at
    started_at="$(date +%s)"

    local total_steps=5
    (( skip_demos )) && total_steps=4
    local step=0

    announce() {
        step=$((step + 1))
        printf '\n%s[%d/%d] %s%s\n' \
            "${C_BOLD}" "${step}" "${total_steps}" "$*" "${C_RESET}" >&2
    }

    # --- 1. Environment ---------------------------------------------------
    announce "Checking environment"
    if ! "${SCRIPTS_DIR}/doctor.sh"; then
        die "Environment check failed -- fix the problems above, then re-run."
    fi

    # --- 2. Configure ------------------------------------------------------
    announce "Configuring"
    local -a configure_args=(--build-type "${build_type}")
    (( fresh )) && configure_args+=(--fresh)
    if ! "${SCRIPTS_DIR}/configure.sh" "${configure_args[@]}"; then
        die "Configure failed."
    fi

    # --- 3. Build ----------------------------------------------------------
    announce "Building"
    if ! "${SCRIPTS_DIR}/build.sh"; then
        die "Build failed."
    fi

    # --- 4. Test -----------------------------------------------------------
    announce "Testing"
    if ! "${SCRIPTS_DIR}/test.sh" --no-build; then
        die "Tests failed."
    fi

    # --- 5. Demos ----------------------------------------------------------
    if (( ! skip_demos )); then
        announce "Running demos"
        if ! "${SCRIPTS_DIR}/demo.sh" --no-build; then
            die "Demos failed."
        fi
    fi

    local elapsed=$(( $(date +%s) - started_at ))

    printf '\n%s%s%s\n' "${C_BOLD}" "========================================" "${C_RESET}" >&2
    log_success "Verification passed (${build_type}, ${elapsed}s)"
    printf '%s%s%s\n' "${C_BOLD}" "========================================" "${C_RESET}" >&2
    printf '\nThe tree is in a releasable state.\n' >&2
    printf 'Next:  ./scripts/release.sh --help\n' >&2
}

main "$@"
