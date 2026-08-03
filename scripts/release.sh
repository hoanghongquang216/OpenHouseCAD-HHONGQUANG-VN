#!/usr/bin/env bash
#
# release.sh -- tag a release and produce a source archive.
#
# Deliberately conservative:
#   * Runs the full verification pipeline first and refuses to continue
#     if anything fails.
#   * Refuses to release from a dirty working tree.
#   * Does NOT push anything unless --push is given explicitly, because
#     pushing a tag is not something you can quietly undo once others
#     have fetched it.

set -euo pipefail

source "$(dirname -- "$(readlink -f -- "${BASH_SOURCE[0]}")")/lib/common.sh"

usage() {
    cat <<'EOF'
Usage: ./scripts/release.sh --tag <name> [options]

Verifies the tree, creates an annotated git tag, and writes a source
archive to dist/.

Required:
  --tag <name>       Tag to create, e.g. spiral-2-doc-003

Options:
  --message <text>   Tag message (default: "Release <tag>")
  --push             Push the tag to 'origin' after creating it.
                     Without this, nothing leaves your machine.
  --skip-verify      Skip the verification pipeline. Strongly discouraged;
                     intended only for re-packaging an already-verified
                     tree.
  --allow-dirty      Proceed even with uncommitted changes. The archive
                     will not match the tag's contents.
  --dry-run          Show what would happen; change nothing.
  --help             Show this message

Exit codes:
  0  Release created.
  1  Something failed or a precondition was not met.

Examples:
  ./scripts/release.sh --tag spiral-2-doc-003
  ./scripts/release.sh --tag spiral-2-doc-003 --message "Layer System" --push
  ./scripts/release.sh --tag v0.1.0-alpha --dry-run
EOF
}

main() {
    local tag=""
    local message=""
    local push=0
    local skip_verify=0
    local allow_dirty=0
    local dry_run=0

    while [[ $# -gt 0 ]]; do
        case "$1" in
            --tag)
                [[ $# -ge 2 ]] || die "--tag requires a value"
                tag="$2"
                shift 2
                ;;
            --message|-m)
                [[ $# -ge 2 ]] || die "--message requires a value"
                message="$2"
                shift 2
                ;;
            --push)        push=1; shift ;;
            --skip-verify) skip_verify=1; shift ;;
            --allow-dirty) allow_dirty=1; shift ;;
            --dry-run)     dry_run=1; shift ;;
            --help|-h)     usage; exit 0 ;;
            *)
                log_error "Unknown option: $1"
                usage >&2
                exit 1
                ;;
        esac
    done

    [[ -n "${tag}" ]] || { log_error "--tag is required"; usage >&2; exit 1; }
    [[ -n "${message}" ]] || message="Release ${tag}"

    require_repo_root
    require_command git

    git -C "${REPO_ROOT}" rev-parse --git-dir >/dev/null 2>&1 \
        || die "Not a git repository: ${REPO_ROOT}"

    # --- Preconditions -----------------------------------------------------
    log_step "Checking preconditions"

    if git -C "${REPO_ROOT}" rev-parse -q --verify "refs/tags/${tag}" >/dev/null; then
        die "Tag '${tag}' already exists. Choose another name, or delete it with:
    git tag -d ${tag}"
    fi
    log_success "Tag '${tag}' is available"

    if [[ -n "$(git -C "${REPO_ROOT}" status --porcelain)" ]]; then
        if (( allow_dirty )); then
            log_warn "Working tree has uncommitted changes (--allow-dirty given)"
        else
            log_error "Working tree has uncommitted changes:"
            git -C "${REPO_ROOT}" status --short >&2
            die "Commit or stash them first (or pass --allow-dirty)."
        fi
    else
        log_success "Working tree is clean"
    fi

    # --- Verify ------------------------------------------------------------
    if (( skip_verify )); then
        log_warn "Skipping verification (--skip-verify)"
    else
        log_step "Verifying before release"
        if ! "${SCRIPTS_DIR}/verify.sh"; then
            die "Verification failed -- not releasing."
        fi
    fi

    local dist_dir="${REPO_ROOT}/dist"
    local archive="${dist_dir}/openhousecad-${tag}.tar.gz"

    if (( dry_run )); then
        log_step "Dry run -- nothing was changed"
        printf '  Would create tag:  %s\n' "${tag}" >&2
        printf '  With message:      %s\n' "${message}" >&2
        printf '  Would write:       %s\n' "${archive}" >&2
        if (( push )); then
            printf '  Would push tag to: origin\n' >&2
        else
            printf '  Would NOT push (no --push)\n' >&2
        fi
        exit 0
    fi

    # --- Tag ---------------------------------------------------------------
    log_step "Creating tag"
    run git -C "${REPO_ROOT}" tag -a "${tag}" -m "${message}"
    log_success "Created annotated tag '${tag}'"

    # --- Archive -----------------------------------------------------------
    # git archive exports exactly what the tag contains -- no build
    # artifacts, no untracked files, nothing ignored.
    log_step "Building source archive"
    mkdir -p "${dist_dir}"
    if ! run git -C "${REPO_ROOT}" archive \
            --format=tar.gz \
            --prefix="openhousecad-${tag}/" \
            -o "${archive}" \
            "${tag}"; then
        # Roll the tag back so a failed run doesn't leave a half-done
        # release behind.
        log_error "Archive failed -- removing the tag just created"
        git -C "${REPO_ROOT}" tag -d "${tag}" >/dev/null 2>&1 || true
        die "Release aborted."
    fi
    log_success "Wrote $(du -h "${archive}" | cut -f1) archive: ${archive}"

    # --- Push --------------------------------------------------------------
    if (( push )); then
        log_step "Pushing tag to origin"
        if ! run git -C "${REPO_ROOT}" push origin "${tag}"; then
            log_error "Push failed. The tag exists locally; retry with:"
            log_error "    git push origin ${tag}"
            exit 1
        fi
        log_success "Pushed '${tag}' to origin"
    else
        log_info "Tag was NOT pushed (no --push given)."
        log_info "To publish it:  git push origin ${tag}"
    fi

    printf '\n' >&2
    log_success "Release ${tag} complete"
}

main "$@"
