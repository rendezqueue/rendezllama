import json
import socket
import subprocess
import sys
import time
import os
import urllib.request
import urllib.error

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 localserv_test.py <path_to_localserv_executable> [args...]")
        sys.exit(1)

    executable_path = sys.argv[1]
    extra_args = sys.argv[2:]

    # Locate src/localserv directory relative to this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    src_dir = os.path.normpath(os.path.join(script_dir, '../../src/localserv'))

    if not os.path.isdir(src_dir):
        print(f"Error: Could not find src directory: {src_dir}")
        sys.exit(1)

    # Create pipe for port communication
    r_fd, w_fd = os.pipe()
    port_arg = f"/dev/fd/{w_fd}"

    # Start the server
    cmd = [executable_path] + extra_args + ["--o_http_port", port_arg]
    print(f"Starting server: {cmd} in {src_dir}")
    process = subprocess.Popen(
        cmd,
        cwd=src_dir,
        stdout=sys.stdout,
        stderr=sys.stderr,
        pass_fds=(w_fd,),
    )

    # Close write end in parent
    os.close(w_fd)

    # Read port from pipe
    try:
        with os.fdopen(r_fd, 'r') as f:
            port_str = f.readline().strip()
            if not port_str:
                raise ValueError("Failed to read port from pipe")
            port = int(port_str)
            print(f"Server listening on port {port}")
    except Exception as e:
        print(f"Failed to read port: {e}")
        process.terminate()
        sys.exit(1)

    # Wait for server to start up (connect to the dynamic port)
    start_time = time.time()
    while time.time() - start_time < 20:
        try:
            with socket.create_connection(('localhost', port), timeout=1):
                break
        except (ConnectionRefusedError, socket.timeout, OSError):
            time.sleep(0.5)
    else:
        print("Failed to connect to server")
        process.terminate()
        sys.exit(1)

    try:
        # Test 1: Serve index.html
        print("\nTest 1: Serve index.html")
        expected_html = b""
        with open(os.path.join(src_dir, 'index.html'), 'rb') as f:
            expected_html = f.read()

        with urllib.request.urlopen(f'http://localhost:{port}/index.html') as response:
            content = response.read()
            if content == expected_html:
                print("PASS: index.html served correctly")
            else:
                print(f"FAIL: index.html content mismatch. Got {len(content)} bytes, expected {len(expected_html)} bytes.")
                sys.exit(1)

        # Test 2: Serve index.js
        print("\nTest 2: Serve index.js")
        expected_js = b""
        with open(os.path.join(src_dir, 'index.js'), 'rb') as f:
            expected_js = f.read()

        with urllib.request.urlopen(f'http://localhost:{port}/index.js') as response:
            content = response.read()
            if content == expected_js:
                print("PASS: index.js served correctly")
            else:
                print(f"FAIL: index.js content mismatch. Got {len(content)} bytes, expected {len(expected_js)} bytes.")
                sys.exit(1)

        # Test 3: Chat functionality
        print("\nTest 3: Chat functionality")
        req = urllib.request.Request(
            f'http://localhost:{port}/chat',
            data=json.dumps({'message': 'Hello'}).encode('utf-8'),
            headers={'Content-Type': 'application/json'},
            method='POST'
        )

        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode('utf-8'))
            print(f"Received response: {data}")

            expected_reply = "Hello! How can I assist you today?"
            if 'reply' in data and data['reply'] == expected_reply:
                print("PASS: Chat response received")
            else:
                print(f"FAIL: Invalid chat response. Expected '{expected_reply}', got '{data.get('reply')}'")
                sys.exit(1)

        # Test 4: 404 for unknown path
        print("\nTest 4: 404 for unknown path")
        try:
            urllib.request.urlopen(f'http://localhost:{port}/unknown')
            print("FAIL: Expected 404")
            sys.exit(1)
        except urllib.error.HTTPError as e:
            if e.code == 404:
                print("PASS: Got 404 as expected")
            else:
                print(f"FAIL: Expected 404, got {e.code}")
                sys.exit(1)

        # Test 5: GET /settings
        print("\nTest 5: GET /settings")
        with urllib.request.urlopen(f'http://localhost:{port}/settings') as response:
            data = json.loads(response.read().decode('utf-8'))
            print(f"Received settings: {data}")
            if 'context_length' in data and isinstance(data['context_length'], int):
                 print("PASS: Settings received")
            else:
                 print("FAIL: Invalid settings response")
                 sys.exit(1)

        # Test 6: POST /reset
        print("\nTest 6: POST /reset")
        req = urllib.request.Request(
            f'http://localhost:{port}/reset',
            data=b"",
            method='POST'
        )
        with urllib.request.urlopen(req) as response:
            if response.status == 200:
                print("PASS: Reset successful")
            else:
                print(f"FAIL: Reset failed with {response.status}")
                sys.exit(1)

        # Test 7: POST /settings (change context length)
        print("\nTest 7: POST /settings (change context length)")
        # Pick a value different from default (2048 or whatever)
        new_ctx = 1024
        req = urllib.request.Request(
            f'http://localhost:{port}/settings',
            data=json.dumps({'context_length': new_ctx}).encode('utf-8'),
            headers={'Content-Type': 'application/json'},
            method='POST'
        )
        with urllib.request.urlopen(req) as response:
            if response.status == 200:
                print("PASS: Settings update successful")
            else:
                 print(f"FAIL: Settings update failed with {response.status}")
                 sys.exit(1)

        # Verify change
        with urllib.request.urlopen(f'http://localhost:{port}/settings') as response:
            data = json.loads(response.read().decode('utf-8'))
            if data.get('context_length') == new_ctx:
                 print("PASS: Context length updated")
            else:
                 print(f"FAIL: Context length not updated. Expected {new_ctx}, got {data.get('context_length')}")
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
