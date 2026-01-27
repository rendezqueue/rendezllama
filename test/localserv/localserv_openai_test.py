import json
import socket
import subprocess
import sys
import time
import os
import tempfile
import urllib.request
import urllib.error
import threading
from http.server import HTTPServer, BaseHTTPRequestHandler

class MockOpenAIHandler(BaseHTTPRequestHandler):
    def do_POST(self):
        try:
            content_len = int(self.headers.get('Content-Length', 0))
            body = self.rfile.read(content_len)
            data = json.loads(body)

            # Verify request structure
            if "messages" not in data or "model" not in data:
                self.send_error(400, "Bad Request: Missing messages or model")
                return

            # Check auth header
            auth_header = self.headers.get('Authorization')
            if auth_header != "Bearer mock-key":
                self.send_error(401, "Unauthorized")
                return

            response = {
                "choices": [
                    {
                        "message": {
                            "role": "assistant",
                            "content": "I am a mock OpenAI agent."
                        }
                    }
                ]
            }
            resp_json = json.dumps(response).encode('utf-8')

            self.send_response(200)
            self.send_header('Content-Type', 'application/json')
            self.send_header('Content-Length', str(len(resp_json)))
            self.end_headers()
            self.wfile.write(resp_json)
        except Exception as e:
            self.send_error(500, str(e))

def start_mock_server(port):
    server = HTTPServer(('localhost', port), MockOpenAIHandler)
    server_thread = threading.Thread(target=server.serve_forever)
    server_thread.daemon = True
    server_thread.start()
    return server

def main():
    if len(sys.argv) < 2:
        print("Usage: python3 localserv_openai_test.py <path_to_localserv_executable> [args...]")
        sys.exit(1)

    executable_path = sys.argv[1]
    extra_args = sys.argv[2:]

    # Locate src/localserv directory relative to this script
    script_dir = os.path.dirname(os.path.abspath(__file__))
    src_dir = os.path.normpath(os.path.join(script_dir, '../../src/localserv'))

    if not os.path.isdir(src_dir):
        print(f"Error: Could not find src directory: {src_dir}")
        sys.exit(1)

    # Start Mock OpenAI Server
    # Find a free port
    mock_port = 0
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(('', 0))
        mock_port = s.getsockname()[1]

    mock_server = start_mock_server(mock_port)
    print(f"Mock OpenAI server started on port {mock_port}")

    # Create a temporary file for port communication
    port_file_fd, port_file_path = tempfile.mkstemp()
    os.close(port_file_fd)

    process = None
    try:
        # Start localserv
        cmd = [executable_path] + extra_args + ["--o_http_port", port_file_path]
        print(f"Starting server: {cmd} in {src_dir}")

        process = subprocess.Popen(
            cmd,
            cwd=src_dir,
            stdout=sys.stdout,
            stderr=sys.stderr,
        )

        # Wait for port
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
            except:
                pass
            time.sleep(0.1)

        if port == 0:
            print("Failed to read port from file")
            sys.exit(1)

        print(f"Localserv listening on port {port}")

        # Wait for startup
        start_time = time.time()
        while time.time() - start_time < 20:
            try:
                with socket.create_connection(('localhost', port), timeout=1):
                    break
            except:
                time.sleep(0.5)
        else:
            print("Failed to connect to localserv")
            sys.exit(1)

        # Test 1: Configure OpenAI settings
        print("\nTest 1: Configure OpenAI settings")
        mock_url = f"http://localhost:{mock_port}/v1/chat/completions"
        settings = {
            "openai_api_url": mock_url,
            "openai_api_key": "mock-key",
            "openai_model": "mock-model"
        }

        req = urllib.request.Request(
            f'http://localhost:{port}/settings',
            data=json.dumps(settings).encode('utf-8'),
            headers={'Content-Type': 'application/json'},
            method='POST'
        )

        with urllib.request.urlopen(req) as response:
            if response.status != 200:
                print(f"FAIL: Settings update failed with {response.status}")
                sys.exit(1)
            print("PASS: Settings updated")

        # Verify settings persisted (in memory)
        with urllib.request.urlopen(f'http://localhost:{port}/settings') as response:
            data = json.loads(response.read().decode('utf-8'))
            if data.get('openai_api_url') == mock_url and data.get('openai_api_key') == "mock-key":
                print("PASS: Settings verified")
            else:
                print(f"FAIL: Settings mismatch: {data}")
                sys.exit(1)

        # Test 2: Chat via OpenAI
        print("\nTest 2: Chat via OpenAI")
        chat_req = {
            "message": "Hello OpenAI"
        }
        req = urllib.request.Request(
            f'http://localhost:{port}/chat',
            data=json.dumps(chat_req).encode('utf-8'),
            headers={'Content-Type': 'application/json'},
            method='POST'
        )

        with urllib.request.urlopen(req) as response:
            data = json.loads(response.read().decode('utf-8'))
            print(f"Received response: {data}")

            expected_reply = "I am a mock OpenAI agent."
            if data.get('reply') == expected_reply:
                print("PASS: Received mock response")
            else:
                print(f"FAIL: Unexpected response: {data.get('reply')}")
                sys.exit(1)

    except Exception as e:
        print(f"Test failed with exception: {e}")
        import traceback
        traceback.print_exc()
        if process:
            process.terminate()
        sys.exit(1)
    finally:
        if process:
            print("Terminating server...")
            process.terminate()
            process.wait()

        if os.path.exists(port_file_path):
            try:
                os.remove(port_file_path)
            except:
                pass

if __name__ == "__main__":
    main()
