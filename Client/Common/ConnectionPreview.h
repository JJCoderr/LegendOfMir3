#pragma once
struct ConnectionPreview {
    enum {First=262,Selection=263,ChecksOnly=264,Last=ChecksOnly};
    static inline bool arm=false;
    static inline SOCKET listener=INVALID_SOCKET;
    static SOCKET Listen(WORD& port) {
        SOCKET socketHandle=socket(AF_INET,SOCK_STREAM,0);sockaddr_in address={};address.sin_family=AF_INET;
        address.sin_addr.s_addr=htonl(INADDR_LOOPBACK);
        if(bind(socketHandle,(sockaddr*)&address,sizeof(address))!=0 || listen(socketHandle,1)!=0) {closesocket(socketHandle);return INVALID_SOCKET;}
        int size=sizeof(address);getsockname(socketHandle,(sockaddr*)&address,&size);port=ntohs(address.sin_port);
        u_long nonblocking=1;ioctlsocket(socketHandle,FIONBIO,&nonblocking);return socketHandle;
    }
    static void DispatchUntil(bool disconnected) {
        const DWORD started=GetTickCount();
        while(GetTickCount()-started<5000) {
            MSG message={};
            if(PeekMessage(&message,g_xMainWnd.GetSafehWnd(),ID_SOCKCLIENT_EVENT_MSG,ID_SOCKCLIENT_EVENT_MSG,PM_REMOVE)) {
                g_xClientSocket.OnSocketMessage(message.wParam,message.lParam);
                if(disconnected?g_xClientSocket.m_sockClient==INVALID_SOCKET:WSAGETSELECTEVENT(message.lParam)==FD_CONNECT) return;
            } else Sleep(1);
        }
    }
    static void Checks() {
        struct Probe : CWHDefProcess {int opened=0;void OnConnectToServer() override {++opened;}char* OnMessageReceive(CHAR*) override {return nullptr;}} probe;
        auto& client=g_xClientSocket;auto* owner=client.m_pxDefProc;const HWND window=g_xMainWnd.GetSafehWnd();
        FILE* report=fopen("connection-retry-check.txt","wb");if(!report) return;
        const auto check=[&](const char* name,bool ok) {fprintf(report,"%s=%d\n",name,ok);};
        ConnectionRetry clock;clock.Fail();check("failure_requests_prompt",clock.phase==ConnectionRetry::Phase::Prompt);
        clock.Answer(true,100);check("wait_1999",!clock.Due(2099));check("retry_2000",clock.Due(2100));
        clock.Answer(true,0xfffffff0u);check("wrapped_clock",!clock.Due(1983) && clock.Due(1984));
        clock.Answer(false,0);check("no_requests_quit",clock.phase==ConnectionRetry::Phase::Quit);
        WORD port=0;SOCKET reserved=Listen(port);check("loopback_listener",reserved!=INVALID_SOCKET && port!=0);
        closesocket(reserved);client.m_pxDefProc=&probe;
        client.ConnectToServer(window,const_cast<char*>("127.0.0.1"),port);DispatchUntil(true);
        check("refused_requests_prompt",client.m_retry.phase==ConnectionRetry::Phase::Prompt && client.m_sockClient==INVALID_SOCKET && probe.opened==0);
        WORD retryPort=0;SOCKET endpoint=Listen(retryPort);client.m_connectionPort=retryPort;
        client.AnswerConnection(true,1000);check("waiting_disables_input",!IsWindowEnabled(window));
        client.PollConnection(2999);check("before_delay_no_socket",client.m_sockClient==INVALID_SOCKET);
        client.PollConnection(3000);check("due_opens_socket",client.m_sockClient!=INVALID_SOCKET && IsWindowEnabled(window));
        DispatchUntil(false);check("retry_calls_page",probe.opened==1 && client.m_retry.phase==ConnectionRetry::Phase::Idle);
        check("same_endpoint",client.m_connectionHost=="127.0.0.1" && client.m_connectionPort==retryPort);
        SOCKET peer=accept(endpoint,nullptr,nullptr);check("retry_accepts",peer!=INVALID_SOCKET);
        const SOCKET active=client.m_sockClient;
        client.OnSocketMessage(INVALID_SOCKET,WSAMAKESELECTREPLY(FD_CLOSE,WSAECONNRESET));
        check("stale_error_ignored",client.m_sockClient==active && !client.m_retry.Blocks());
        closesocket(peer);DispatchUntil(true);
        check("graceful_close_no_retry",client.m_sockClient==INVALID_SOCKET && !client.m_retry.Blocks());
        client.m_sockClient=socket(AF_INET,SOCK_STREAM,0);
        client.OnSocketMessage(client.m_sockClient,WSAMAKESELECTREPLY(FD_READ,WSAECONNRESET));
        check("read_error_prompts",client.m_sockClient==INVALID_SOCKET && client.m_retry.phase==ConnectionRetry::Phase::Prompt);
        client.AnswerConnection(false,4000);MSG quit={};
        check("no_posts_close",PeekMessage(&quit,window,WM_CLOSE,WM_CLOSE,PM_REMOVE)!=FALSE);
        client.DisconnectToServer();client.m_retry.Fail();client.AnswerConnection(true,5000);client.DisconnectToServer();
        check("manual_disconnect_cancels",IsWindowEnabled(window) && !client.m_retry.Blocks());
        check("invalid_socket_event_ignored",!client.OnSocketMessage(INVALID_SOCKET,WSAMAKESELECTREPLY(FD_CONNECT,WSAECONNREFUSED)) && !client.m_retry.Blocks());
        CLoginMessages captions;captions.Load("./CmList.dat");check("localized_prompt",!captions.Get(ConnectionRetry::Message).empty());
        closesocket(endpoint);client.m_pxDefProc=owner;fclose(report);
    }
    static const char* Load(int mode) {
        if(mode==Selection) {LoginVisual::ResizeCanvas(LoginLayout::Width,LoginLayout::Height);g_xChrSelProc.Preview("select");}
        else g_xLoginProc.Load(false);
        if(mode==ChecksOnly) Checks();
        else {
            WORD port=0;listener=Listen(port);
            g_xClientSocket.m_connectionWindow=g_xMainWnd.GetSafehWnd();
            g_xClientSocket.m_connectionHost="127.0.0.1";g_xClientSocket.m_connectionPort=port;
            g_xClientSocket.m_pxDefProc=mode==Selection?static_cast<CWHDefProcess*>(&g_xChrSelProc):static_cast<CWHDefProcess*>(&g_xLoginProc);
            arm=true;
        }
        return mode==Selection?"selection-connection-background.bmp":"login-connection-background.bmp";
    }
    static void AfterFrame() {
        if(!arm) return;arm=false;
        g_xClientSocket.m_sockClient=socket(AF_INET,SOCK_STREAM,0);
        g_xClientSocket.OnSocketMessage(g_xClientSocket.m_sockClient,WSAMAKESELECTREPLY(FD_CONNECT,WSAECONNREFUSED));
    }
};
