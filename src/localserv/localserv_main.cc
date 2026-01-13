#include <cctype>
#include <cerrno>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <fildesh/fildesh.h>
#include <fildesh/ostream.hh>
#include <fildesh/string.hh>

#include "src/chat/display.hh"
#include "src/chat/opt.hh"
#include "src/chat/trajectory.hh"
#include "src/language/inference.hh"
#include "src/language/vocabulary.hh"

extern "C" {
#include <fildesh/fildesh_compat_errno.h>
}

#ifndef _MSC_VER
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#else
#include <winsock2.h>
typedef SSIZE_T ssize_t;
typedef int socklen_t;
#endif

using rendezllama::ChatDisplay;
using rendezllama::ChatOptions;
using rendezllama::ChatTrajectory;
using rendezllama::GlobalScope;
using rendezllama::Inference;
using rendezllama::Vocabulary;

#ifndef _MSC_VER
typedef int fildesh_compat_socket_t;
#define FILDESH_COMPAT_SOCKET_NULL -1
static bool fildesh_compat_socket_ok(fildesh_compat_socket_t sockfd) {
  return sockfd > 0;
}
#else
typedef SOCKET fildesh_compat_socket_t;
#define FILDESH_COMPAT_SOCKET_NULL INVALID_SOCKET
static bool fildesh_compat_socket_ok(fildesh_compat_socket_t sockfd) {
  return sockfd != INVALID_SOCKET;
}
#endif


static inline void fildesh_compat_socket_close(fildesh_compat_socket_t sockfd) {
  if (!fildesh_compat_socket_ok(sockfd)) {return;}
#ifndef _MSC_VER
  shutdown(sockfd, SHUT_RDWR);
  close(sockfd);
#else
  shutdown(sockfd, SD_BOTH);
  closesocket(sockfd);
#endif
}

static void set_socket_timeout(fildesh_compat_socket_t fd, int seconds) {
#ifdef _MSC_VER
  DWORD timeout = seconds * 1000;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
#else
  struct timeval tv;
  tv.tv_sec = seconds;
  tv.tv_usec = 0;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof(tv));
#endif
}


static
  fildesh_compat_socket_t
setup_socket(const char* hostname, int* port)
{
  // Create a socket.
  fildesh_compat_socket_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (!fildesh_compat_socket_ok(sockfd)) {
    fildesh_compat_errno_trace();
    return FILDESH_COMPAT_SOCKET_NULL;
  }

  int yes = 1;
  if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes))) {
    fildesh_compat_errno_trace();
  }

  // Bind the socket to a port.
  struct sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(*port);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (0 != bind(sockfd, (struct sockaddr *)&addr, sizeof(addr))) {
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(sockfd);
    return FILDESH_COMPAT_SOCKET_NULL;
  }

  // Listen for connections.
  if (0 != listen(sockfd, 5)) {
    fildesh_compat_errno_trace();
    fildesh_compat_socket_close(sockfd);
    return FILDESH_COMPAT_SOCKET_NULL;
  }

  if (*port == 0) {
    socklen_t len = sizeof(addr);
    if (getsockname(sockfd, (struct sockaddr *)&addr, &len) == -1) {
      fildesh_compat_errno_trace();
      fildesh_compat_socket_close(sockfd);
      return FILDESH_COMPAT_SOCKET_NULL;
    }
    *port = ntohs(addr.sin_port);
  }

  return sockfd;
}

static fildesh_compat_socket_t listening_socket_fd = FILDESH_COMPAT_SOCKET_NULL;

static
  void
exit_signal_fn(int sig)
{
  fildesh_compat_socket_t sockfd = listening_socket_fd;
  listening_socket_fd = -1;
  fildesh_compat_socket_close(sockfd);
}

static void print_usage(const char* argv0) {
  fprintf(stderr, "usage: %s --model <model_path> [--context_length <n>] [--o_http_port <filepath>]\n", argv0);
}

static std::string extract_json_string(const std::string& json, const std::string& key) {
  std::string pattern = "\"" + key + "\":";
  size_t start = json.find(pattern);
  if (start == std::string::npos) return "";
  start += pattern.length();

  // Skip whitespace
  while (start < json.length() && isspace(json[start])) start++;

  if (start >= json.length() || json[start] != '"') return "";
  start++; // skip opening quote

  size_t end = start;
  while (end < json.length()) {
    if (json[end] == '"' && json[end-1] != '\\') break;
    end++;
  }

  if (end >= json.length()) return "";

  return json.substr(start, end - start);
}

int main(int argc, char** argv) {
#ifdef _MSC_VER
  WSADATA wsa_data;
  if (0 != WSAStartup(MAKEWORD(2,2), &wsa_data)) {
    fildesh_compat_errno_trace();
    return 1;
  }
#endif

  GlobalScope global_scope;
  ChatOptions opt;
  std::string o_http_port_filepath;

  int argi = 1;
  while (argi < argc) {
    if (strcmp(argv[argi], "--model") == 0) {
      argi += 1;
      if (argi == argc) {
        print_usage(argv[0]);
        return 1;
      }
      opt.model_filename = argv[argi];
    } else if (strcmp(argv[argi], "--context_length") == 0) {
      argi += 1;
      if (argi == argc) {
        print_usage(argv[0]);
        return 1;
      }
      opt.context_token_limit = std::stoi(argv[argi]);
    } else if (strcmp(argv[argi], "--o_http_port") == 0) {
      argi += 1;
      if (argi == argc) {
        print_usage(argv[0]);
        return 1;
      }
      o_http_port_filepath = argv[argi];
    } else {
      print_usage(argv[0]);
      return 1;
    }
    argi += 1;
  }

  if (opt.model_filename.empty()) {
    print_usage(argv[0]);
    return 1;
  }

  // Turn off llama.cpp logging
  llama_log_set(
      [](enum ggml_log_level level, const char* text, void* /* user_data */) {
        if (level >= GGML_LOG_LEVEL_ERROR) {
          fprintf(stderr, "%s", text);
        }
      },
      nullptr);

  auto [model, ctx] = rendezllama::make_llama_context(opt);
  if (!model) { return 1; }
  if (!ctx) { return 1; }

  Vocabulary vocabulary(model);
  ChatDisplay chat_disp;
  chat_disp.out_ = open_FildeshOF("/dev/stderr");
  ChatTrajectory chat_traj(vocabulary.eos_token_id());
  Inference inference(vocabulary);

  // Configure sampling
  // Use deterministic (greedy) sampling for reproducibility
  opt.infer_via.emplace<rendezllama::inference::Sampling>();
  auto& sampling = std::get<rendezllama::inference::Sampling>(opt.infer_via);
  sampling.pick_via = rendezllama::inference::Determinism();

  std::vector<Vocabulary::ChatMessage> messages;
  std::vector<char> formatted(llama_n_ctx(ctx));
  int prev_len = 0;

  int port = 8080;
  if (!o_http_port_filepath.empty()) {
    port = 0;
  }

  listening_socket_fd = setup_socket("localhost", &port);
  signal(SIGINT, exit_signal_fn);

  if (!o_http_port_filepath.empty()) {
    fildesh::ofstream out(o_http_port_filepath.c_str());
    out << port << std::endl;
  }

  std::cerr << "Listening on http://localhost:" << port << std::endl;

  // Accept connections and handle requests.
  while (fildesh_compat_socket_ok(listening_socket_fd)) {
    fildesh_compat_socket_t fd = accept(listening_socket_fd, NULL, NULL);
    if (!fildesh_compat_socket_ok(fd)) {
      break;
    }
    set_socket_timeout(fd, 3);

    // Read the request from the client.
    char buf[16384]; // Larger buffer for chat messages
    std::string request_str;
    size_t body_start_pos = std::string::npos;
    size_t content_length = 0;
    bool headers_received = false;

    ssize_t n;
    while (true) {
      n = recv(fd, buf, sizeof(buf), 0);
      if (n < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
          fildesh_compat_errno_trace();
        }
        break;
      }
      if (n == 0) break; // Connection closed

      request_str.append(buf, n);

      if (!headers_received) {
        body_start_pos = request_str.find("\r\n\r\n");
        if (body_start_pos != std::string::npos) {
          headers_received = true;
          body_start_pos += 4; // Skip the CRLFCRLF

          // Parse Content-Length
          std::string header_str = request_str.substr(0, body_start_pos);
          std::istringstream header_stream(header_str);
          std::string line;
          while (std::getline(header_stream, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();

            std::string key_check = "Content-Length:";
            if (line.size() > key_check.size()) {
              bool match = true;
              for (size_t i = 0; i < key_check.size(); ++i) {
                if (std::tolower((unsigned char)line[i]) != std::tolower((unsigned char)key_check[i])) {
                  match = false;
                  break;
                }
              }
              if (match) {
                try {
                  content_length = std::stoul(line.substr(key_check.size()));
                } catch (...) {}
              }
            }
          }
        }
      }

      if (headers_received) {
        if (request_str.length() >= body_start_pos + content_length) {
          break; // We have the full message
        }
      }
    }

    if (n < 0) {
      fildesh_compat_socket_close(fd);
      continue;
    }

    bool handled = false;
    if (!request_str.empty()) {
      std::istringstream iss(request_str);
      std::string method, path;
      iss >> method >> path;

      std::cerr << "Request method: " << method << " path: " << path << std::endl;

      if (method == "GET") {
        const char* filename = NULL;
        const char* content_type = "text/plain";
        if (path == "/" || path == "/index.html") {
          filename = "index.html";
          content_type = "text/html";
        } else if (path == "/index.js") {
          filename = "index.js";
          content_type = "application/javascript";
        }

        if (filename) {
          std::string open_filename = filename;
          std::ifstream f(open_filename.c_str(), std::ios::binary);
          if (!f) {
            open_filename = "src/localserv/";
            open_filename += filename;
            f.open(open_filename.c_str(), std::ios::binary);
          }
          if (f) {
            std::ostringstream header_ss;
            header_ss << "HTTP/1.1 200 OK\r\n";
            header_ss << "Content-Type: " << content_type << "\r\n";
            f.seekg(0, std::ios::end);
            size_t len = f.tellg();
            f.seekg(0, std::ios::beg);
            header_ss << "Content-Length: " << len << "\r\n";
            header_ss << "\r\n";
            std::string header = header_ss.str();
            send(fd, header.c_str(), header.length(), 0);

            while (f.read(buf, sizeof(buf))) {
              send(fd, buf, f.gcount(), 0);
            }
            if (f.gcount() > 0) {
              send(fd, buf, f.gcount(), 0);
            }
            handled = true;
          }
        }
      } else if (method == "POST" && path == "/chat") {
        // Find body start
        size_t body_pos = request_str.find("\r\n\r\n");
        if (body_pos != std::string::npos) {
          std::string body = request_str.substr(body_pos + 4);
          std::string user_msg = extract_json_string(body, "message");

          if (!user_msg.empty()) {
            std::cerr << "User: " << user_msg << std::endl;

            messages.push_back({"user", user_msg});

            int new_len = vocabulary.chat_apply_template(messages, formatted, true);
            if (new_len < 0) {
              std::cerr << "Using fallback chat template" << std::endl;
              std::string prompt = "\nUser: " + user_msg + "\nAssistant:";
              chat_traj.tokenize_append(prompt, vocabulary);
            } else {
              std::string delta(formatted.begin() + prev_len, formatted.begin() + new_len);

              // Tokenize with special=true because chat template output contains special tokens (e.g. <s>, [INST])
              // Vocabulary::tokenize_to disables special tokens, so we use llama_tokenize directly.
              std::vector<llama_token> tokens(delta.size() + 1);
              int n_tokens = llama_tokenize(llama_model_get_vocab(model), delta.c_str(), delta.size(), tokens.data(), tokens.size(), false, true);
              if (n_tokens < 0) {
                tokens.resize(-n_tokens);
                n_tokens = llama_tokenize(llama_model_get_vocab(model), delta.c_str(), delta.size(), tokens.data(), tokens.size(), false, true);
              }
              tokens.resize(n_tokens);

              std::vector<Vocabulary::Token_id> trajectory_tokens;
              trajectory_tokens.reserve(tokens.size());
              for (auto t : tokens) {
                trajectory_tokens.push_back(t);
              }

              chat_traj.insert_all_at(chat_traj.token_count(), trajectory_tokens);
              // Advance display counter so we don't echo user input/template parts
              chat_traj.display_token_count_ = chat_traj.token_count();
            }

            const size_t response_start_idx = chat_traj.token_count();

            if (inference.commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
              // Generation loop
              int tokens_generated = 0;
              while (tokens_generated < 512) {
                inference.sample_to_trajectory(chat_traj, ctx, false);
                Vocabulary::Token_id new_token_id = chat_traj.token();
                tokens_generated++;

                if (new_token_id == vocabulary.eos_token_id()) {
                  break;
                }

                if (!inference.commit_to_context(ctx, chat_disp, chat_traj, opt, model)) {
                  break;
                }
              }

              // Extract response
              fildesh::ostringstream response_ss;
              for (size_t i = response_start_idx; i < chat_traj.token_count(); ++i) {
                if (chat_traj.token_at(i) != vocabulary.eos_token_id()) {
                  vocabulary.detokenize_to(response_ss.c_struct(), chat_traj.token_at(i));
                }
              }
              std::string_view response_view = response_ss.view();
              std::string response(response_view);

              std::cerr << "Assistant: " << response << std::endl;

              messages.push_back({"assistant", response});
              int temp_len = vocabulary.chat_apply_template(messages, formatted, false);
              if (temp_len >= 0) {
                prev_len = temp_len;
              }

              // Send response
              std::ostringstream json_ss;
              // Basic JSON escaping
              json_ss << "{\"reply\": \"";
              for (char c : response) {
                if (c == '"') json_ss << "\\\"";
                else if (c == '\\') json_ss << "\\\\";
                else if (c == '\n') json_ss << "\\n";
                else if (c == '\r') json_ss << "\\r";
                else if (c == '\t') json_ss << "\\t";
                else if ((unsigned char)c < 32) {} // Ignore other control chars
                else json_ss << c;
              }
              json_ss << "\"}";

              std::string json_resp = json_ss.str();
              std::ostringstream header_ss;
              header_ss << "HTTP/1.1 200 OK\r\n";
              header_ss << "Content-Type: application/json\r\n";
              header_ss << "Content-Length: " << json_resp.length() << "\r\n";
              header_ss << "\r\n";

              send(fd, header_ss.str().c_str(), header_ss.str().length(), 0);
              send(fd, json_resp.c_str(), json_resp.length(), 0);
              handled = true;
            } else {
              std::cerr << "commit_to_context failed" << std::endl;
            }
          }
        }
      }
    }

    if (!handled) {
      // Fallback: 404 Not Found
      std::string resp = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
      send(fd, resp.c_str(), resp.length(), 0);
    }

    fildesh_compat_socket_close(fd);
  }

  fildesh_compat_socket_close(listening_socket_fd);
  llama_free(ctx);
  llama_model_free(model);
#ifdef _MSC_VER
  WSACleanup();
#endif
  return 0;
}
