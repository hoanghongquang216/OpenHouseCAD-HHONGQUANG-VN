#!/usr/bin/env bash
#
# bootstrap.sh -- install the toolchain needed to build OpenHouseCAD.
#
# For a fresh Linux/WSL machine. Everything it installs is standard
# distro packaging; it does not fetch anything from unofficial sources.
# If you'd rather install by hand, run doctor.sh to see exactly what's
# missing and install just that.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/bootstrap.sh [options]

Installs the build toolchain: a C++23 compiler, CMake, Ninja, and Git.

Options:
  --dry-run   Print the install command instead of running it
  --yes       Don't ask for confirmation
  --help      Show this message

Supported package managers: apt (Debian/Ubuntu/WSL), dnf (Fedora),
pacman (Arch). On anything else, this prints the required packages and
exits so you can install them yourself.

Requires sudo for the actual installation. Run ./scripts/doctor.sh
afterwards to confirm the result.
EOF
}

detect_package_manager() {
    if command -v apt-get >/dev/null 2>&1; then
        echo apt
    elif command -v dnf >/dev/null 2>&1; then
        echo dnf
    elif command -v pacman >/dev/null 2>&1; then
        echo pacman
    else
        echo unknown
    fi
}

main() {
    local dry_run=0
    local assume_yes=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --dry-run) dry_run=1; shift ;;
            --yes|-y)  assume_yes=1; shift ;;
            --help|-h) usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    local pm
    pm="$(detect_package_manager)"

    local -a cmd=()
    case "${pm}" in
        apt)
            cmd=(sudo apt-get install -y build-essential g++-13 cmake ninja-build git)
            ;;
        dnf)
            cmd=(sudo dnf install -y gcc-c++ cmake ninja-build git)
            ;;
        pacman)
            cmd=(sudo pacman -S --needed base-devel cmake ninja git)
            ;;
        unknown)
            log_warn "No supported package manager found (looked for apt-get, dnf, pacman)."
            log_info "Install these yourself, then run ./scripts/doctor.sh:"
            printf '    - A C++23 compiler (GCC 13+ or Clang 17+)\n' >&2
            printf '    - CMake 3.25 or newer\n' >&2
            printf '    - Ninja (optional, but faster than make)\n' >&2
            printf '    - Git\n' >&2
            exit 1
            ;;
    esac

    log_step "Detected package manager: ${pm}"
    printf '\nWill run:\n    %s\n\n' "${cmd[*]}" >&2

    if (( dry_run )); then
        log_info "Dry run -- nothing was installed."
        exit 0
    fi

    if (( ! assume_yes )); then
        local reply
        read -r -p "Proceed? [y/N] " reply
        case "${reply}" in
            [yY]|[yY][eE][sS]) ;;
            *) log_info "Cancelled."; exit 0 ;;
        esac
    fi

    # apt needs its package lists refreshed on a fresh image, otherwise
    # installs fail with "Unable to locate package".
    if [[ "${pm}" == "apt" ]]; then
        log_step "Updating package lists"
        run sudo apt-get update
    fi

    log_step "Installing"
    if ! run "${cmd[@]}"; then
        die "Installation failed. Install the packages manually, then run ./scripts/doctor.sh"
    fi

    log_success "Toolchain installed"
    printf '\nNext:  ./scripts/doctor.sh\n' >&2
}

main "$@"
