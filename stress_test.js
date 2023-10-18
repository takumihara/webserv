import { check } from "k6";
import http from "k6/http";

export const options = {
  thresholds: {
    http_req_failed: ["rate<0.01"],
    http_req_duration: ["p(90)<2000"],
  },
};

const url = "http://localhost/html/keijiban/index.html";

export default function () {
  const headers = {
    "Content-Type": "application/json",
  };

  const res = http.get(url);
  check(res, {
    is_status_200: (r) => r.status === 200,
  });
}
