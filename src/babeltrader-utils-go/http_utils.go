package babeltrader_utils_go

import (
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"net/http"
	"strings"

	common "github.com/MuggleWei/babel-trader/src/babeltrader-common-go"
)

func HttpRequest(client *http.Client, reqType string, data string, url string, requstHeaders map[string]string) ([]byte, error) {
	req, _ := http.NewRequest(reqType, url, strings.NewReader(data))

	if requstHeaders != nil {
		for k, v := range requstHeaders {
			req.Header.Add(k, v)
		}
	}

	resp, err := client.Do(req)
	if err != nil {
		return nil, err
	}

	defer resp.Body.Close()

	bodyData, err := ioutil.ReadAll(resp.Body)
	if err != nil {
		return nil, err
	}

	if resp.StatusCode != 200 {
		return nil, errors.New(fmt.Sprintf("HttpStatusCode:%d ,Desc:%s", resp.StatusCode, string(bodyData)))
	}

	return bodyData, nil
}

func ReadPostBody(r *http.Request, obj interface{}) error {
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		return err
	}

	err = json.Unmarshal(body, obj)
	if err != nil {
		return err
	}

	return nil
}

func HttpResponse(w http.ResponseWriter, msg string, errId int64, errMsg string, data interface{}) {
	rsp := common.MessageRspCommon{
		Message: msg,
		ErrId:   errId,
		ErrMsg:  errMsg,
		Data:    data,
	}

	b, err := json.Marshal(rsp)
	if err != nil {
		rsp = common.MessageRspCommon{
			Message: msg,
			ErrId:   -1,
			ErrMsg:  err.Error(),
		}

		b, _ = json.Marshal(rsp)
		fmt.Fprint(w, string(b))
	}

	fmt.Fprint(w, string(b))
}
