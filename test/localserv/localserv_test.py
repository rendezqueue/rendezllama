import socket
import subprocess
import sys
import time
import os
import urllib.request

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 localserv_test.py <path_to_localserv_executable>")
        sys.exit(1)

    executable_path = sys.argv[1]

    # Locate src/localserv directory relative to this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    src_dir = os.path.normpath(os.path.join(script_dir, '../../src/localserv'))

    if not os.path.isdir(src_dir):
        print(f"Error: Could not find src directory: {src_dir}")
        sys.exit(1)

    # Start the server
    print(f"Starting server: {executable_path} in {src_dir}")
    process = subprocess.Popen(
        [executable_path],
        cwd=src_dir,
        stdout=sys.stdout,
        stderr=sys.stderr,
    )

    # Wait for server to start up
    time.sleep(1)

    try:
        # Test 1: Echo functionality
        print("Test 1: Echo functionality")
        print("Connecting to localhost:8080...")
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.connect(('localhost', 8080))

            message = b"hello"
            print(f"Sending: {message}")
            s.sendall(message)

            data = s.recv(1024)
            print(f"Received: {data}")

            if data != message:
                print(f"FAIL: Expected {message}, got {data}")
                sys.exit(1)
            else:
                print("PASS: Echo works")

        # Test 2: Serve index.html
        print("\nTest 2: Serve index.html")
        expected_html = b""
        with open(os.path.join(src_dir, 'index.html'), 'rb') as f:
            expected_html = f.read()

        with urllib.request.urlopen('http://localhost:8080/index.html') as response:
            content = response.read()
            if content == expected_html:
                print("PASS: index.html served correctly")
            else:
                print(f"FAIL: index.html content mismatch. Got {len(content)} bytes, expected {len(expected_html)} bytes.")
                print(f"Got start: {content[:100]}")
                sys.exit(1)

        # Test 3: Serve index.js
        print("\nTest 3: Serve index.js")
        expected_js = b""
        with open(os.path.join(src_dir, 'index.js'), 'rb') as f:
            expected_js = f.read()

        with urllib.request.urlopen('http://localhost:8080/index.js') as response:
            content = response.read()
            if content == expected_js:
                print("PASS: index.js served correctly")
            else:
                print(f"FAIL: index.js content mismatch. Got {len(content)} bytes, expected {len(expected_js)} bytes.")
                print(f"Got start: {content[:100]}")
                sys.exit(1)

    except Exception as e:
        print(f"Test failed with exception: {e}")
        import traceback
        traceback.print_exc()
        process.terminate()
        sys.exit(1)
    finally:
        # Clean up
        print("Terminating server...")
        process.terminate()
        process.wait()

if __name__ == "__main__":
    main()
