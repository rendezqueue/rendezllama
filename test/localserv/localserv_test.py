import json
import socket
import subprocess
import sys
import time
import os
import tempfile
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

    # Create a temporary file for port communication
    # We use a named temporary file but close it so the subprocess can open it
    # On Windows, we can't open it if it's already open, so we close it first.
    port_file_fd, port_file_path = tempfile.mkstemp()
    os.close(port_file_fd)
    # Ensure it's empty
    with open(port_file_path, 'w') as f:
        pass

    process = None
    try:
        # Start the server
        cmd = [executable_path] + extra_args + ["--o_http_port", port_file_path]
        print(f"Starting server: {cmd} in {src_dir}")

        # We don't need pass_fds anymore
        process = subprocess.Popen(
            cmd,
            cwd=src_dir,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )

        # Wait for port to be written
        port = 0
        start_time = time.time()
        while time.time() - start_time < 20:
            if process.poll() is not None:
                print("Server process exited prematurely")
                sys.exit(1)

            try:
                with open(port_file_path, 'r') as f:
                    content = f.read().strip()
                    if content:
                        port = int(content)
                        break
            except ValueError:
                pass
            except Exception as e:
                # File might not be ready or readable yet
                pass
            time.sleep(0.1)

        if port == 0:
            print("Failed to read port from file")
            sys.exit(1)

        print(f"Server listening on port {port}")

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
            sys.exit(1)

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
            if 'reply' in data and data['reply'].strip() == expected_reply:
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
        if process:
            process.terminate()
        sys.exit(1)
    finally:
        # Clean up
        if process:
            print("Terminating server...")
            process.terminate()
            process.wait()

        if os.path.exists(port_file_path):
            try:
                os.remove(port_file_path)
            except OSError:
                pass

if __name__ == "__main__":
    main()
