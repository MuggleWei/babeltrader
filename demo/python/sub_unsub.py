import time

import requests

# ctp demo
addr = "127.0.0.1:6001"
sub_msg = {
    "market": "ctp",
    "type": "future",
    "symbol": "rb",
    "contract": "1905"
}

# # xtp demo
# addr = "127.0.0.1:6002"
# sub_msg = {
#     "market": "xtp",
#     "exchange": "SSE",
#     "type": "spot",
#     "symbol": "601857",
# }


def output_subed_topic():
    r = requests.post("http://" + addr + "/topic/get", json={})
    print(r.text)


def sub():
    r = requests.post("http://" + addr + "/topic/sub", json=sub_msg)
    print(r.text)


def unsub():
    r = requests.post("http://" + addr + "/topic/unsub", json=sub_msg)
    print(r.text)

if __name__ == '__main__':
    output_subed_topic()

    sub()
    time.sleep(3)
    output_subed_topic()

    unsub()
    time.sleep(3)
    output_subed_topic()

