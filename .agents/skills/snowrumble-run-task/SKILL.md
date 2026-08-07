---
name: snowrumble-run-task
description: Run and adapt SnowRumble role-based work safely. Use in the SnowRumble repository when a role Codex needs to start or continue a Task, change priorities, split or add a Task, handle a bug or unwanted result, pause or replace work, request a cross-role contract, resolve an ownership conflict, record a handoff, or update the role PLAN and central integration board.
---

# SnowRumble Task Runner

## Establish the role

1. Identify the current person as `S`, `J`, `K`, or `C` from the conversation.
2. If the person is not explicit, ask who this Codex represents and do not mutate project files.
3. Keep the confirmed role for the session until the user explicitly changes it.

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

## Verify and close

1. Follow `docs/TESTING.md` for the current verification mode.
2. Update the Task with actual files, APIs, execution locations, manual work, and result checks.
3. Record handoffs with real class, function, event, property, asset, branch, and commit names when available.
4. Update the role PLAN status and change log.
5. If the work changes a public contract, lock, merge order, or integration gate, have `C` update `docs/PLANS.md`.
6. Do not mark completion before the required human result confirmation.
