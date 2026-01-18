import argparse
import subprocess
import sys
import time

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--binary', required=True, help='Path to assistant_cli executable')
    parser.add_argument('--model', required=True, help='Path to model file')
    args = parser.parse_args()

    # Start the process
    # We use -m argument as expected by the binary
    cmd = [args.binary, '--model', args.model]

    print(f"Running: {' '.join(cmd)}")

    # We need to pipe stdin to send input, and stdout to check output.
    process = subprocess.Popen(
        cmd,
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=sys.stderr, # Redirect stderr to this script's stderr so we see logs
        text=True,
        bufsize=0 # Unbuffered to see output immediately if possible
    )

    try:
        # Give it a moment to load the model
        # Note: Interactive testing with Popen can be tricky due to buffering.
        # But since we use text=True and bufsize=0 (although buffering is line-based in text mode usually),
        # we might need to be careful.

        # We can just use communicate() to send one input and get output if the program exits or we don't need a back-and-forth loop in real-time.
        # However, the assistant_cli loop waits for input.
        # Let's try sending one line and closing stdin.
        # But wait, if we close stdin, std::getline will fail/return false, and the loop will break.
        # This is actually perfect for a test.

        input_text = "Say hello.\n"
        stdout_data, stderr_data = process.communicate(input=input_text, timeout=60)

        print("STDOUT output:")
        print(stdout_data)

        if process.returncode != 0:
            print(f"Process exited with error code {process.returncode}")
            sys.exit(1)

        # Basic verification: Check if it printed the assistant response prefix or some text.
        # The code prints "> " prompts and then response.
        if "> " not in stdout_data:
            print("FAILURE: Did not find prompt '> ' in output")
            sys.exit(1)

        expected_response = "Hello! How can I assist you today?"
        if expected_response not in stdout_data:
             print(f"FAILURE: Output does not contain expected string '{expected_response}'")
             sys.exit(1)

        print("SUCCESS: Test passed.")

    except subprocess.TimeoutExpired:
        process.kill()
        stdout_data, stderr_data = process.communicate()
        print("TIMEOUT")
        print(stdout_data)
        sys.exit(1)

if __name__ == '__main__':
    main()
