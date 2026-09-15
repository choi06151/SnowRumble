import { execFileSync, spawnSync } from "node:child_process";
import { existsSync, mkdirSync, mkdtempSync, readFileSync, rmSync, writeFileSync } from "node:fs";
import { dirname, join, resolve } from "node:path";
import { tmpdir } from "node:os";
import process from "node:process";

function runGit(args) {
  return execFileSync("git", args, { encoding: "utf8" })
    .split(/\r?\n/)
    .map((file) => file.trim())
    .filter(Boolean);
}

function isTextFile(file) {
  return !/\.(uasset|umap|png|jpg|jpeg|gif|wav|mp3|ogg|fbx|pak|dll|exe|lib|obj|pdb)$/i.test(file);
}

const files = (process.argv.length > 2
  ? process.argv.slice(2)
  : runGit(["diff", "--cached", "--name-only", "--diff-filter=ACMR"]))
  .filter(isTextFile);
const usesWorkingTreeFiles = process.argv.length > 2;
if (files.length === 0) {
  console.log("No staged text files to check with EditorConfig.");
  process.exit(0);
}

const checker = resolve("node_modules", "editorconfig-checker", "dist", "index.js");
if (!existsSync(checker)) {
  console.error("editorconfig-checker was not found. Run npm install first.");
  process.exit(1);
}

console.log(`Checking ${files.length} text file(s) with EditorConfig...`);
const temporaryRoot = mkdtempSync(join(tmpdir(), "snowrumble-editorconfig-"));

try {
  const configPath = resolve(".editorconfig");
  writeFileSync(join(temporaryRoot, ".editorconfig"), readFileSync(configPath));

  for (const file of files) {
    const normalizedFile = file.replaceAll("\\", "/");
    const destination = join(temporaryRoot, ...normalizedFile.split("/"));
    mkdirSync(dirname(destination), { recursive: true });
    const content = usesWorkingTreeFiles
      ? readFileSync(normalizedFile)
      : execFileSync("git", ["show", `:${normalizedFile}`]);
    writeFileSync(destination, content);
  }

  const result = spawnSync(process.execPath, [checker, "-no-color"], {
    cwd: temporaryRoot,
    stdio: "inherit",
    shell: false
  });

  process.exitCode = result.status ?? 1;
} finally {
  rmSync(temporaryRoot, { recursive: true, force: true });
}
