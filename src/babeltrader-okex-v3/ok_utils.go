package babeltrader_okex_v3

import (
	"bytes"
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base64"
	"encoding/json"
	"errors"
	"io/ioutil"
	"log"
	"net/http"
	"strings"
	"time"
)

/*
the methods in this file just copy from github.com/okcoin-okex/open-api-v3-sdk/okex-go-sdk-api/utils.go
*/

const (
	/*
	  http headers
	*/
	OK_ACCESS_KEY        = "OK-ACCESS-KEY"
	OK_ACCESS_SIGN       = "OK-ACCESS-SIGN"
	OK_ACCESS_TIMESTAMP  = "OK-ACCESS-TIMESTAMP"
	OK_ACCESS_PASSPHRASE = "OK-ACCESS-PASSPHRASE"

	/**
	  paging params
	*/
	OK_FROM  = "OK-FROM"
	OK_TO    = "OK-TO"
	OK_LIMIT = "OK-LIMIT"

	CONTENT_TYPE = "Content-Type"
	ACCEPT       = "Accept"
	COOKIE       = "Cookie"
	LOCALE       = "locale="

	APPLICATION_JSON      = "application/json"
	APPLICATION_JSON_UTF8 = "application/json; charset=UTF-8"

	/*
	  i18n: internationalization
	*/
	ENGLISH            = "en_US"
	SIMPLIFIED_CHINESE = "zh_CN"
	//zh_TW || zh_HK
	TRADITIONAL_CHINESE = "zh_HK"

	/*
	  http methods
	*/
	GET    = "GET"
	POST   = "POST"
	DELETE = "DELETE"

	/*
	 others
	*/
	ResultDataJsonString = "resultDataJsonString"
	ResultPageJsonString = "resultPageJsonString"
)

/*
 Get a iso time
  eg: 2018-03-16T18:02:48.284Z
*/
func IsoTime() string {
	utcTime := time.Now().UTC()
	iso := utcTime.String()
	isoBytes := []byte(iso)
	iso = string(isoBytes[:10]) + "T" + string(isoBytes[11:23]) + "Z"
	return iso
}

/*
 signing a message
 using: hmac sha256 + base64
  eg:
    message = Pre_hash function comment
    secretKey = E65791902180E9EF4510DB6A77F6EBAE

  return signed string = TO6uwdqz+31SIPkd4I+9NiZGmVH74dXi+Fd5X0EzzSQ=
*/
func HmacSha256Base64Signer(message string, secretKey string) (string, error) {
	mac := hmac.New(sha256.New, []byte(secretKey))
	_, err := mac.Write([]byte(message))
	if err != nil {
		return "", err
	}
	return base64.StdEncoding.EncodeToString(mac.Sum(nil)), nil
}

/*
 the pre hash string
  eg:
    timestamp = 2018-03-08T10:59:25.789Z
    method  = POST
    request_path = /orders?before=2&limit=30
    body = {"product_id":"BTC-USD-0309","order_id":"377454671037440"}

  return pre hash string = 2018-03-08T10:59:25.789ZPOST/orders?before=2&limit=30{"product_id":"BTC-USD-0309","order_id":"377454671037440"}
*/
func PreHashString(timestamp string, method string, requestPath string, body string) string {
	return timestamp + strings.ToUpper(method) + requestPath + body
}

/*
 Set http request headers:
   Accept: application/json
   Content-Type: application/json; charset=UTF-8  (default)
   Cookie: locale=en_US        (English)
   OK-ACCESS-KEY: (Your setting)
   OK-ACCESS-SIGN: (Use your setting, auto sign and add)
   OK-ACCESS-TIMESTAMP: (Auto add)
   OK-ACCESS-PASSPHRASE: Your setting
*/
func Headers(request *http.Request, config *Config, timestamp string, sign string) {
	request.Header.Add(ACCEPT, APPLICATION_JSON)
	request.Header.Add(CONTENT_TYPE, APPLICATION_JSON_UTF8)
	request.Header.Add(COOKIE, LOCALE+ENGLISH)
	request.Header.Add(OK_ACCESS_KEY, config.Key)
	request.Header.Add(OK_ACCESS_SIGN, sign)
	request.Header.Add(OK_ACCESS_TIMESTAMP, timestamp)
	request.Header.Add(OK_ACCESS_PASSPHRASE, config.Passphrase)
}

/*
 Get a http request body is a json string and a byte array.
*/
func ParseRequestParams(params interface{}) (string, *bytes.Reader, error) {
	if params == nil {
		return "", nil, errors.New("illegal parameter")
	}
	data, err := json.Marshal(params)
	if err != nil {
		return "", nil, errors.New("json convert string error")
	}
	jsonBody := string(data)
	binBody := bytes.NewReader(data)
	return jsonBody, binBody, nil
}

/*
 Send a http request to remote server and get a response data
*/
func HttpRequst(client *http.Client, config *Config, method string, requestPath string, params, result interface{}) (response *http.Response, err error) {
	// uri
	endpoint := config.RestEndpoint
	if strings.HasSuffix(endpoint, "/") {
		endpoint = endpoint[0 : len(endpoint)-1]
	}
	url := endpoint + requestPath

	// get json and bin styles request body
	var jsonBody string
	var binBody = bytes.NewReader(make([]byte, 0))
	if params != nil {
		jsonBody, binBody, err = ParseRequestParams(params)
		if err != nil {
			return response, err
		}
	}

	// get a http request
	request, err := http.NewRequest(method, url, binBody)
	if err != nil {
		return response, err
	}

	// Sign and set request headers
	timestamp := IsoTime()
	preHash := PreHashString(timestamp, method, requestPath, jsonBody)
	sign, err := HmacSha256Base64Signer(preHash, config.Secret)
	if err != nil {
		return response, err
	}
	Headers(request, config, timestamp, sign)

	// send a request to remote server, and get a response
	response, err = client.Do(request)
	if err != nil {
		return response, err
	}
	defer response.Body.Close()

	// get a response results and parse
	status := response.StatusCode
	message := response.Status
	body, err := ioutil.ReadAll(response.Body)
	if err != nil {
		return response, err
	}

	log.Printf("[Info] recv rest rsp from okex: %v\n", string(body))

	if status >= 200 && status < 300 {
		if body != nil && result != nil {
			err := json.Unmarshal(body, result)
			if err != nil {
				return response, err
			}
		}
		return response, nil
	} else {
		log.Printf("Http error result: status=%v, message=%v\n", status, message)
		return response, errors.New(message)
	}
	return response, nil
}
