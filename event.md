事件是通过为```wkeWebView```设置回调来进行处理，你可以调用原生接口来重置事件的处理函数，也可以通过继承```CMiniblink```，重载事件处理函数。

# 事件

|名称    |wke对应回调    |    备注|
|---|---|---|
|virtual void OnTitleChanged(LPCWSTR title)|void wkeOnTitleChanged(wkeWebView webView, wkeTitleChangedCallback callback, void* callbackParam)| |
|virtual void OnUrlChanged(LPCWSTR url, CWebFrame *frame)|void wkeOnURLChanged2(wkeWebView webView, wkeURLChangedCallback2 callback, void* callbackParam)| |
|virtual void OnAlert(LPCWSTR msg)|void wkeOnAlertBox(wkeWebView webView, wkeAlertBoxCallback callback, void* callbackParam)| |
|virtual bool OnConfirm(LPCWSTR msg)|void wkeOnConfirmBox(wkeWebView webView, wkeConfirmBoxCallback callback, void* callbackParam)| |
|virtual bool OnPrompt(LPCWSTR msg, LPCWSTR defaultResult, LPCWSTR result)| void wkeOnPromptBox(wkeWebView webView, wkePromptBoxCallback callback, void* callbackParam)| |
|virtual bool OnNavigation(wkeNavigationType navigationType, LPCWSTR url)| void wkeOnNavigation(wkeWebView webView, wkeNavigationCallback callback, void* param)| |
|virtual CMiniblink* OnNewOpen(wkeNavigationType navigationType, LPCWSTR url, const wkeWindowFeatures* windowFeatures)| |当打开一个新窗口前，触发该事件，若要在当前窗口开打，请返回```this```。|
|virtual void OnDocumentReady(CWebFrame* frame)|void wkeOnDocumentReady2(wkeWebView webView, wkeDocumentReady2Callback callback, void* param)| |
|virtual bool OnDownload(const char *url)|void wkeOnDownload(wkeWebView webView, wkeDownloadCallback callback, void* param)| |
|virtual bool OnResponse(const char* url, CNetJob* job)| void wkeNetOnResponse(wkeWebView webView, wkeNetResponseCallback callback, void* param)|  |
