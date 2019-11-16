#!/usr/bin/env bash

set -e

if [[ -f ".git/hooks/.changed" ]]; then
    git_rev=$(git rev-parse HEAD)
    rm .git/hooks/.changed
    echo "${git_rev}" > .git/hooks/hooks-revision
    echo "${git_rev}" > tools/git-hooks/hooks-revision
    git add tools/git-hooks/hooks-revision > /dev/null
    git commit --amend --no-edit --no-verify > /dev/null
    echo -e "\e[32;1mUpdated git hooks revision to \e[33;1m${git_rev}\e[0m"
fi
