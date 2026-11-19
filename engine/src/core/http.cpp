#include "core/http.hpp"

#include <curl/curl.h>

#include <algorithm>
#include <chrono>
#include <thread>

#include "core/util.hpp"

namespace fomo {

namespace {
size_t write_cb(char* ptr, size_t size, size_t nmemb, void* userdata) {
  auto* out = static_cast<std::string*>(userdata);
  out->append(ptr, size * nmemb);
  return size * nmemb;
}
}  // namespace

std::string host_of(const std::string& url) {
  auto p = url.find("://");
  size_t start = (p == std::string::npos) ? 0 : p + 3;
  auto end = url.find_first_of("/?:", start);
  return url.substr(start, end == std::string::npos ? std::string::npos : end - start);
}

CurlHttp::CurlHttp() { curl_global_init(CURL_GLOBAL_DEFAULT); }
CurlHttp::~CurlHttp() { curl_global_cleanup(); }

void CurlHttp::add_bucket(const std::string& prefix, const std::string& bucket, int gap_ms) {
  std::lock_guard<std::mutex> lock(mu_);
  prefix_bucket_[prefix] = bucket;
  gap_ms_[bucket] = gap_ms;
}

std::string CurlHttp::bucket_of(const std::string& url) const {
  auto p = url.find("://");
  std::string hp = url.substr(p == std::string::npos ? 0 : p + 3);
  std::string best = host_of(url);
  size_t best_len = 0;
  for (const auto& [prefix, bucket] : prefix_bucket_) {
    if (prefix.size() > best_len && hp.compare(0, prefix.size(), prefix) == 0) {
      best = bucket;
      best_len = prefix.size();
    }
  }
  return best;
}

void CurlHttp::wait_turn(const std::string& bucket) {
  int64_t slot = 0;
  {
    std::lock_guard<std::mutex> lock(mu_);
    auto g = gap_ms_.find(bucket);
    const int gap = g != gap_ms_.end() ? g->second : default_gap_ms_;
    const int64_t now = now_ms();
    slot = std::max(now, next_slot_ms_[bucket]);
    next_slot_ms_[bucket] = slot + gap;
  }
  const int64_t wait = slot - now_ms();
  if (wait > 0) std::this_thread::sleep_for(std::chrono::milliseconds(wait));
}

HttpResponse CurlHttp::perform(const std::string& url, const std::string* body,
                               const std::vector<std::string>& headers) {
  const std::string host = host_of(url);
  std::string bucket;
  {
    std::lock_guard<std::mutex> lock(mu_);
    bucket = bucket_of(url);
  }
  const int64_t t0 = now_ms();
  HttpResponse r;
  int backoff_ms = 1000;
  bool was_limited = false;
  for (int attempt = 0; attempt < 3; ++attempt) {
    wait_turn(bucket);
    CURL* c = curl_easy_init();
    if (!c) {
      r.error = "curl_easy_init failed";
      break;
    }
    std::string buf;
    struct curl_slist* hdrs = nullptr;
    hdrs = curl_slist_append(hdrs, "Accept: application/json");
    hdrs = curl_slist_append(hdrs, "User-Agent: fomo-family-engine/0.1");
    if (body) hdrs = curl_slist_append(hdrs, "Content-Type: application/json");
    for (const auto& h : headers) hdrs = curl_slist_append(hdrs, h.c_str());
    curl_easy_setopt(c, CURLOPT_URL, url.c_str());
    curl_easy_setopt(c, CURLOPT_HTTPHEADER, hdrs);
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, write_cb);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &buf);
    curl_easy_setopt(c, CURLOPT_TIMEOUT_MS, 15000L);
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT_MS, 8000L);
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_ACCEPT_ENCODING, "");
    if (body) {
      curl_easy_setopt(c, CURLOPT_POST, 1L);
      curl_easy_setopt(c, CURLOPT_POSTFIELDS, body->c_str());
    }
    CURLcode rc = curl_easy_perform(c);
    long status = 0;
    if (rc == CURLE_OK) curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(hdrs);
    curl_easy_cleanup(c);
    r.status = status;
    r.body = std::move(buf);
    r.error = rc == CURLE_OK ? "" : curl_easy_strerror(rc);
    const bool retry = (rc != CURLE_OK) || status == 429 || status >= 500;
    if (status == 429) was_limited = true;
    if (!retry) break;
    std::this_thread::sleep_for(std::chrono::milliseconds(backoff_ms));
    backoff_ms *= 2;
  }
  r.ms = static_cast<double>(now_ms() - t0);
  std::lock_guard<std::mutex> lock(mu_);
  auto& s = stats_[host];
  s.calls++;
  s.total_ms += r.ms;
  s.last_ms = r.ms;
  if (was_limited) s.rate_limited++;
  if (!r.ok()) {
    s.errors++;
    s.last_error_ms = now_ms();
  }
  return r;
}

HttpResponse CurlHttp::get(const std::string& url) { return perform(url, nullptr, {}); }

HttpResponse CurlHttp::post_json(const std::string& url, const std::string& body,
                                 const std::vector<std::string>& headers) {
  return perform(url, &body, headers);
}

std::map<std::string, HostStats> CurlHttp::stats() const {
  std::lock_guard<std::mutex> lock(mu_);
  return stats_;
}

}  // namespace fomo
