
void InsertLogMsg(LPTSTR text) { if(text) { mir3::ConsoleLog(ServiceName, text); } }
void InsertLogMsg(UINT id) { wchar_t text[1024]={}; if(LoadString(g_hInst,id,text,_countof(text))) InsertLogMsg(text); }
#ifdef LOGPARAM_STR
void InsertLogMsgParam(UINT id,void* parameter,BYTE flags) {
    wchar_t format[256]={},text[1024]={};
    if(!LoadString(g_hInst,id,format,_countof(format))) return;
    if(flags==LOGPARAM_STR) _snwprintf_s(text,_countof(text),_TRUNCATE,format,static_cast<LPTSTR>(parameter));
    else if(flags==LOGPARAM_INT) _snwprintf_s(text,_countof(text),_TRUNCATE,format,*static_cast<int*>(parameter));
    InsertLogMsg(text);
}
#endif
void ReportServiceState(BOOL running) { InsertLogMsg(running?IDS_STARTSERVICE:IDS_STOPSERVICE); }
