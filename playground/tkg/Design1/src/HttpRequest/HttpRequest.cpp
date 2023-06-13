#include "HttpRequest.hpp"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <sstream>

#include "Config/validation.h"
#include "HttpException.hpp"
#include "const.hpp"
#include "debug.hpp"
#include "helper.hpp"

bool HttpRequest::methodIs(Method method) const { return method_ == method; };

const std::string &HttpRequest::getBody() const { return body_; }

URI *HttpRequest::getRequestTarget() const { return request_target_; }
const HttpRequest::Method &HttpRequest::getMethod() const { return method_; }
const HttpRequest::Host &HttpRequest::getHost() const { return headers_.host; }
bool HttpRequest::isChunked() {
  std::vector<TransferEncoding> &transferEncodings = headers_.transfer_encodings;
  return std::find(transferEncodings.begin(), transferEncodings.end(), HttpRequest::Chunked) != transferEncodings.end();
}

void HttpRequest::printHeaders() {
  DEBUG_PUTS("HEADER PARSED");
  DEBUG_PRINTF("host: %s \n", (headers_.host.uri_host + ":" + std::to_string(headers_.host.port)).c_str());
  DEBUG_PRINTF("content-length: %zu \n", headers_.content_length);
  DEBUG_PRINTF("transfer-encoding: %s \n", isChunked() ? "chunked" : "none");
  char buf[30];
  std::strftime(buf, sizeof(buf), "%a, %d %b %Y %H:%M:%S", &headers_.date);
  DEBUG_PRINTF("date: %s \n", buf);
}
