#pragma once
// HTTP client used by every data agent. One rate limiter per host, retries
// with backoff on 429/5xx, and per-host health stats for the ApiHealth agent.
#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace fomo {

struct HttpResponse {
  long status = 0;      // 0 = network error
  std::string body;
  double ms = 0;        // wall time incl. retries
  std::string error;
  bool ok() const { return status >= 200 && status < 300; }
};

struct HostStats {
  long calls = 0;
  long errors = 0;      // non-2xx or network error, after retries
  long rate_limited = 0;
  double total_ms = 0;
  double last_ms = 0;
  int64_t last_error_ms = 0;
  double avg_ms() const { return calls ? total_ms / static_cast<double>(calls) : 0; }
  double error_rate() const { return calls ? static_cast<double>(errors) / static_cast<double>(calls) : 0; }
};

class Http {
 public:
  virtual ~Http() = default;
  virtual HttpResponse get(const std::string& url) = 0;
  virtual HttpResponse post_json(const std::string& url, const std::string& body,
                                 const std::vector<std::string>& headers = {}) = 0;
  virtual std::map<std::string, HostStats> stats() const = 0;
};

// Real network client (libcurl). Thread-safe.
class CurlHttp : public Http {
 public:
  CurlHttp();
  ~CurlHttp() override;
  // Minimum gap between requests that share a rate-limit bucket. A bucket is
  // chosen by the longest matching "host/path" prefix, else the bare host.
  // Example: add_bucket("lite-api.jup.ag/tokens", "jup-meta", 13000).
  void add_bucket(const std::string& prefix, const std::string& bucket, int gap_ms);
  void set_default_gap_ms(int gap_ms) { default_gap_ms_ = gap_ms; }
  HttpResponse get(const std::string& url) override;
  HttpResponse post_json(const std::string& url, const std::string& body,
                         const std::vector<std::string>& headers = {}) override;
  std::map<std::string, HostStats> stats() const override;

 private:
  HttpResponse perform(const std::string& url, const std::string* body,
                       const std::vector<std::string>& headers);
  std::string bucket_of(const std::string& url) const;
  void wait_turn(const std::string& bucket);
  mutable std::mutex mu_;
  int default_gap_ms_ = 250;
  std::map<std::string, std::string> prefix_bucket_;
  std::map<std::string, int> gap_ms_;
  std::map<std::string, int64_t> next_slot_ms_;
  std::map<std::string, HostStats> stats_;
};

std::string host_of(const std::string& url);

}  // namespace fomo
