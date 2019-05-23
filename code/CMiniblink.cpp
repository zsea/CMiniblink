#include "CMiniblink.h"

DuiLib::CNetJob::CNetJob(wkeNetJob job) :m_job(job) {}
void DuiLib::CNetJob::SetHTTPHeaderField(wchar_t * key, wchar_t * value, bool response)
{
	::wkeNetSetHTTPHeaderField(m_job, key, value, response);
}
void DuiLib::CNetJob::SetMIMEType(const char * type)
{
	::wkeNetSetMIMEType(m_job, const_cast<char*>(type));
}
const char * DuiLib::CNetJob::GetMIMEType() const
{
	return ::wkeNetGetMIMEType(m_job, NULL);
}
void DuiLib::CNetJob::CancelRequest()
{
	::wkeNetCancelRequest(m_job);
}
wkeRequestType DuiLib::CNetJob::GetRequestMethod()
{
	return ::wkeNetGetRequestMethod(m_job);
}
wkePostBodyElements * DuiLib::CNetJob::GetPostBody()
{
	return ::wkeNetGetPostBody(m_job);
}

void DuiLib::CNetJob::SetResponseData(void* buf, int len)
{
	::wkeNetSetData(m_job, buf, len);
}

DuiLib::CMiniblink::CMiniblink() :DuiLib::CMiniblink(NULL) {

}
DuiLib::CMiniblink::CMiniblink(wkeWebView mbWebView) : m_cursor(-1)
{
	if (mbWebView != NULL) {
		m_wkeWebView = mbWebView;
	}
	else
	{
		Initialize();
		m_wkeWebView = ::wkeCreateWebView();
	}

}

DuiLib::CMiniblink::~CMiniblink()
{
	//this->SetVisible(false);
	m_released = true;
	if (m_wkeWebView != NULL) {
		wkeDestroyWebView(m_wkeWebView);
		m_wkeWebView = NULL;
	}
	GetManager()->RemoveMessageFilter(this);
}

LPCTSTR DuiLib::CMiniblink::GetClass() const
{
	return L"Miniblink";
}

LPVOID DuiLib::CMiniblink::GetInterface(LPCTSTR pstrName)
{
	if (_tcscmp(pstrName, L"Miniblink") == 0) return static_cast<CMiniblink*>(this);
	return CControlUI::GetInterface(pstrName);
}
bool DuiLib::CMiniblink::DoPaint(HDC hDC, const RECT& rcPaint, CControlUI* pStopControl) {
	//CControlUI::DoPaint(hDC, rcPaint, pStopControl);
	if (hDC != NULL) {
		HDC mb_hdc = ::wkeGetViewDC(m_wkeWebView);
		if (mb_hdc != NULL) {
			::BitBlt(hDC, rcPaint.left, rcPaint.top, rcPaint.right - rcPaint.left, rcPaint.bottom - rcPaint.top, mb_hdc, 0, 0, SRCCOPY);
			::ReleaseDC(NULL, mb_hdc);
		}
	}
	updateCursor();
	return true;
}
void DuiLib::CMiniblink::DoInit() {
	::wkeSetTransparent(m_wkeWebView, false);
	::wkeOnPaintUpdated(m_wkeWebView, MBPaintUpdate, this);
	::wkeSetHandle(m_wkeWebView, GetManager()->GetPaintWindow());

	::wkeOnTitleChanged(m_wkeWebView, MBTitleChanged, this);
	::wkeOnURLChanged2(m_wkeWebView, MBUrlChanged, this);
	::wkeOnAlertBox(m_wkeWebView, MBAlert, this);
	::wkeOnConfirmBox(m_wkeWebView, MBConfirm, this);
	::wkeOnPromptBox(m_wkeWebView, MBPromptBox, this);
	::wkeOnNavigation(m_wkeWebView, MBNavigation, this);
	::wkeOnCreateView(m_wkeWebView, MBCreateView, this);
	::wkeOnDocumentReady2(m_wkeWebView, MBDocumentReady, this);
	::wkeOnDownload(m_wkeWebView, MBDownload, this);
	::wkeNetOnResponse(m_wkeWebView, MBNetResponse, this);
	::wkeOnConsole(m_wkeWebView, MBConsole, this);
	::wkeOnLoadUrlBegin(m_wkeWebView, MBLoadUrlBegin, this);
	::wkeOnLoadUrlEnd(m_wkeWebView, MBLoadUrlEnd, this);
	::wkeOnDidCreateScriptContext(m_wkeWebView, MBCreateScriptContext, this);
	::wkeOnWillReleaseScriptContext(m_wkeWebView, MBReleaseScriptContext, this);
	::wkeOnWillMediaLoad(m_wkeWebView, MBMediaLoad, this);
	::wkeOnLoadingFinish(m_wkeWebView, MBLoadedFinish, this);

	GetManager()->AddMessageFilter(this);

}
void  DuiLib::CMiniblink::Goto(LPCTSTR url) {
	if (_tcslen(url) >= 8 && (((url[0] == 'H' || url[0] == 'h') && (url[1] == 'T' || url[1] == 't') && (url[2] == 'T' || url[2] == 't') && (url[3] == 'P' || url[3] == 'p') && url[4] == ':'&&url[5] == '/'&&url[6] == '/')
		|| ((url[0] == 'H' || url[0] == 'h') && (url[1] == 'T' || url[1] == 't') && (url[2] == 'T' || url[2] == 't') && (url[3] == 'P' || url[3] == 'p') && (url[4] == 'S' || url[4] == 's') && url[5] == ':'&&url[6] == '/'&&url[7] == '/')
		)||StrCmp(url,L"about:blank")==0
		)
	{
#ifdef UNICODE
		::wkeLoadURLW(m_wkeWebView, url);
#else
		::wkeLoadURL(m_wkeWebView, url);
#endif
	}
	else {
#ifdef UNICODE
		::wkeLoadFileW(m_wkeWebView, url);
#else
		::wkeLoadFile(m_wkeWebView, url);
#endif
	}
}
void DuiLib::CMiniblink::SetPos(RECT rc, bool bNeedUpdate) {
	CControlUI::SetPos(rc, bNeedUpdate);
	wkeResize(m_wkeWebView, rc.right - rc.left, rc.bottom - rc.top);
}

void DuiLib::CMiniblink::DoEvent(TEventUI& event) {
	if (!this->IsEnabled() || !this->IsVisible()) return;
	RECT rc = GetPos();
	POINT pt = { event.ptMouse.x - rc.left, event.ptMouse.y - rc.top };
	static WkeCursorInfoType cursorInfo = WkeCursorInfoType::WkeCursorInfoPointer;
	switch (event.Type)
	{
	case UIEVENT_MOUSEENTER:
	case UIEVENT_MOUSELEAVE:
	case UIEVENT_MOUSEMOVE:
	{
		unsigned int flags = 0;
		if (event.wParam & MK_CONTROL)
			flags |= WKE_CONTROL;
		if (event.wParam & MK_SHIFT)
			flags |= WKE_SHIFT;
		if (event.wParam & MK_LBUTTON)
			flags |= WKE_LBUTTON;
		if (event.wParam & MK_MBUTTON)
			flags |= WKE_MBUTTON;
		if (event.wParam & MK_RBUTTON)
			flags |= WKE_RBUTTON;
		wkeFireMouseEvent(m_wkeWebView, WKE_MSG_MOUSEMOVE, pt.x, pt.y, flags);
		updateCursor();
	}
	break;
	case UIEVENT_BUTTONDOWN:
		this->SetFocus();
		wkeFireMouseEvent(m_wkeWebView, WKE_MSG_LBUTTONDOWN, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_BUTTONUP:
		wkeFireMouseEvent(m_wkeWebView, WKE_MSG_LBUTTONUP, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_RBUTTONDOWN:
		this->SetFocus();
		wkeFireMouseEvent(m_wkeWebView, WKE_MSG_RBUTTONDOWN, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_DBLCLICK:
		wkeFireMouseEvent(m_wkeWebView, WKE_MSG_LBUTTONDBLCLK, pt.x, pt.y, event.wKeyState);
		break;
	case UIEVENT_SCROLLWHEEL:
		wkeFireMouseWheelEvent(m_wkeWebView, pt.x, pt.y, event.wParam == SB_LINEUP ? 120 : -120, event.wKeyState);
		break;
	case UIEVENT_KEYDOWN:
		this->SetFocus();
		wkeFireKeyDownEvent(m_wkeWebView, event.chKey, event.lParam, false);
		break;
	case UIEVENT_KEYUP:
		wkeFireKeyUpEvent(m_wkeWebView, event.chKey, event.lParam, false);
		break;
	case UIEVENT_CHAR:
		wkeFireKeyPressEvent(m_wkeWebView, event.chKey, event.lParam, false);
		break;
	case UIEVENT_SETFOCUS:
		this->SetFocus();
		wkeSetFocus(m_wkeWebView);
		this->Invalidate();
		break;
	case UIEVENT_KILLFOCUS:
		wkeKillFocus(m_wkeWebView);
		this->Invalidate();
		break;
	case UIEVENT_SETCURSOR: {
		/*HWND hwnd = this->GetManager()->GetPaintWindow();
		if (wkeFireWindowsMessage(m_wkeWebView, hwnd, WM_SETCURSOR, 0, 0, NULL)) {
			return;
		}*/
		//return;
	}
							break;
	case UIEVENT_CONTEXTMENU:
	{
		unsigned int flags = 0;
		if (event.wParam & MK_CONTROL)
			flags |= WKE_CONTROL;
		if (event.wParam & MK_SHIFT)
			flags |= WKE_SHIFT;
		wkeFireContextMenuEvent(m_wkeWebView, pt.x, pt.y, flags);
		break;
	}
	/*case UIEVENT_TIMER:
		if (event.wParam == EVENT_TICK_TIEMER_ID) {
			Invalidate();
		}
		break;*/

	default: break;
	}
	CControlUI::DoEvent(event);
}

LRESULT DuiLib::CMiniblink::MessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, bool & bHandled)
{
	
	if (uMsg == GetRunJsMessageId()) {
		bHandled = true;
		JS_ARG *arg = (JS_ARG *)wParam;
		if (arg->webView == NULL || arg->webView == (wkeWebView)0xcccccccc) return S_OK;
		jsValue value;
		jsExecState es;

		if (arg->frameId == NULL||arg->frameId== (wkeWebFrameHandle)0xcccccccc) {
			value = ::wkeRunJS(arg->webView, arg->js);
			es = ::wkeGlobalExec(arg->webView);
		}
		else
		{
			value = ::wkeRunJsByFrame(arg->webView, arg->frameId, arg->js, true);
			es = ::wkeGetGlobalExecByFrame(arg->webView,arg->frameId);
		}
		switch (arg->type)
		{
		case JS_RESULT_TYPE::JS_INT: {
			int* ret = (int*)arg->result;
			*ret = jsToInt(es, value);
			break;
		}
		case JS_RESULT_TYPE::JS_BOOL: {
			bool* ret = (bool*)arg->result;
			*ret = jsToBoolean(es, value);
			break;
		}
		case JS_RESULT_TYPE::JS_CHAR:
		{
			char* ret = (char*)arg->result;
			int* maxLength = (int*)arg->param;
			strcpy_s(ret, *maxLength, jsToString(es, value));
			break;
		}
		case JS_RESULT_TYPE::JS_WCHAR:
		{
			WCHAR* ret = (WCHAR*)arg->result;
			StrCpyW(ret, jsToStringW(es, value));
			//strcpy_s(ret, (rsize_t)arg->param, jsToStringW(es, value));
			break;
		}
		case JS_RESULT_TYPE::JS_DOUBLE: {
			double* ret = (double*)arg->result;
			*ret = jsToDouble(es, value);
			break;
		}
		default:
			break;
		}
		return S_OK;
	}
	else if (uMsg == GetActionMessageId()) {
		bHandled = true;
		wkeWebView web = (wkeWebView)wParam;
		MB_ACTION_ITEM *action = (MB_ACTION_ITEM *)lParam;
		switch (action->sender)
		{
		case MB_ACTION_SENDER::KEY:
		{
			MB_ACTION_KEY_DATA *data = (MB_ACTION_KEY_DATA *)action->data;
			if (data->event == MB_ACTION_KEY_EVENT::DOWN) {
				::wkeFireKeyDownEvent(web, data->code, data->flags, data->flags);
			}
			else if (data->event == MB_ACTION_KEY_EVENT::PRESS) {
				::wkeFireKeyPressEvent(web, data->code, data->flags, data->flags);
			}
			else if (data->event == MB_ACTION_KEY_EVENT::UP) {
				::wkeFireKeyUpEvent(web, data->code, data->flags, data->flags);
			}
			break;
		}
		case MB_ACTION_SENDER::MENU:{
			MB_ACTION_MENU_DATA *data = (MB_ACTION_MENU_DATA *)action->data;
			::wkeFireContextMenuEvent(web, data->x, data->y, data->flags);
			break;
		}
		case MB_ACTION_SENDER::MOUSE: {
			MB_ACTION_MOUSE_DATA *data = (MB_ACTION_MOUSE_DATA *)action->data;
			::wkeFireMouseEvent(web, data->message, data->x, data->y, data->flags);
			break;
		}
		case MB_ACTION_SENDER::WHEEL: {
			MB_ACTION_WHEEL_DATA *data = (MB_ACTION_WHEEL_DATA *)action->data;
			::wkeFireMouseWheelEvent(web, data->x, data->y, data->delta, data->flags);
			break;
		}
		default:
			break;
		}
		return S_OK;
	}

	CControlUI *current = GetManager()->FindControl(GetManager()->GetMousePos());
	//
	if (current != this||!current->IsEnabled()) return S_OK;
	if (uMsg == WM_IME_STARTCOMPOSITION) {
		const RECT controlPos = this->GetPos();

		wkeRect caret = wkeGetCaretRect(m_wkeWebView);
		COMPOSITIONFORM COMPOSITIONFORM;
		COMPOSITIONFORM.dwStyle = CFS_POINT | CFS_FORCE_POSITION;
		COMPOSITIONFORM.ptCurrentPos.x = caret.x + controlPos.left;
		COMPOSITIONFORM.ptCurrentPos.y = caret.y + controlPos.top;

		HIMC hIMC = ::ImmGetContext(GetManager()->GetPaintWindow());
		::ImmSetCompositionWindow(hIMC, &COMPOSITIONFORM);
		::ImmReleaseContext(GetManager()->GetPaintWindow(), hIMC);

		bHandled = true;
	}
	else if (uMsg == WM_SETCURSOR) {
		bHandled = true;
	}
	return S_OK;
}

bool DuiLib::CMiniblink::IsEnabled() const
{
	return CControlUI::IsEnabled();
}

void DuiLib::CMiniblink::SetEnabled(bool enable)
{
	CControlUI::SetEnabled(enable);
}

void DuiLib::CMiniblink::SetVisible(bool enable)
{
	CControlUI::SetVisible(enable);
}

void DuiLib::CMiniblink::SetAttribute(LPCTSTR pstrName, LPCTSTR pstrValue)
{
	if (_tcscmp(pstrName, L"url") == 0) {
		this->Goto(pstrValue);
		return;
	}
	CControlUI::SetAttribute(pstrName, pstrValue);
}

void DuiLib::CMiniblink::OnTitleChanged(LPCWSTR title)
{
	//OutputDebugString(title);
}

void DuiLib::CMiniblink::OnUrlChanged(LPCWSTR url, CWebFrame *frame)
{
}

void DuiLib::CMiniblink::OnAlert(LPCWSTR msg)
{
	::MessageBox(GetManager()->GetPaintWindow(), msg, L"确定", MB_OK|MB_ICONINFORMATION);
}

bool DuiLib::CMiniblink::OnConfirm(LPCWSTR msg)
{
	return ::MessageBox(GetManager()->GetPaintWindow(), msg, L"询问", MB_OKCANCEL | MB_ICONQUESTION) == IDOK;
	//return false;
}

bool DuiLib::CMiniblink::OnPrompt(LPCWSTR msg, LPCWSTR defaultResult, LPCWSTR result)
{

	return false;
}

bool DuiLib::CMiniblink::OnNavigation(wkeNavigationType navigationType, LPCWSTR url)
{
	return true;
}

DuiLib::CMiniblink * DuiLib::CMiniblink::OnNewOpen(wkeNavigationType navigationType, LPCWSTR url, const wkeWindowFeatures * windowFeatures)
{
	//wkeNavigationType::
	//this->Goto(url);
	return this;
}

void DuiLib::CMiniblink::OnDocumentReady(CWebFrame * frame)
{
		
}

bool DuiLib::CMiniblink::OnDownload(const char * url)
{
	return false;
}

bool DuiLib::CMiniblink::OnResponse(const char * url, CNetJob * job)
{
	return false;
}

void DuiLib::CMiniblink::OnConsole(wkeConsoleLevel level, LPCWSTR message, LPCWSTR sourceName, unsigned sourceLine, LPCWSTR stackTrace)
{
}

bool DuiLib::CMiniblink::OnRequestBegin(const char * url, CNetJob * job)
{
	return false;
}

void DuiLib::CMiniblink::OnRequestEnd(const char * url, CNetJob * job, void * buf, int len)
{
}

void DuiLib::CMiniblink::OnCreateScriptContext(CWebFrame * frame, void * context, int extensionGroup, int worldId)
{
}

void DuiLib::CMiniblink::OnReleaseScriptContext(CWebFrame * frame, void * context, int worldId)
{
}

void DuiLib::CMiniblink::OnMediaLoad(const char * url, wkeMediaLoadInfo * info)
{
}
void DuiLib::CMiniblink::OnLoadingFinish(LPCWSTR url, wkeLoadingResult result, LPCWSTR failedReason)
{
}

bool DuiLib::CMiniblink::IsVisible() const
{
	return CControlUI::IsVisible();
}

unsigned DuiLib::CMiniblink::GetVersion() {
	return wkeGetVersion();
}
wkeWebView DuiLib::CMiniblink::GetWebView() {
	return m_wkeWebView;
}
bool DuiLib::CMiniblink::IsDocumentReady() {
	return ::wkeIsDocumentReady(m_wkeWebView);
}
void DuiLib::CMiniblink::Reload() {
	::wkeReload(m_wkeWebView);
}
LPCWSTR DuiLib::CMiniblink::GetTitle() {
#ifdef UNICODE
	return ::wkeGetTitleW(m_wkeWebView);
#else
	return ::wkeGetTitle(m_wkeWebView);
#endif // UNICODE
}
int DuiLib::CMiniblink::GetWidth() {
	return ::wkeGetWidth(m_wkeWebView);
}
int DuiLib::CMiniblink::GetHeight() {
	return ::wkeGetHeight(m_wkeWebView);
}
int DuiLib::CMiniblink::GetContentWidth() {
	return ::wkeGetContentWidth(m_wkeWebView);
}
int DuiLib::CMiniblink::GetContentHeight() {
	return ::wkeGetContentHeight(m_wkeWebView);
}
bool DuiLib::CMiniblink::CanGoBack() {
	return ::wkeCanGoBack(m_wkeWebView);
}
bool DuiLib::CMiniblink::GoBack() {
	if (this->CanGoBack()) {
		return ::wkeGoBack(m_wkeWebView);
	}
	return false;
}
bool DuiLib::CMiniblink::CanGoForward() {
	return ::wkeCanGoForward(m_wkeWebView);
}
bool DuiLib::CMiniblink::GoForward() {
	if (this->CanGoForward()) {
		return ::wkeGoForward(m_wkeWebView);
	}
	return false;
}
void DuiLib::CMiniblink::EditorSelectAll() {
	::wkeEditorSelectAll(m_wkeWebView);
}
void DuiLib::CMiniblink::EditorUnSelect() {
	::wkeEditorUnSelect(m_wkeWebView);
}
void DuiLib::CMiniblink::EditorCopy() {
	::wkeEditorCopy(m_wkeWebView);
}
void DuiLib::CMiniblink::EditorCut() {
	::wkeEditorCut(m_wkeWebView);
}
void DuiLib::CMiniblink::EditorDelete() {
	::wkeEditorDelete(m_wkeWebView);
}
void DuiLib::CMiniblink::EditorUndo() {
	::wkeEditorUndo(m_wkeWebView);
}
void DuiLib::CMiniblink::EditorRedo() {
	::wkeEditorRedo(m_wkeWebView);
}
LPCWSTR DuiLib::CMiniblink::GetCookie() {
#ifdef UNICODE
	return ::wkeGetCookieW(m_wkeWebView);
#else
	return ::wkeGetCookie(m_wkeWebView);
#endif // UNICODE
}
void DuiLib::CMiniblink::SetCookie(const char *cookie, const char *url) {
	const char *_url = url;
	if (_url == NULL) {
		_url = GetUrl();
	}
	::wkeSetCookie(m_wkeWebView, _url, cookie);
}
const char *DuiLib::CMiniblink::GetUrl() const {
	return ::wkeGetURL(m_wkeWebView);
}
void DuiLib::CMiniblink::SetCookieEnabled(bool enable) {
	::wkeSetCookieEnabled(m_wkeWebView, enable);
}
bool DuiLib::CMiniblink::IsCookieEnabled() {
	return ::wkeIsCookieEnabled(m_wkeWebView);
}
void DuiLib::CMiniblink::SetCookieFile(LPCWSTR file) {
	::wkeSetCookieJarFullPath(m_wkeWebView, file);
}
void DuiLib::CMiniblink::SetLocalStoragePath(LPCWSTR path) {
	::wkeSetLocalStorageFullPath(m_wkeWebView, path);
}
void DuiLib::CMiniblink::SetZoom(float factor) {
	::wkeSetZoomFactor(m_wkeWebView, factor);
}
float DuiLib::CMiniblink::GetZoom() {
	return ::wkeGetZoomFactor(m_wkeWebView);
}
void DuiLib::CMiniblink::SetProxy(const wkeProxy* proxy) {
	::wkeSetViewProxy(m_wkeWebView, const_cast<wkeProxy*>(proxy));
}
void DuiLib::CMiniblink::SetMemoryCacheEnable(bool enable) {
	::wkeSetMemoryCacheEnable(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetTouchEnabled(bool enable) {
	::wkeSetTouchEnabled(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetMouseEnabled(bool enable) {
	::wkeSetMouseEnabled(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetNavigationToNewWindowEnable(bool enable) {
	::wkeSetNavigationToNewWindowEnable(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetCspCheckEnable(bool enable) {
	::wkeSetCspCheckEnable(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetNpapiPluginsEnabled(bool enable) {
	::wkeSetNpapiPluginsEnabled(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetHeadlessEnabled(bool enable) {
	::wkeSetHeadlessEnabled(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetDebugConfig(const char* key, const char* value) {
	::wkeSetDebugConfig(m_wkeWebView, key, key);
}
void DuiLib::CMiniblink::SetTransparent(bool enable) {
	::wkeSetTransparent(m_wkeWebView, enable);
}
void DuiLib::CMiniblink::SetUserAgent(LPCWSTR userAgent) {
#ifdef UNICODE
	return ::wkeSetUserAgentW(m_wkeWebView, userAgent);
#else
	return ::wkeSetUserAgent(m_wkeWebView, userAgent);
#endif // UNICODE
}

const char * DuiLib::CMiniblink::GetUserAgent() const
{
	return ::wkeGetUserAgent(m_wkeWebView);
}

void DuiLib::CMiniblink::SetHtml(const char * html, const char * baseUrl)
{
	if (baseUrl == NULL) {
		::wkeLoadHTML(m_wkeWebView, html);
	}
	else {
		::wkeLoadHtmlWithBaseUrl(m_wkeWebView, html, baseUrl);
	}
}

void DuiLib::CMiniblink::RunJs(const char* js)
{
	JS_ARG arg;
	arg.frameId = NULL;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_UNDEFINED;
	arg.webView = m_wkeWebView;
	::SendMessage(GetManager()->GetPaintWindow(), GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CMiniblink::RunJs(const char* js, int * result)
{
	JS_ARG arg;
	arg.frameId = NULL;
	arg.js = js;
	arg.result = result;
	arg.type = JS_RESULT_TYPE::JS_INT;
	arg.webView = m_wkeWebView;
	::SendMessage(GetManager()->GetPaintWindow(), GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CMiniblink::RunJs(const char* js, double * result)
{
	JS_ARG arg;
	arg.frameId = NULL;
	arg.js = js;
	arg.result = result;
	arg.type = JS_RESULT_TYPE::JS_DOUBLE;
	arg.webView = m_wkeWebView;
	::SendMessage(GetManager()->GetPaintWindow(), GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CMiniblink::RunJs(const char* js, bool * result)
{
	JS_ARG arg;
	arg.frameId = NULL;
	arg.js = js;
	arg.result = result;
	arg.type = JS_RESULT_TYPE::JS_BOOL;
	arg.webView = m_wkeWebView;
	::SendMessage(GetManager()->GetPaintWindow(), GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CMiniblink::RunJs(const char* js, char * result, int len)
{
	JS_ARG arg;
	arg.frameId = NULL;
	arg.js = js;
	arg.result = result;
	arg.type = JS_RESULT_TYPE::JS_CHAR;
	arg.param = &len;
	arg.webView = m_wkeWebView;
	::SendMessage(GetManager()->GetPaintWindow(), GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CMiniblink::RunJs(const char* js, wchar_t * result, int len)
{
	JS_ARG arg;
	arg.frameId = NULL;
	arg.js = js;
	arg.result = result;
	arg.type = JS_RESULT_TYPE::JS_WCHAR;
	arg.param = &len;
	arg.webView = m_wkeWebView;
	::SendMessage(GetManager()->GetPaintWindow(), GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CMiniblink::SetSize(int width, int height)
{
	if (!this->IsVisible()) {
		::wkeResize(m_wkeWebView, width, height);
	}
}

void DuiLib::CMiniblink::SetDeviceParameter(const char * device, const char * paramStr, int paramInt, float paramFloat)
{
	::wkeSetDeviceParameter(m_wkeWebView, device, paramStr, paramInt, paramFloat);
}

void DuiLib::CMiniblink::SetSettings(const wkeViewSettings * settings)
{
	::wkeSetViewSettings(m_wkeWebView, settings);
}

void DuiLib::CMiniblink::SimulationAction(MB_ACTION_ITEM * action)
{
	if (action == NULL || action->data == NULL) return;
	if (action->async) {
		::PostMessage(GetManager()->GetPaintWindow(), GetActionMessageId(), (WPARAM)m_wkeWebView, (LPARAM)action);
	}
	else
	{
		::SendMessage(GetManager()->GetPaintWindow(), GetActionMessageId(), (WPARAM)m_wkeWebView, (LPARAM)action);
	}
	
}

void DuiLib::CMiniblink::FlushCookie()
{
	::wkePerformCookieCommand(m_wkeWebView, wkeCookieCommand::wkeCookieCommandFlushCookiesToFile);
}

/*静态函数*/
void DuiLib::CMiniblink::Initialize()
{
	static bool isInitialized = ::wkeIsInitialize==NULL?false:(::wkeIsInitialize());
	if (!isInitialized) {
		::wkeInitialize();
		isInitialized = true;
	}

}

void DuiLib::CMiniblink::SetWkeDllPath(LPCTSTR dllPath)
{
	Initialize();
	::wkeSetWkeDllPath(dllPath);
}
void DuiLib::CMiniblink::SetGlobalProxy(const wkeProxy* proxy) {
	Initialize();
	::wkeSetProxy(proxy);
}

UINT DuiLib::CMiniblink::GetRunJsMessageId()
{
	static UINT id = ::RegisterWindowMessage(L"MB_JS_EXECUTE");
	return id;
}

UINT DuiLib::CMiniblink::GetActionMessageId()
{
	static UINT id = ::RegisterWindowMessage(L"MB_ACTION_EXECUTE");
	return id;
}

/*回调函数*/
void DuiLib::CMiniblink::MBPaintUpdate(wkeWebView webView, void* param, const HDC hdc, int x, int y, int cx, int cy) {
	CMiniblink *cmb = (CMiniblink *)param;
	cmb->Invalidate();
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBTitleChanged(wkeWebView webView, void * param, const wkeString title)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
#ifdef UNICODE
	cmb->OnTitleChanged(::wkeGetStringW(title));
#else
	cmb->OnTitleChanged(::wkeGetString(title));
#endif // UNICODE	

}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBUrlChanged(wkeWebView webView, void * param, wkeWebFrameHandle frameId, const wkeString url)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
	CWebFrame frame(frameId, cmb->GetWebView(),cmb->GetManager()->GetPaintWindow());
#ifdef UNICODE
	cmb->OnUrlChanged(::wkeGetStringW(url), &frame);
#else
	cmb->OnUrlChanged(::wkeGetString(url), &frame);
#endif // UNICODE	
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBAlert(wkeWebView webView, void * param, const wkeString msg)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
#ifdef UNICODE
	cmb->OnAlert(::wkeGetStringW(msg));
#else
	cmb->OnAlert(::wkeGetString(msg));
#endif // UNICODE	
}

bool WKE_CALL_TYPE DuiLib::CMiniblink::MBConfirm(wkeWebView webView, void * param, const wkeString msg)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return false;
#ifdef UNICODE
	return cmb->OnConfirm(::wkeGetStringW(msg));
#else
	return cmb->OnConfirm(::wkeGetString(msg));
#endif // UNICODE	
}

bool WKE_CALL_TYPE DuiLib::CMiniblink::MBPromptBox(wkeWebView webView, void * param, const wkeString msg, const wkeString defaultResult, wkeString result)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return false;
#ifdef UNICODE
	return cmb->OnPrompt(::wkeGetStringW(msg), ::wkeGetStringW(defaultResult), ::wkeGetStringW(result));
#else
	return cmb->OnPrompt(::wkeGetString(msg), ::wkeGetString(defaultResult), ::wkeGetString(result));
#endif // UNICODE
}

bool WKE_CALL_TYPE DuiLib::CMiniblink::MBNavigation(wkeWebView webView, void * param, wkeNavigationType navigationType, const wkeString url)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return false;
#ifdef UNICODE
	return cmb->OnNavigation(navigationType, ::wkeGetStringW(url));
#else
	return cmb->OnNavigation(navigationType, ::wkeGetString(url));
#endif // UNICODE
}

wkeWebView WKE_CALL_TYPE DuiLib::CMiniblink::MBCreateView(wkeWebView webView, void * param, wkeNavigationType navigationType, const wkeString url, const wkeWindowFeatures * windowFeatures)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return NULL;
#ifdef UNICODE
	return cmb->OnNewOpen(navigationType, ::wkeGetStringW(url), windowFeatures)->GetWebView();
#else
	return cmb->OnNewOpen(navigationType, ::wkeGetString(url), windowFeatures)->GetWebView();
#endif // UNICODE
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBDocumentReady(wkeWebView webView, void * param, wkeWebFrameHandle frameId)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
	CWebFrame frame(frameId, cmb->GetWebView(), cmb->GetManager()->GetPaintWindow());
	return cmb->OnDocumentReady(&frame);
}

bool WKE_CALL_TYPE DuiLib::CMiniblink::MBDownload(wkeWebView webView, void * param, const char * url)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return false;
	return cmb->OnDownload(url);
}

bool WKE_CALL_TYPE DuiLib::CMiniblink::MBNetResponse(wkeWebView webView, void * param, const char * url, wkeNetJob job)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return false;
	CNetJob _job(job);
	return cmb->OnResponse(url, &_job);
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBConsole(wkeWebView webView, void * param, wkeConsoleLevel level, const wkeString message, const wkeString sourceName, unsigned sourceLine, const wkeString stackTrace)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
#ifdef UNICODE
	return cmb->OnConsole(level, ::wkeGetStringW(message), ::wkeGetStringW(sourceName), sourceLine, ::wkeGetStringW(stackTrace));
#else
	return cmb->OnConsole(level, ::wkeGetString(message), ::wkeGetString(sourceName), sourceLine, ::wkeGetString(stackTrace));
#endif // UNICODE
}

bool WKE_CALL_TYPE DuiLib::CMiniblink::MBLoadUrlBegin(wkeWebView webView, void * param, const char * url, wkeNetJob job)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return false;
	CNetJob _job(job);
	return cmb->OnRequestBegin(url, &_job);
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBLoadUrlEnd(wkeWebView webView, void * param, const char * url, wkeNetJob job, void * buf, int len)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
	CNetJob _job(job);
	return cmb->OnRequestEnd(url, &_job, buf, len);
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBCreateScriptContext(wkeWebView webView, void * param, wkeWebFrameHandle frameId, void * context, int extensionGroup, int worldId)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
	CWebFrame frame(frameId, cmb->GetWebView(), cmb->GetManager()->GetPaintWindow());
	return cmb->OnCreateScriptContext(&frame, context, extensionGroup, worldId);
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBReleaseScriptContext(wkeWebView webView, void * param, wkeWebFrameHandle frameId, void * context, int worldId)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
	CWebFrame frame(frameId, cmb->GetWebView(), cmb->GetManager()->GetPaintWindow());
	return cmb->OnReleaseScriptContext(&frame, context, worldId);
}

void WKE_CALL_TYPE DuiLib::CMiniblink::MBMediaLoad(wkeWebView webView, void * param, const char * url, wkeMediaLoadInfo * info)

{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
	return cmb->OnMediaLoad(url, info);
}
void WKE_CALL_TYPE DuiLib::CMiniblink::MBLoadedFinish(wkeWebView webView, void* param, const wkeString url, wkeLoadingResult result, const wkeString failedReason)
{
	CMiniblink *cmb = (CMiniblink *)param;
	if (cmb == NULL || cmb->m_released) return;
#ifdef UNICODE
	return cmb->OnLoadingFinish(::wkeGetStringW(url), result, ::wkeGetStringW(failedReason));
#else
	return cmb->OnLoadingFinish(::wkeGetString(url), result, ::wkeGetString(failedReason));
#endif // UNICODE

}
void DuiLib::CMiniblink::updateCursor()
{
	int cursorInfo = wkeGetCursorInfoType(m_wkeWebView);
	if (m_cursor != cursorInfo) {
		m_cursor = cursorInfo;
		HCURSOR curosr = ::LoadCursor(NULL, IDC_ARROW);
		switch (cursorInfo)
		{
		case WkeCursorInfoPointer:
			curosr = ::LoadCursor(NULL, IDC_ARROW);
			break;
		case WkeCursorInfoCross:
			curosr = ::LoadCursor(NULL, IDC_CROSS);
			break;
		case WkeCursorInfoHand:
			curosr = ::LoadCursor(NULL, IDC_HAND);
			break;
		case WkeCursorInfoIBeam:
			curosr = ::LoadCursor(NULL, IDC_IBEAM);
			break;
		case WkeCursorInfoWait:
			curosr = ::LoadCursor(NULL, IDC_WAIT);
			break;
		case WkeCursorInfoHelp:
			curosr = ::LoadCursor(NULL, IDC_HELP);
			break;
		case WkeCursorInfoEastResize:
			curosr = ::LoadCursor(NULL, IDC_SIZEWE);
			break;
		case WkeCursorInfoNorthResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENS);
			break;
		case WkeCursorInfoNorthEastResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENESW);
			break;
		case WkeCursorInfoNorthWestResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENWSE);
			break;
		case WkeCursorInfoSouthResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENS);
			break;
		case WkeCursorInfoSouthEastResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENWSE);
			break;
		case WkeCursorInfoSouthWestResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENESW);
			break;
		case WkeCursorInfoWestResize:
			curosr = ::LoadCursor(NULL, IDC_SIZEWE);
			break;
		case WkeCursorInfoNorthSouthResize:
			curosr = ::LoadCursor(NULL, IDC_SIZENS);
			break;
		case WkeCursorInfoEastWestResize:
			curosr = ::LoadCursor(NULL, IDC_SIZEWE);
			break;
		case WkeCursorInfoNorthEastSouthWestResize:
			curosr = ::LoadCursor(NULL, IDC_SIZEALL);
			break;
		case WkeCursorInfoNorthWestSouthEastResize:
			curosr = ::LoadCursor(NULL, IDC_SIZEALL);
			break;
		case WkeCursorInfoColumnResize:
		case WkeCursorInfoRowResize:
			curosr = ::LoadCursor(NULL, IDC_ARROW);
			break;
		case WkeCursorInfoMove:
			curosr = ::LoadCursor(NULL, IDC_SIZEALL);
			break;
		case WkeCursorInfoNotAllowed:
			curosr = ::LoadCursor(NULL, IDC_NO);
			break;
		case WkeCursorInfoGrab:
			curosr = ::LoadCursor(NULL, IDC_HAND);
			break;
		default:
			break;
		}
		::SetCursor(curosr);
	}
}

DuiLib::CWebFrame::CWebFrame(wkeWebFrameHandle frameId, wkeWebView webview,HWND hwnd) :m_webFrame(frameId), m_web(webview),m_hwnd(hwnd)
{
}

bool DuiLib::CWebFrame::IsMainFrame()
{
	return ::wkeIsMainFrame(m_web, m_webFrame);
}

const char * DuiLib::CWebFrame::GetUrl() const
{
	return ::wkeGetFrameUrl(m_web, m_webFrame);
}

void DuiLib::CWebFrame::RunJs(const char * js)
{
	JS_ARG arg;
	arg.frameId = m_webFrame;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_UNDEFINED;
	arg.webView = m_web;
	::SendMessage(m_hwnd, CMiniblink::GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CWebFrame::RunJs(const char * js, int * result)
{
	JS_ARG arg;
	arg.frameId = m_webFrame;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_INT;
	arg.webView = m_web;
	::SendMessage(m_hwnd, CMiniblink::GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CWebFrame::RunJs(const char * js, double * result)
{
	JS_ARG arg;
	arg.frameId = m_webFrame;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_DOUBLE;
	arg.webView = m_web;
	::SendMessage(m_hwnd, CMiniblink::GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CWebFrame::RunJs(const char * js, bool * result)
{
	JS_ARG arg;
	arg.frameId = m_webFrame;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_BOOL;
	arg.webView = m_web;
	::SendMessage(m_hwnd, CMiniblink::GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CWebFrame::RunJs(const char * js, char * result, int len)
{
	JS_ARG arg;
	arg.frameId = m_webFrame;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_CHAR;
	arg.webView = m_web;
	arg.param = &len;
	::SendMessage(m_hwnd, CMiniblink::GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CWebFrame::RunJs(const char * js, wchar_t * result, int len)
{
	JS_ARG arg;
	arg.frameId = m_webFrame;
	arg.js = js;
	arg.type = JS_RESULT_TYPE::JS_WCHAR;
	arg.webView = m_web;
	arg.param = &len;
	::SendMessage(m_hwnd, CMiniblink::GetRunJsMessageId(), (WPARAM)&arg, NULL);
}

void DuiLib::CNetJob::HookRequest()
{
	::wkeNetHookRequest(m_job);
}
