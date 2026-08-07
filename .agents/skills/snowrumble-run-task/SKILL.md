---
name: snowrumble-run-task
description: Run and adapt SnowRumble role-based work safely. Use in the SnowRumble repository when a role Codex starts a session or Task, checks or switches the role branch, synchronizes remote changes, starts or continues a Task, changes priorities, splits or adds a Task, handles a bug or unwanted result, pauses or replaces work, requests a cross-role contract, resolves an ownership conflict, records a handoff, publishes completed work to the role branch and master, or updates the role PLAN and central integration board.
---

# SnowRumble Task Runner

## Establish the role

1. Identify the current person as `S`, `J`, `K`, or `C` from the conversation.
2. If the person is not explicit, ask who this Codex represents and do not mutate project files.
3. Keep the confirmed role for the session until the user explicitly changes it.

## Synchronize the role branch

Before starting or resuming project work, and whenever the confirmed role changes:

1. Inspect the current branch, upstream, and working tree without changing them.
2. Run `git fetch origin --prune` to refresh remote branch state.
3. Map the normal work branch directly from the role: `C`, `K`, `S`, or `J`. Use `master` only for integration.
4. If the current branch differs, switch to the role branch only when the working tree has no staged, tracked, or untracked changes that could be carried across branches. Never auto-stash, auto-commit, reset, or discard work to enable switching.
5. If the role branch has no upstream and the matching `origin/<role>` exists, connect that upstream.
6. Compare the local branch with its upstream after fetch.
7. If the branch is only behind and the working tree is clean, run `git pull --ff-only` automatically.
8. If it is current or only ahead, continue without pulling.
9. If it has diverged, has local changes while behind, lacks the expected remote branch, or fetch/pull fails, stop before project edits and report the exact condition. Do not auto-merge, rebase, force-push, or overwrite files.

Do not ask the developer to run routine fetch, branch switch, upstream setup, or safe fast-forward pull commands that Codex can perform directly.

## Load only the required context

Follow `AGENTS.md` and read the session documents in its stated order. Read the confirmed role's `ROLE_*.md` and `PLAN_*.md`. When acting on a Task, also read:

- the current Task;
- directly required predecessor Tasks and their handoffs;
- only the project documents triggered by the requested action.

Do not load every role's backlog or every Task. Read another role's Task only when it is a direct dependency or handoff source.

## Classify the request

Choose one path from the user's conversation:

- **Run or continue**: select the highest-priority startable Task unless the user chooses another.
- **Reprioritize**: reorder the role PLAN without renaming Task IDs.
- **Change scope**: update the current Task when its result is unchanged; split or add a Task when the work is independently testable or would make the current Task too large.
- **Handle an error**: keep the fix in the current Task when it is required by its completion condition; otherwise create a separate Task with the next unused role ID.
- **Wait or replace**: mark the Task `대기` with a restart condition, or `대체` with a link to the replacement Task.
- **Integrate across roles**: use the cross-role request path below.

The developer describes desired outcomes and problems. Maintain PLAN and Task documents on the developer's behalf.

## First task style calibration

Before a confirmed role starts its first implementation Task in the current role plan:

1. Check the role's `PLAN_*.md` for whether the developer style has already been recorded.
2. If it is not recorded, pause before implementation approval and ask the developer how they want to work:
   - Blueprint-focused;
   - C++-focused;
   - mixed C++ and Blueprint.
3. Summarize the role's current Task queue and the highest-priority startable Tasks so the developer can review whether the order matches their style.
4. Adapt the role `PLAN_*.md` and directly affected role-local Task documents to the confirmed style before starting implementation.
5. Keep stable Task IDs. Reorder priorities, split Task scope, or add role-local notes instead of renaming existing Tasks.
6. For C, update `docs/PLANS.md` if the style adaptation changes public dependencies, contracts, locks, or integration gates.
7. For K, S, or J, record cross-role effects under that role PLAN's `통합 변경 요청` instead of editing central documents directly.

Style adaptation must not weaken server authority, replication ownership, file ownership, or MVP requirements. Blueprint-focused work still uses public C++ contracts for authoritative gameplay state.

## Apply role-local autonomy

Within the confirmed role's owned scope, update its `PLAN_*.md` and Task documents when the conversation clearly establishes the change. Preserve stable Task IDs and record the reason in the plan or Task change history.

Allow role-local changes only when they do not alter:

- the GDD or MVP result;
- a public API, event, replicated state, or authority boundary;
- another role's files, assets, Task order, or ownership;
- a shared `.uasset` or `.umap` assignment;
- the cross-role merge order.

## Route cross-role changes

For `K`, `S`, or `J`, write a concise entry under `통합 변경 요청` in that role's PLAN and stop at the ownership boundary. Do not edit `docs/PLANS.md` or another role's Task.

For `C`, inspect role requests and update `docs/PLANS.md` only when public dependencies, contracts, locks, or integration gates change. Keep role-internal order out of the central board.

## Start implementation safely

When the user asks to implement rather than only plan:

1. Verify required decisions and predecessor contracts.
2. Inspect the actual code and assets before naming candidate files.
3. Confirm feature owner, contract owner, asset editor, and existing locks.
4. Separate local input and presentation, server validation and authority, replicated results, and Blueprint or asset work.
5. Produce the exact implementation approval request required by `docs/WORKFLOW.md`.
6. Wait for approval before implementation.
7. Modify only the approved and locked scope.

Use `docs/STYLEGUIDE.md` for encapsulation and naming. Never duplicate authoritative state in UI, item, or level logic.

## React to conflicts and blockers

- Stop only the overlapping target and preserve both sides' work.
- Never overwrite, revert, rename, move, or auto-merge another owner's file or Unreal binary asset.
- Use the conflict report in `docs/COLLABORATION.md`.
- Mark a Task `대기` only with a concrete reason and restart condition, then select another startable role-local Task if useful.
- Keep GDD ambiguity as a user decision; do not silently invent a game rule.

## Publish completed work

When the user explicitly says the work is complete and asks Codex to push:

1. Treat the statement as human result confirmation for the named work. Identify the completed Task; do not mark unrelated work complete.
2. Verify the current role branch, required tests, Task completion conditions, ownership, locks, and handoff records.
3. Update the role Task and PLAN before committing. For non-C roles, preserve central-document ownership and record any pending integration request in the role PLAN.
4. Fetch origin and stop if the role branch or `master` has unexpected divergence, the remote changed incompatibly, or a required contract or merge-order decision is unresolved.
5. Stage only files owned by the completed Task plus its required records. Never use a broad add when unrelated modifications or untracked files exist, and never include secrets.
6. Commit with a Task-specific message and push the confirmed role branch. If there is nothing new to commit, use the already verified role HEAD.
7. Integrate the pushed role commit into the latest `origin/master` only when the Task is complete, tests passed, ownership is clean, and every public-contract or ordering approval is already recorded.
8. Use a clean integration context. If the role worktree contains unrelated files, use a separate temporary worktree rather than carrying them to `master`.
9. Fast-forward local `master` to `origin/master`, create an identifiable non-fast-forward merge from the pushed role commit, run the required integration checks, and push `master` without force.
10. Fast-forward the role branch to the resulting `master` merge commit when safe, push it, and return the developer's worktree to the role branch.
11. Report the role commit, master merge commit, pushed branches, verification, and any files intentionally excluded.

If role push succeeds but master integration conflicts, validation fails, or a concurrent master push wins the race, keep the role commit and remote branch intact, do not force or invent a resolution, and report that master integration remains pending.

## Verify and close

1. Follow `docs/TESTING.md` for the current verification mode.
2. Update the Task with actual files, APIs, execution locations, manual work, and result checks.
3. Record handoffs with real class, function, event, property, asset, branch, and commit names when available.
4. Update the role PLAN status and change log.
5. If the work changes a public contract, lock, merge order, or integration gate, have `C` update `docs/PLANS.md`.
6. Do not mark completion before the required human result confirmation.
