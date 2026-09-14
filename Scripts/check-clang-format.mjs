import { execFileSync, spawnSync } from "node:child_process";
import { existsSync } from "node:fs";
import process from "node:process";

const cppExtensions = new Set([".h", ".cpp"]);

function runGit(args) {
  return execFileSync("git", args, { encoding: "utf8" })
    .split(/\r?\n/)
    .map((file) => file.trim())
    .filter(Boolean);
}

function getFilesToCheck() {
  if (process.argv.length > 2) {
    return process.argv.slice(2);
  }

  if (process.env.GITHUB_ACTIONS === "true") {
    if (process.env.GITHUB_EVENT_NAME === "pull_request" && process.env.GITHUB_BASE_REF) {
      return runGit(["diff", "--name-only", `origin/${process.env.GITHUB_BASE_REF}...HEAD`]);
    }

    const before = process.env.GITHUB_EVENT_BEFORE;
    if (before && !/^0+$/.test(before)) {
      return runGit(["diff", "--name-only", before, "HEAD"]);
    }

    return runGit(["diff", "--name-only", "HEAD^", "HEAD"]);
  }

  return runGit(["diff", "--cached", "--name-only", "--diff-filter=ACMR"]);
}

function isCppSourceFile(file) {
  if (!file.startsWith("Source/")) {
    return false;
  }

  const extensionIndex = file.lastIndexOf(".");
  return extensionIndex >= 0 && cppExtensions.has(file.slice(extensionIndex).toLowerCase());
}

function findClangFormat() {
  const candidates = [
    "clang-format",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\Llvm\\x64\\bin\\clang-format.exe",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\VC\\Tools\\Llvm\\x64\\bin\\clang-format.exe",
    "C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\VC\\Tools\\Llvm\\x64\\bin\\clang-format.exe"
  ];

  for (const candidate of candidates) {
    if (candidate !== "clang-format" && existsSync(candidate)) {
      return candidate;
    }

    if (candidate === "clang-format") {
      const probe = spawnSync(candidate, ["--version"], {
        stdio: "ignore",
        shell: false
      });

      if (probe.status === 0) {
        return candidate;
      }
    }
  }

  return null;
}

const files = getFilesToCheck().filter(isCppSourceFile);
if (files.length === 0) {
  console.log("No changed Unreal C++ files to check.");
  process.exit(0);
}

const clangFormat = findClangFormat();
if (!clangFormat) {
  console.error("clang-format was not found. Install it or add it to PATH.");
  process.exit(1);
}

console.log(`Checking ${files.length} Unreal C++ file(s) with clang-format...`);
const result = spawnSync(clangFormat, ["--dry-run", "--Werror", "--style=file", ...files], {
  stdio: "inherit",
  shell: false
});

process.exit(result.status ?? 1);
