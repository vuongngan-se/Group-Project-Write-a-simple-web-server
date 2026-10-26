# CS631 APUE Group Project: sws (Simple Web Server)

## 1. Member 1: Core Network & Daemon Engine (Vuong Le Bao Ngan)
- **Files**: `main.c`, `network.h`, `network.c`, `daemon.h`, `daemon.c`, `sws.h`, `Makefile`
- **Features**:
  - CLI argument parsing via `getopt(3)`: `-c`, `-d`, `-h`, `-i`, `-l`, `-p`, and docroot `dir`.
  - Dual-stack IPv4 and IPv6 socket binding using `getaddrinfo(3)` with `AI_PASSIVE`.
  - Main event loop with `poll(2)`.
  - Standard APUE daemonization (`setsid`, two-phase `fork`, stdio redirection).
  - Multi-process concurrency (`fork` per request) and single-connection sequential mode (`-d`).
  - Signal handling (`SIGCHLD` reaping with `waitpid`, `SIGTERM`/`SIGINT`, `SIGPIPE` ignore).

---

## 2. Member 2: HTTP Protocol Engine & Response Builder (Tran Loc)
- **Files**: `http.h`, `http.c`, `header.h`, `header.c`, `response.h`, `response.c`
- **Features**:
  - Request line parsing: Method, Request-URI, and HTTP-Version.
  - Supports `GET` and `HEAD` methods; returns 501 Not Implemented for others.
  - Validates relative URIs (RFC 1945 Section 3.2.1) and quietly downgrades `HTTP/1.1` to `HTTP/1.0`.
  - Parses `If-Modified-Since` header supporting RFC 1123, RFC 850, and asctime formats.
  - Generates RFC 1945 status codes (`200 OK`, `304 Not Modified`, `400`, `403`, `404`, `500`, `501`).
  - Builds required HTTP/1.0 response headers (`Date` GMT, `Server`, `Last-Modified` GMT, `Content-Type`, `Content-Length`).
  - Streams file body in chunks for `GET` and omits body for `HEAD`.

---

## 3. Member 3: VFS, Security, User Directories & Directory Indexing (Nguyen Dang Khoi)
- **Files**: `uri.h`, `uri.c`, `dir_index.h`, `dir_index.c`, `mime.h`, `mime.c`
- **Features**:
  - URI resolution against docroot and protection against path traversal attacks (`../` returns 403 Forbidden).
  - User directory resolution: translates `/~<user>/` to `/home/<user>/sws/` via `getpwnam(3)`.
  - Directory Indexing: generates alphanumeric HTML index when `index.html` is absent; ignores dotfiles.
  - MIME type detection using `libmagic` (`magic(5)`) with fallback extension mappings.

---

## 4. Member 4: CGI Engine, Logging Subsystem & DevOps/QA (Ngo Van Lap)
- **Files**: `cgi.h`, `cgi.c`, `logger.h`, `logger.c`, `test_sws.sh`, `checklist`
- **Features**:
  - RFC 3875 Common Gateway Interface (CGI) execution under `/cgi-bin/...` with environment variables.
  - Process isolation using `fork(2)`, `pipe(2)`, `dup2(2)`, and `execl(2)`.
  - Logging subsystem with Apache common format: `%a %t "%r" %>s %b` (GMT ISO 8601 timestamps).
  - Automated test suite (`test_sws.sh`) and course checklist.

---

## 5. Hướng dẫn Biên dịch & Chạy trên Linux (Build & Run Guide)

### 5.1. Cài đặt các gói phụ thuộc (Prerequisites)
Server sử dụng thư viện `libmagic` để nhận diện MIME type. Trên Linux, cài đặt bằng:
* **Ubuntu / Debian / WSL / Kali**:
  ```bash
  sudo apt update
  sudo apt install -y build-essential libmagic-dev curl
  ```
* **Fedora / RHEL / CentOS**:
  ```bash
  sudo dnf install -y gcc make file-devel curl
  ```

### 5.2. Biên dịch (Compilation)
```bash
make clean
make
```
*Lệnh này dùng `gcc` với các cờ nghiêm ngặt `-Wall -Werror -Wextra -Wformat=2 -Wpedantic` và sinh ra file thực thi `./sws`.*

### 5.3. Các chế độ chạy Server (Execution Modes)

#### Chế độ 1: Gỡ lỗi (Debug Mode `-d`) - Chạy foreground, in log ra màn hình
```bash
./sws -d -p 8080 /path/to/docroot
```
*(Dừng server bằng tổ hợp phím `Ctrl + C`)*

#### Chế độ 2: Daemonize (Chạy ngầm dưới nền, ghi log vào file)
```bash
./sws -p 8080 -l sws.log /path/to/docroot
```
* Xem log thời gian thực: `tail -f sws.log`
* Dừng server chạy ngầm: `pkill sws`

#### Chế độ 3: Đầy đủ tính năng CGI (`-c dir`)
```bash
./sws -d -p 8080 -c /path/to/cgi-bin -l sws.log /path/to/docroot
```

### 5.4. Kiểm thử (Testing)

1. **Mở trình duyệt Web**:
   Truy cập: `http://localhost:8080/`

2. **Dùng lệnh `curl`**:
   * Kiểm tra GET request (200 OK):
     ```bash
     curl -i http://localhost:8080/
     ```
   * Kiểm tra HEAD request (chỉ lấy header):
     ```bash
     curl -I http://localhost:8080/index.html
     ```
   * Kiểm tra chống tấn công Path Traversal (403 Forbidden):
     ```bash
     curl -i --path-as-is http://localhost:8080/../../../etc/passwd
     ```
   * Kiểm tra If-Modified-Since (304 Not Modified):
     ```bash
     curl -i -H "If-Modified-Since: Mon, 26 Oct 2026 15:00:00 GMT" http://localhost:8080/index.html
     ```

3. **Chạy kịch bản kiểm thử tự động toàn diện**:
   ```bash
   sh test_sws.sh
   ```
