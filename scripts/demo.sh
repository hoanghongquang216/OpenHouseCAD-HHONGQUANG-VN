#!/usr/bin/env bash
#
# demo.sh -- run the example programs and collect their SVG output.
#
# The demos are the project's "does it actually work" evidence (see the
# Spiral development model in docs/ROADMAP_EXECUTION.md): each one
# produces a real .svg file you can open in a browser.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/demo.sh [options]

Runs every demo executable and writes its .svg output into a single
directory.

Options:
  --output-dir <dir>  Where to write the .svg files
                      (default: <build>/demo-output)
  --only <name>       Run just one demo (substring match, e.g. 'layers')
  --list              List the demos that would run, then exit
  --no-build          Do not build first; fail if executables are missing
  --help              Show this message

Environment:
  BUILD_DIR   Build directory (default: <repo>/build)

Examples:
  ./scripts/demo.sh
  ./scripts/demo.sh --only layers
  ./scripts/demo.sh --output-dir /tmp/svg
EOF
}

# Demo executables are discovered by scanning the build tree rather than
# hard-coded here, so adding a new example to CMakeLists.txt doesn't also
# require editing this script.
find_demos() {
    find "${BUILD_DIR}" -type f -perm -u+x -name 'OpenHouse*Demo' 2>/dev/null | sort
}

main() {
    local output_dir=""
    local only=""
    local list_only=0
    local do_build=1

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --output-dir)
                [[ $# -ge 2 ]] || die "--output-dir requires a value"
                output_dir="$2"
                shift 2
                ;;
            --only)
                [[ $# -ge 2 ]] || die "--only requires a value"
                only="$2"
                shift 2
                ;;
            --list)     list_only=1; shift ;;
            --no-build) do_build=0; shift ;;
            --help|-h)  usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    require_repo_root

    if (( do_build )) && (( ! list_only )); then
        "${SCRIPTS_DIR}/build.sh"
    fi

    [[ -d "${BUILD_DIR}" ]] || die "No build directory at ${BUILD_DIR} -- run ./scripts/build.sh"

    local -a demos=()
    while IFS= read -r line; do
        [[ -n "${line}" ]] && demos+=("${line}")
    done < <(find_demos)

    if (( ${#demos[@]} == 0 )); then
        die "No demo executables found under ${BUILD_DIR} (has the project been built?)"
    fi

    # Apply --only filter.
    if [[ -n "${only}" ]]; then
        local -a filtered=()
        local d
        for d in "${demos[@]}"; do
            if [[ "$(basename "${d}")" == *"${only}"* ]]; then
                filtered+=("${d}")
            fi
        done
        if (( ${#filtered[@]} == 0 )); then
            log_error "No demo matched '${only}'. Available:"
            for d in "${demos[@]}"; do
                printf '    %s\n' "$(basename "${d}")" >&2
            done
            exit 1
        fi
        demos=("${filtered[@]}")
    fi

    if (( list_only )); then
        local d
        for d in "${demos[@]}"; do
            printf '%s\n' "$(basename "${d}")"
        done
        exit 0
    fi

    [[ -n "${output_dir}" ]] || output_dir="${BUILD_DIR}/demo-output"
    mkdir -p "${output_dir}"

    log_step "Running ${#demos[@]} demo(s)"

    local failures=0
    local d name svg
    for d in "${demos[@]}"; do
        name="$(basename "${d}")"
        svg="${output_dir}/${name}.svg"
        if "${d}" "${svg}" >/dev/null 2>&1; then
            if [[ -s "${svg}" ]]; then
                printf '  %s[ ok ]%s %-32s -> %s\n' \
                    "${C_GREEN}" "${C_RESET}" "${name}" "${svg}" >&2
            else
                # Exited 0 but wrote nothing useful -- worth flagging,
                # since a silently empty SVG looks like success otherwise.
                printf '  %s[FAIL]%s %-32s (exited 0 but produced no output)\n' \
                    "${C_RED}" "${C_RESET}" "${name}" >&2
                failures=$((failures + 1))
            fi
        else
            printf '  %s[FAIL]%s %-32s (non-zero exit)\n' \
                "${C_RED}" "${C_RESET}" "${name}" >&2
            failures=$((failures + 1))
        fi
    done

    if (( failures > 0 )); then
        log_error "${failures} demo(s) failed"
        exit 1
    fi

    log_success "All demos ran; output in ${output_dir}"
    printf '\nOpen any of the .svg files in a browser to view them.\n' >&2
}

main "$@"
