#!/usr/bin/env bash
#
# clean.sh -- remove build artifacts.
#
# Only ever touches the build directory and generated output. Never
# removes anything tracked in source control.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/clean.sh [options]

Removes the build directory (and, with --all, other generated output).

Options:
  --all     Also remove release archives from dist/
  --dry-run Show what would be removed, remove nothing
  --help    Show this message

Environment:
  BUILD_DIR   Build directory (default: <repo>/build)

Never removes source files. Safe to run at any time; the build can
always be recreated with ./scripts/build.sh.
EOF
}

main() {
    local all=0
    local dry_run=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --all)     all=1; shift ;;
            --dry-run) dry_run=1; shift ;;
            --help|-h) usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    require_repo_root

    local -a targets=()
    [[ -d "${BUILD_DIR}" ]] && targets+=("${BUILD_DIR}")
    if (( all )) && [[ -d "${REPO_ROOT}/dist" ]]; then
        targets+=("${REPO_ROOT}/dist")
    fi

    if (( ${#targets[@]} == 0 )); then
        log_success "Nothing to clean"
        exit 0
    fi

    # Refuse to delete anything outside the repository. A bad BUILD_DIR
    # (say, BUILD_DIR=/ or an unset variable expanding oddly) should not
    # be able to turn this into a destructive command.
    local t
    for t in "${targets[@]}"; do
        local resolved
        resolved="$(cd -- "${t}" && pwd)"
        if [[ "${resolved}" != "${REPO_ROOT}/"* ]]; then
            die "Refusing to remove '${resolved}': outside the repository (${REPO_ROOT})"
        fi
    done

    if (( dry_run )); then
        log_info "Would remove:"
        for t in "${targets[@]}"; do
            printf '    %s\n' "${t}" >&2
        done
        exit 0
    fi

    log_step "Cleaning"
    for t in "${targets[@]}"; do
        run rm -rf "${t}"
    done

    log_success "Clean"
}

main "$@"
