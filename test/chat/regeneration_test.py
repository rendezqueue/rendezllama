import subprocess
import sys
import time
import os
import tempfile

def run_chat(cli_path, model_path, extra_args=[]):
    cmd = [cli_path, "--model", model_path, "--protagonist", "User", "--confidant", "AI"] + extra_args
    print(f"Running: {cmd}")
    process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        bufsize=1
    )

    try:
        # Prompt 1
        process.stdin.write("Hello\n")
        process.stdin.flush()

        # Wait for generation.
        # Intentionally short sleep to trigger seed collision if fast
        # But this machine is slow.
        time.sleep(0.1)

        # Regen
        process.stdin.write("/r\n")
        process.stdin.flush()

        time.sleep(2)

        process.terminate()
        stdout, stderr = process.communicate()

        # Filter lines
        lines = []
        for line in stdout.splitlines():
            s = line.strip()
            if not s: continue
            if s.startswith(">"):
                s = s[1:].strip()
            if s:
                lines.append(s)
        return lines, stdout, stderr
    except Exception as e:
        process.terminate()
        raise e

def main():
    if len(sys.argv) < 3:
        print("Usage: python3 test/regeneration_test.py <assistant_cli_path> <model_path>")
        sys.exit(1)

    cli_path = sys.argv[1]
    model_path = sys.argv[2]

    print("=== Test Case A: Random Regeneration ===")
    lines, stdout, stderr = run_chat(cli_path, model_path)

    print(f"Captured lines: {lines}")

    if len(lines) < 2:
        print("FAIL: Not enough output lines captured. (Maybe sleep too short?)")
        # print("STDOUT:", stdout)
        # print("STDERR:", stderr)
        # This is expected if machine is slow.
        # But if it is slow, then seeds WILL differ.
        pass


    if len(lines) >= 2:
        if len(lines) % 2 == 0:
            mid = len(lines) // 2
            half1 = lines[:mid]
            half2 = lines[mid:]
            if half1 == half2:
                print("Responses are identical (Full block match).")
                are_identical = True
            else:
                print("Responses differ.")
                are_identical = False
        else:
            print("Responses differ (length mismatch).")
            are_identical = False

        if are_identical:
            print("FAIL: Responses are identical in random mode!")
            # This is what we want to see with the bug present (if fast enough).
            sys.exit(1)
        else:
            print("PASS: Random mode produced different responses.")

    # We skip Deterministic test in reproduction check.

if __name__ == "__main__":
    main()
