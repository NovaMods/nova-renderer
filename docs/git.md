# Git Formalities

In order to keep some semblance of an organized git history, there are
a few rules that you must follow to keep things proper.

## Commit Messages

Commit messages must be in the imperative mood and follow all guidelines
put forward in [Chris Beams' Guide](https://chris.beams.io/posts/git-commit/).
This is good reading to get the intent of having proper git history.

Additionally, we "namespace" our commits. Each commit should only concern
one particular module. This module should be one of the following. If
a change could fall under multiple, it should be split up to keep commits
as small in scope as possible.

* `[all]` = affects many different modules/crates
* `[cmake]` = changes to the build
* `[ci]` = CI on all platforms.
  * `[ci-nix]` = CI on Mac and Linux.
    * `[ci-osx]` = CI on OSX.
    * `[ci-linux]` = CI on Linux.
  * `[ci-win]` = CI on Windows.
* `[docs]` = documentation changes.
* `[ide]` = changes to accommodate a particular ide
* `[git]` = git specific changes (.gitignore etc).
* `[*]` = (fill in * with the name) Nova module `nova-*`. (ex. `nova-shaderpack` would be `[shaderpack]`)
* `[tests]` = testing related changes.
* `[tools]` = changes to assorted tooling

## Merge Style

You should clean up your branch before merge and rebase away all issues
with the history. This history will be merged with a **merge commit**, so
having a sane history is important. Additionally you should **NEVER MERGE MASTER** as that
leads to [Foxtrot](https://blog.developer.atlassian.com/stop-foxtrots-now/) merges. If you need to
bring your branch up to date, always use rebasing. See below for some helpful tips.

## Git IDE

It is generally recommended to use a git IDE of some kind to make the operations easier
to deal with and to visualize. I (cwfitzgerald) am partial to Sublime Merge, but VSCode and others
would work just as well.

## Git Cheatsheet

Rebasing can cause diverging histories, especially if people are going in and meddling in each other's
branches. This is how to deal with common situations.

#### Upstream Updated Normally

``` 
git pull --ff origin <branch>
```

If you have conflicting changes in your working directory:

```
git stash
git pull --ff origin <branch>
git stash pop
```

This may cause some "merge" conflicts that need to be resolved, but will do so without
causing unnecessary merge commits. 

#### Upstream Force Pushed

This operation will nuke all changes your working directory. Always use `stash` unless
your working directory is unchanged.

```
git stash
git fetch origin
git reset --hard origin/<branch>
git stash pop
```

#### Master Changed

This is when you need to rebase on master to get changes in master.

```
git fetch origin
git rebase origin/master
```

You will get plenty of merge issues. Look at the issues,
resolve them in a way that makes sense _for your branch_, stage them, then issue one of the following
commands:

```
git rebase --continue # if index isn't empty
git rebase --skip     # if index is empty
```

To tell if the index is empty, just do:

```
git status
```

If there are green entries the index has changes.
