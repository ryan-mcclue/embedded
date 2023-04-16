// SPDX-License-Identifier: zlib-acknowledgement

// bug sourcing: https://interrupt.memfault.com/blog/git-bisect

// version (tags, branches)
// pre-release --> alpha, beta, rc
// MAJOR (incompatible with previous)
// MINOR (average)
// PATCH (hotfix)
// TODO(Ryan): Company procedure in making a pull request
// git ethos 'one idea is one commit':
// In Git, this means squashing checkpoint commits as you go (with git commit --amend) or before pushing (with git rebase -i or git merge --squash), or having a strict policy where your master/trunk contains only merge commits and each is a merge between the old master and a branch which represents a single idea. Although this preserves the checkpoint commits along the branches, you can view master alone as a series of single-idea commits.

// reproducible (so can go back for older binaries and regenerate symbols)
// (should compile from different directories)

// https://www.heroku.com/postgres
// build metrics database (free hosted db --> postgres://<username>:<password>@<server>:<port>/<database_name>) 
// (redash to visualise?)

// CI + testing (using mocks is implicitly white-box testing)
// want a database for static analysis info? 
// code coverage necessary for certification
// 100% code coverage desirable (code units), however 100% test coverage (i.e. all branches) perhaps too much time
// Prioritise features, and risky code (battery, firmware update, factory reset)

// Note: once your build system stabilizes, you’ll likely want to set up your own docker images with your compiler pre-installed so you do not have to incur the cost of download + installation on every build. For now, this is good enough. 

// .noinit only applies to SRAM?
// .noinit persists on reboot, e.g. reboot counter variable
