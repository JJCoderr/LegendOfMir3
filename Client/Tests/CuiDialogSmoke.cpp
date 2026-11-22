#include "Dialogs.h"
#include "LoginClient.h"
#include "CuiTestPlan.h"

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace {
using namespace mir3_client;
namespace fs = std::filesystem;

void Check(bool passed, const char* detail)
{
    if (!passed) throw std::runtime_error(detail);
}

template<typename Action>
void Case(const char* name, Action action)
{
    std::cout << "[RUN]  " << name << std::endl;
    action();
    std::cout << "[PASS] " << name << '\n';
}

void Field(std::string& value, std::size_t offset, std::uint32_t number, unsigned size = 4)
{
    for (unsigned i = 0; i < size; ++i) value[offset+i] = static_cast<char>(number >> (8*i));
}

std::string Encode(std::string_view value)
{
    std::string bytes((value.size()*4+2)/3+1, '\0');
    fnEncode6BitBuf(reinterpret_cast<unsigned char*>(const_cast<char*>(value.data())), bytes.data(),
        static_cast<int>(value.size()), static_cast<int>(bytes.size()));
    bytes.pop_back();
    return bytes;
}

std::string Packet(unsigned opcode, std::uint32_t actor = 0, WORD first = 0, WORD second = 0,
                   WORD third = 0, std::string_view body = {})
{
    std::string header(12,'\0');
    Field(header,0,actor);
    Field(header,4,opcode,2);
    Field(header,6,first,2);
    Field(header,8,second,2);
    Field(header,10,third,2);
    return "#" + Encode(header) + Encode(body) + '!';
}

std::string ScenePacket(unsigned opcode, std::uint32_t actor, WORD first, WORD second, WORD third,
                        std::string_view wire)
{
    std::string header(12,'\0');
    header[0] = static_cast<char>(opcode);
    header[4] = static_cast<char>(opcode >> 8);
    header[5] = static_cast<char>(actor);
    header[7] = static_cast<char>(actor >> 8);
    header[9] = static_cast<char>(actor >> 16);
    header[11] = static_cast<char>(actor >> 24);
    header[1] = static_cast<char>(first);
    header[6] = static_cast<char>(first >> 8);
    header[2] = static_cast<char>(second);
    header[8] = static_cast<char>(second >> 8);
    header[3] = static_cast<char>(third);
    header[10] = static_cast<char>(third >> 8);
    return "#" + Encode(header) + std::string(wire) + '!';
}

class GatewayFixture {
    SOCKET listener_ = INVALID_SOCKET, peer_ = INVALID_SOCKET;
    std::string pending_;
    unsigned port_ = 0, sequence_ = 0;

    static void Nonblocking(SOCKET socket)
    {
        u_long setting=1;
        Check(ioctlsocket(socket,FIONBIO,&setting)==0, "set nonblocking socket");
    }

public:
    ~GatewayFixture()
    {
        if (peer_ != INVALID_SOCKET) closesocket(peer_);
        if (listener_ != INVALID_SOCKET) closesocket(listener_);
    }
    GatewayFixture() = default;
    GatewayFixture(const GatewayFixture&) = delete;
    GatewayFixture& operator=(const GatewayFixture&) = delete;

    unsigned Listen()
    {
        listener_ = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        Check(listener_ != INVALID_SOCKET,"create loopback socket");
        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        int length = sizeof address;
        Check(bind(listener_,reinterpret_cast<sockaddr*>(&address),length)==0 &&
            listen(listener_,1)==0 && getsockname(listener_,reinterpret_cast<sockaddr*>(&address),&length)==0,
            "listen on loopback");
        Nonblocking(listener_);
        port_ = ntohs(address.sin_port);
        return port_;
    }
    std::string Address() const { return "127.0.0.1/" + std::to_string(port_); }

    void Accept(LoginClient& client)
    {
        if (peer_ != INVALID_SOCKET) closesocket(peer_);
        sequence_ = 0;
        pending_.clear();
        const auto deadline = GetTickCount64()+5000;
        do {
            client.Poll();
            peer_ = accept(listener_,nullptr,nullptr);
            if (peer_ != INVALID_SOCKET) {
                Nonblocking(peer_);
                return;
            }
            Check(WSAGetLastError()==WSAEWOULDBLOCK,"accept loopback connection");
            Sleep(1);
        } while (GetTickCount64()<deadline);
        Check(false,"client did not connect in time");
    }

    void Send(std::string_view data)
    {
        const auto deadline = GetTickCount64()+5000;
        while (!data.empty() && GetTickCount64()<deadline) {
            const int n = send(peer_,data.data(),static_cast<int>(data.size()),0);
            if (n>0) data.remove_prefix(n);
            else {
                Check(n==SOCKET_ERROR && WSAGetLastError()==WSAEWOULDBLOCK,"send fixture packet");
                Sleep(1);
            }
        }
        Check(data.empty(),"fixture send timed out");
    }

    void ExpectBytes(LoginClient& client, std::string_view expected)
    {
        const auto deadline = GetTickCount64()+5000;
        while (pending_.size()<expected.size() && GetTickCount64()<deadline) {
            client.Poll();
            char bytes[4096];
            const int n = recv(peer_,bytes,sizeof bytes,0);
            if (n>0) pending_.append(bytes,n);
            else if (n==0) break;
            else Check(WSAGetLastError()==WSAEWOULDBLOCK,"receive client packet");
            Sleep(1);
        }
        if (std::string_view(pending_).substr(0,expected.size()) != expected)
            throw std::runtime_error("wire mismatch, expected "+std::string(expected)+", got "+pending_);
        pending_.erase(0,expected.size());
    }

    void Expect(LoginClient& client, std::string packet)
    {
        sequence_ = sequence_%9+1;
        packet.insert(1,1,static_cast<char>('0'+sequence_));
        ExpectBytes(client,packet);
    }

    void Reply(LoginClient& client, std::string packet)
    {
        Send(packet+'*');
        ExpectBytes(client,"*");
    }

    void Game(LoginClient& client, unsigned opcode, std::uint32_t actor = 0, WORD first = 0,
              WORD second = 0, WORD third = 0, std::string_view body = {}, bool alreadyEncoded = false)
    {
        const auto wire = alreadyEncoded ? std::string(body) : Encode(body);
        Reply(client,ScenePacket(opcode,actor,first,second,third,wire));
    }
};

class Preview {
    HWND window_ = nullptr;
    BITMAPINFO bitmap_{};
    const unsigned char* pixels_ = nullptr;
    POINT pointer_{-1,-1};
    bool advance_ = false;

    static LRESULT CALLBACK Handle(HWND window, UINT message, WPARAM key, LPARAM data)
    {
        if (message==WM_NCCREATE) {
            const auto* create = reinterpret_cast<const CREATESTRUCTW*>(data);
            SetWindowLongPtrW(window,GWLP_USERDATA,reinterpret_cast<LONG_PTR>(create->lpCreateParams));
        }
        auto* preview = reinterpret_cast<Preview*>(GetWindowLongPtrW(window,GWLP_USERDATA));
        if (!preview) return DefWindowProcW(window,message,key,data);
        switch (message) {
        case WM_PAINT: {
            PAINTSTRUCT paint{};
            const HDC dc = BeginPaint(window,&paint);
            const auto& info = preview->bitmap_.bmiHeader;
            if (preview->pixels_)
                SetDIBitsToDevice(dc,0,0,info.biWidth,-info.biHeight,0,0,0,-info.biHeight,
                    preview->pixels_,&preview->bitmap_,DIB_RGB_COLORS);
            const auto [x,y]=preview->pointer_;
            if (x>=0) {
                const auto pen=CreatePen(PS_SOLID,2,RGB(255,196,0));
                const auto oldPen=SelectObject(dc,pen);
                const auto oldBrush=SelectObject(dc,GetStockObject(HOLLOW_BRUSH));
                Ellipse(dc,x-8,y-8,x+9,y+9);
                SelectObject(dc,oldBrush);
                SelectObject(dc,oldPen);
                DeleteObject(pen);
            }
            EndPaint(window,&paint);
            return 0;
        }
        case WM_KEYDOWN:
            if (key==VK_ESCAPE) DestroyWindow(window);
            if ((key==VK_SPACE || key==VK_RETURN) && !(data & (1L<<30))) preview->advance_=true;
            return 0;
        case WM_LBUTTONUP:
        case WM_TIMER:
            preview->advance_=true;
            return 0;
        case WM_DESTROY:
            preview->window_=nullptr;
            return 0;
        }
        return DefWindowProcW(window,message,key,data);
    }

public:
    explicit Preview(bool enabled)
    {
        if (!enabled) return;
        WNDCLASSW windowClass{};
        windowClass.lpfnWndProc=Handle;
        windowClass.hInstance=GetModuleHandleW(nullptr);
        windowClass.hCursor=LoadCursor(nullptr,IDC_ARROW);
        windowClass.lpszClassName=L"Mir3CuiDialogSmokePreview";
        Check(RegisterClassW(&windowClass)!=0,"register preview window");
        window_=CreateWindowW(windowClass.lpszClassName,L"CUI dialog replay",WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
            CW_USEDEFAULT,CW_USEDEFAULT,800,600,nullptr,nullptr,windowClass.hInstance,this);
        Check(window_!=nullptr,"create preview window");
    }

    ~Preview() { if (window_) DestroyWindow(window_); }
    explicit operator bool() const { return window_!=nullptr; }

    void Show(std::string_view name, const std::vector<unsigned char>& pixels, int width, int height,
              UINT duration, POINT pointer={-1,-1})
    {
        pixels_=pixels.data();
        pointer_=pointer;
        bitmap_.bmiHeader={sizeof(BITMAPINFOHEADER),width,-height,1,32,BI_RGB};
        RECT bounds{0,0,width,height};
        AdjustWindowRect(&bounds,static_cast<DWORD>(GetWindowLongW(window_,GWL_STYLE)),FALSE);
        SetWindowPos(window_,nullptr,0,0,bounds.right-bounds.left,bounds.bottom-bounds.top,SWP_NOMOVE | SWP_NOZORDER);
        const auto title = L"CUI 对话框自动回放 | " + std::wstring(name.begin(),name.end()) +
            L" | Esc 结束预览";
        SetWindowTextW(window_,title.c_str());
        ShowWindow(window_,SW_SHOW);
        InvalidateRect(window_,nullptr,FALSE);
        UpdateWindow(window_);
        advance_=false;
        Check(SetTimer(window_,1,duration,nullptr)!=0,"start preview timer");
        MSG message{};
        while (window_ && !advance_) {
            const BOOL received = GetMessageW(&message,nullptr,0,0);
            Check(received>0,"read preview window message");
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        if (window_) KillTimer(window_,1);
        pixels_=nullptr;
    }
};

class Screen {
    cui_ctx* ui_ = nullptr;
    cui_sw_renderer* renderer_ = nullptr;
    Dialogs art_;
    fs::path screenshots_;
    Preview& preview_;
    int width_ = 800, height_ = 600;
    const std::vector<mir3_test::Action>& defaults_;
    const std::vector<mir3_test::Action>& actions_;
    bool reporting_=true;
    std::size_t nextAction_ = 0;

public:
    Screen(const fs::path& resources, const std::vector<mir3_test::Action>& defaults,
           const mir3_test::Scenario& scenario, fs::path screenshots, Preview& preview)
        : screenshots_(std::move(screenshots)), preview_(preview), defaults_(defaults),
          actions_(scenario.actions)
    {
        ui_ = cui_create(nullptr);
        renderer_ = cui_sw_create();
        Check(ui_ && renderer_,"create CUI software renderer");
        auto iface = cui_sw_iface(renderer_);
        cui_set_renderer(ui_,&iface);
        cui_set_theme(ui_,CUI_THEME_DARK);
        cui_set_accent(ui_,CUI_ACCENT_AMBER);
        cui_set_viewport(ui_,800,600,1);
        Check(art_.Open(ui_,resources),"open original CUI resources");
        if (!screenshots_.empty()) {
            fs::create_directories(screenshots_);
        }
        if (!screenshots_.empty() || preview_) {
            const auto font = fs::path(_wgetenv(L"WINDIR") ? _wgetenv(L"WINDIR") : L"C:/Windows") / "Fonts/msyh.ttc";
            if (fs::exists(font)) {
                const auto face = cui_font_load_file(ui_,font.string().c_str());
                if (face>=0) {
                    cui_font_add_fallback(ui_,CUI_FONT_BODY,face);
                    cui_font_add_fallback(ui_,CUI_FONT_MEDIUM,face);
                }
            }
        }
    }
    void Report(bool reporting) { reporting_=reporting; }
    ~Screen()
    {
        art_.Close(ui_);
        if (ui_) cui_destroy(ui_);
        if (renderer_) cui_sw_destroy(renderer_);
    }

    void Draw(LoginClient& client, bool native = true, float dt = 0.033f)
    {
        const bool wide = client.stage==Stage::Groups || client.stage==Stage::GameSession || client.returnToCharactersAt;
        const int width = wide ? 800 : 640, height = wide ? 600 : 480;
        if (width_!=width || height_!=height) {
            width_=width;
            height_=height;
            cui_set_viewport(ui_,static_cast<float>(width_),static_cast<float>(height_),1);
        }
        cui_begin_frame(ui_,dt);
        DrawDialogs(ui_,client,native ? &art_ : nullptr);
        cui_end_frame(ui_);
    }
    void Actions(LoginClient& client, std::string_view step, bool native = true)
    {
        const bool custom=reporting_ && nextAction_<actions_.size() && actions_[nextAction_].step==step;
        const auto& source=custom ? actions_ : defaults_;
        std::size_t next=custom ? nextAction_ :
            static_cast<std::size_t>(std::ranges::find(source,step,&mir3_test::Action::step)-source.begin());
        if (next==source.size())
            throw std::runtime_error("missing or out-of-order CSV step: "+std::string(step));
        const auto show = [&](std::string label, POINT pointer={-1,-1}, UINT duration=250) {
            if (reporting_ && preview_) preview_.Show(label,Pixels(),width_,height_,duration,pointer);
        };
        do {
            const auto& action=source[next++];
            Draw(client,native);
            if (action.kind=="click" || action.kind=="move" || action.kind=="press" || action.kind=="release") {
                POINT position{};
                std::istringstream coordinates(action.value);
                std::string extra;
                Check(bool(coordinates>>position.x>>position.y) && !(coordinates>>extra) &&
                    position.x>=0 && position.x<width_ && position.y>=0 && position.y<height_,
                    "CSV pointer position must be inside the current viewport");
                cui_input_mouse_move(ui_,static_cast<float>(position.x),static_cast<float>(position.y));
                Draw(client,native);
                if (action.kind=="click" || action.kind=="press") {
                    cui_input_mouse_button(ui_,CUI_MOUSE_LEFT,true);
                    Draw(client,native);
                }
                if (action.kind=="release") {
                    cui_input_mouse_button(ui_,CUI_MOUSE_LEFT,false);
                    Draw(client,native);
                }
                show(action.step+" | "+action.kind+" "+action.value,position);
                if (action.kind=="click") {
                    cui_input_mouse_button(ui_,CUI_MOUSE_LEFT,false);
                    Draw(client,native);
                }
            } else if (action.kind=="wheel") {
                float delta=0;
                std::string extra;
                std::istringstream value(action.value);
                Check(bool(value>>delta) && !(value>>extra) && std::isfinite(delta),"invalid CSV wheel delta");
                cui_input_scroll(ui_,0,delta);
                Draw(client,native);
                show(action.step+" | wheel "+action.value);
            } else if (action.kind=="key") {
                cui_key key=CUI_KEY_NONE;
                unsigned modifiers=0;
                if (action.value.size()==1 && action.value[0]>='A' && action.value[0]<='Z')
                    key=static_cast<cui_key>(CUI_KEY_A+action.value[0]-'A');
                else if (action.value=="Enter") key=CUI_KEY_ENTER;
                else if (action.value=="Escape") key=CUI_KEY_ESCAPE;
                else if (action.value=="Tab") key=CUI_KEY_TAB;
                else if (action.value=="Space") key=CUI_KEY_SPACE;
                else if (action.value=="Backspace") key=CUI_KEY_BACKSPACE;
                else if (action.value=="Ctrl+A") { key=CUI_KEY_A; modifiers=CUI_MOD_CTRL; }
                else if (action.value=="Alt+M") { key=CUI_KEY_M; modifiers=CUI_MOD_ALT; }
                Check(key!=CUI_KEY_NONE,"unknown CSV key");
                cui_input_key(ui_,key,true,modifiers);
                Draw(client,native);
                cui_input_key(ui_,key,false,0);
                Draw(client,native);
                show(action.step+" | Key "+action.value);
            } else if (action.kind=="text") {
                cui_input_text(ui_,action.value.c_str());
                Draw(client,native);
                show(action.step+" | Text");
            } else throw std::runtime_error("unknown CSV action: "+action.kind);
        } while (next<source.size() && source[next].step==step);
        if (custom) nextAction_=next;
    }
    void CheckActionsConsumed() const
    {
        if (nextAction_!=actions_.size()) throw std::runtime_error("unused or out-of-order CSV step: "+actions_[nextAction_].step);
    }
    std::vector<unsigned char> Pixels() const
    {
        std::vector<unsigned char> pixels(static_cast<std::size_t>(width_)*height_*4);
        cui_sw_render(renderer_,cui_get_draw_data(ui_),pixels.data(),width_,height_,width_*4,CUI_SW_BGRA);
        return pixels;
    }
    void Snapshot(LoginClient& client, const char* name, bool native = true)
    {
        if (!reporting_) return;
        Capture(client,name,native);
        if (nextAction_<actions_.size() && actions_[nextAction_].step==name)
            Actions(client,name,native);
    }

private:
    void Capture(LoginClient& client, const char* name, bool native)
    {
        Draw(client,native);
        if (screenshots_.empty() && !preview_) return;
        for (int i=0;i<9;++i) Draw(client,native);
        const auto pixels=Pixels();
        if (preview_) {
            std::cout << "[VIEW] " << name << std::endl;
            preview_.Show(name,pixels,width_,height_,1200);
        }
        if (screenshots_.empty()) return;
        BITMAPFILEHEADER header{};
        BITMAPINFOHEADER info{};
        header.bfType=0x4d42;
        header.bfOffBits=sizeof header+sizeof info;
        header.bfSize=header.bfOffBits+static_cast<DWORD>(pixels.size());
        info.biSize=sizeof info;
        info.biWidth=width_;
        info.biHeight=-height_;
        info.biPlanes=1;
        info.biBitCount=32;
        std::ofstream file(screenshots_/(std::string(name)+".bmp"),std::ios::binary);
        file.write(reinterpret_cast<const char*>(&header),sizeof header);
        file.write(reinterpret_cast<const char*>(&info),sizeof info);
        file.write(reinterpret_cast<const char*>(pixels.data()),pixels.size());
        Check(bool(file),"write CUI screenshot");
    }
};

class ScenarioRun {
    std::string_view selected_;
    Screen& screen_;
    bool ran_=false;

public:
    ScenarioRun(std::string_view selected, Screen& screen) : selected_(selected), screen_(screen) {}

    template<typename Action>
    bool Startup(const char* id, const char* name, Action action)
    {
        if (selected_.empty() || selected_==id) Run(id,name,action);
        else {
            screen_.Report(false);
            action();
            screen_.Report(true);
        }
        return ran_ && !selected_.empty();
    }

    template<typename Action>
    void Run(const char* id, const char* name, Action action)
    {
        if (!selected_.empty() && selected_!=id) return;
        ran_=true;
        const auto label=std::string(id)+" / "+name;
        Case(label.c_str(),[&] {
            action();
            if (Named()) screen_.CheckActionsConsumed();
        });
    }

    bool Ran() const { return ran_; }
    bool Named() const { return !selected_.empty(); }
};

void StartSession(LoginClient& client, Screen& screen, GatewayFixture& characters, GatewayFixture& game,
                  ScenarioRun& scenario)
{
    GatewayFixture login;
    const auto loginPort = login.Listen();
    characters.Listen();
    game.Listen();

    if (scenario.Startup("startup","server groups and settings", [&] {
        std::istringstream config(
            "[SERVERGROUP]\nCount=1\nServer1=Local,Local lines,100,200\n"
            "[ServerGroup1]\nCount=2\nServer1=Offline,empty,empty\nServer2=Online,LOOPBACK,unused\n"
            "[LOOPBACK]\nServerAddr=127.0.0.1\nParam1="+std::to_string(loginPort)+"\n");
        Check(client.serverGroups.Load(config) && client.serverGroups.groups[0].lines.size()==2,"load group and two lines");
        client.stage = Stage::Groups;
        screen.Snapshot(client,"01-groups");
        screen.Actions(client,"select-group");
        Check(client.serverGroups.selected==0,"select group from its map button");
        screen.Actions(client,"disabled-line");
        Check(client.stage==Stage::Groups,"disabled line leaves group open");
        screen.Actions(client,"online-line");
        Check(client.stage==Stage::Login && std::string(client.port)==std::to_string(loginPort),
            "active line opens configured login gateway");
        screen.Snapshot(client,"02-startup");
        screen.Snapshot(client,"03-account");
        client.status="This line has an invalid login configuration.";
        client.settingsOpen = true;
        screen.Snapshot(client,"04-gateway-settings");
        screen.Actions(client,"dismiss-login-connection-notice");
        Check(!client.status.empty(),"gateway settings defer the pending sign-in notice");
        client.settingsOpen = false;
        screen.Snapshot(client,"04b-login-connection-notice");
        screen.Actions(client,"dismiss-login-connection-notice");
        Check(client.status.empty(),"textured login notice dismisses uncatalogued status");
    })) return;

    if (scenario.Startup("login","password error, retry and billing notice", [&] {
        strcpy_s(client.host,"127.0.0.1");
        std::snprintf(client.port,sizeof client.port,"%u",loginPort);
        screen.Actions(client,"enter-account");
        Check(std::string_view(client.account)=="fixture","account typed into login form");
        client.configuredServers={{"Local","Local"}};
        const auto signIn = [&](const char* step) {
            screen.Actions(client,step);
            login.Accept(client);
            login.Expect(client,Packet(2001,0,0,0,0,"fixture/test"));
            Check(!client.password[0],"password cleared after queue");
        };
        signIn("login-first");
        login.Send(Packet(SM_PASSWD_FAIL,static_cast<unsigned>(-1)));
        const auto deadline = GetTickCount64()+5000;
        while (client.stage!=Stage::Login && GetTickCount64()<deadline) { client.Poll(); Sleep(1); }
        Check(client.stage==Stage::Login && client.loginNoticeId==113,"password rejection returns to login");
        screen.Snapshot(client,"05-password-rejected");
        client.DismissNotice();
        signIn("login-retry");
        login.Reply(client,Packet(529,0,0,3,0,"Local(2);"));
        Check(client.stage==Stage::Servers && client.billing.message==103,"billing notice blocks server selection");
        screen.Snapshot(client,"06-billing");
        screen.Actions(client,"dismiss-billing");
        Check(!client.billing.message && !client.servers.empty(),"dismiss billing notice");
        screen.Snapshot(client,"07-server-list");
        client.Select(client.servers[0]);
        login.Expect(client,Packet(104,0,0,0,0,"Local"));
    })) return;

    if (scenario.Startup("characters","character gateway / empty list, create, duplicate, deletion prompt", [&] {
        login.Send(Packet(530,static_cast<unsigned>(-123),0,0,0,characters.Address()+'/'));
        characters.Accept(client);
        characters.Expect(client,Packet(100,0,0,0,0,"fixture/-123"));
        characters.Reply(client,Packet(520));
        Check(client.characters.empty() && client.characterNoticeId==219,"empty character list notice");
        screen.Snapshot(client,"08-empty-characters");
        client.characterNotice.clear();
        client.OpenCharacterDialog(CharacterDialog::Create);
        screen.Actions(client,"invalid-character",false);
        Check(!client.characterRequest && client.characterNoticeId==225,"invalid name stays in creation");
        screen.Snapshot(client,"09-create-validation");
        client.characterNotice.clear();
        client.OpenCharacterDialog(CharacterDialog::Create);
        client.profession=2;
        client.gender=1;
        screen.Actions(client,"type-character-name",false);
        Check(std::string_view(client.characterInput)=="Hero","replacement character name typed");
        screen.Snapshot(client,"10-create");
        client.SubmitCharacterDialog();
        characters.Expect(client,Packet(101,0,0,0,0,"fixture/Hero/1/2/1"));
        characters.Reply(client,Packet(522,2));
        Check(client.characterDialog==CharacterDialog::Create && client.characterNoticeId==224,"duplicate name stays open");
        client.characterNotice.clear();
        client.SubmitCharacterDialog();
        characters.Expect(client,Packet(101,0,0,0,0,"fixture/Hero/1/2/1"));
        characters.Send(Packet(521));
        characters.Expect(client,Packet(100,0,0,0,0,"fixture/-123"));
        characters.Reply(client,Packet(520,2,0,0,0,"Hero/2/x/10/1/*Second/1/x/7/0"));
        Check(client.characters.size()==2 && client.characterIndex==0,"created role arrives in refreshed list");
        screen.Snapshot(client,"11-character-list");
        client.OpenCharacterDialog(CharacterDialog::Delete);
        screen.Snapshot(client,"12-delete-confirmation");
        client.CloseCharacterDialog();
        Check(!client.characterRequest,"cancel delete does not queue a request");
        client.characterIndex=1;
        client.OpenCharacterDialog(CharacterDialog::Delete);
        strcpy_s(client.characterInput,"Second");
        client.SubmitCharacterDialog();
        characters.Expect(client,Packet(102,0,0,0,0,"Second/Second"));
        characters.Expect(client,Packet(100,0,0,0,0,"fixture/-123"));
        characters.Reply(client,Packet(523)+Packet(520,1,0,0,0,"Hero/2/x/10/1"));
        Check(client.characters.size()==1 && client.characters[0].name=="Hero","delete returns to refreshed list");
    })) return;

    if (scenario.Startup("game-entry","game gateway / failed start, retry and enter HUD", [&] {
        client.StartCharacter();
        characters.Expect(client,Packet(103,0,0,0,0,"fixture/Hero"));
        characters.Reply(client,Packet(526));
        Check(client.stage==Stage::Characters && client.characterNoticeId==221,"failed start retains character list");
        client.characterNotice.clear();
        client.StartCharacter();
        characters.Expect(client,Packet(103,0,0,0,0,"fixture/Hero"));
        characters.Send(Packet(525,0,0,0,0,game.Address()));
        game.Accept(client);
        game.Expect(client,"#"+Encode("**fixture/Hero/-1051903415/-1420889006/-80534433/1")+'!');
        std::string feature(16,'\0');
        feature[0]=1;
        feature[3]=0x43;
        const auto body = Encode(feature)+Encode(std::string(5,'\0'))+Encode(std::string(16,'\0'));
        game.Reply(client,"#+GOOD!"+ScenePacket(50,100,28,45,6,body));
        Check(client.stage==Stage::GameSession && client.gameCharacter=="Hero","valid logon enters CUI HUD");
        game.Game(client,GameChangeMap,100,436,439,0,"0");
        screen.Snapshot(client,"13-hud");
    })) return;
}
std::string SampleItem()
{
    std::string item(100,'\0');
    item.replace(0,5,"Sword");
    item[30]=5;
    Field(item,36,45,2);
    Field(item,62,1234);
    Field(item,66,100,2);
    Field(item,68,100,2);
    Field(item,71,1,2);
    return item;
}

void InspectGameDialogs(LoginClient& client, Screen& screen, GatewayFixture& characters, GatewayFixture& game,
                        ScenarioRun& scenario)
{
    const auto item = SampleItem();
    const auto open = [&](unsigned opcode,std::uint32_t actor=0,WORD first=0,
                          std::string_view text={}) { game.Game(client,opcode,actor,first,0,0,text); };
    const auto prepareBag = [&] {
        if (client.inventory.initialized) return;
        client.OpenInventory();
        game.Expect(client,Packet(81));
        game.Game(client,201,0,0,0,1,Encode(item)+"/",true);
        client.CloseInventory();
    };

    scenario.Run("hud","HUD / chat, reception and target map controls", [&] {
        screen.Actions(client,"leave-chat");
        Check(!client.chat.composing,"leave initial chat editing mode");
        const auto expanded = client.chat.expanded;
        screen.Actions(client,"expand-chat");
        Check(client.chat.expanded!=expanded,"toggle chat height");
        screen.Snapshot(client,"14-chat-expanded");
        screen.Actions(client,"toggle-private-reception");
        game.Expect(client,Packet(mir3_cui::Say,0,0,0,0,
            mir3_cui::ChatReception::ToggleCommand(mir3_cui::ReceptionChannel::Private)));
        game.Game(client,mir3_cui::ClientOptions,0x04000000);
        Check(client.chat.reception.confirmed[0],"textured HUD reception button applies server options");
        screen.Snapshot(client,"15-reception");
        screen.Actions(client,"collapse-chat");
        client.ToggleMinimap(0);
        game.Expect(client,Packet(1033));
        game.Game(client,710,0,1001);
        screen.Snapshot(client,"16-minimap");
        Check(client.minimap.visible,"minimap reply draws the requested map");
        screen.Actions(client,"resize-minimap");
        Check(client.minimap.large,"resize minimap");
        screen.Actions(client,"minimap-opacity");
        Check(client.minimap.translucent,"toggle minimap opacity");
        screen.Snapshot(client,"17-minimap-large");
        client.minimap.visible=false;
    });

    scenario.Run("inventory","0-1, 8, 16 / bag, equipment, discard and quick slots", [&] {
        screen.Actions(client,"open-bag");
        game.Expect(client,Packet(81));
        game.Game(client,201,0,0,0,1,Encode(item)+"/",true);
        open(653,100);
        Check(client.inventory.open && client.inventory.items.size()==1,"bag reply contains an item");
        screen.Snapshot(client,"18-bag");
        const std::string equipment = "0/"+Encode(item)+"/";
        game.Game(client,621,0,0,0,0,equipment,true);
        client.OpenEquipment();
        Check(client.equipment.slots[0].key.id==1234,"equipment slot receives the item");
        screen.Snapshot(client,"19-equipment");
        client.equipment.open=false;
        client.BeginInventoryDrop(mir3_cui::DropDialog::Gold);
        Check(client.inventory.drop==mir3_cui::DropDialog::Gold,"gold amount prompt");
        screen.Snapshot(client,"20-drop-gold");
        client.inventory.drop=mir3_cui::DropDialog::None;
        client.CloseInventory();
        screen.Actions(client,"open-item-shortcuts");
        screen.Snapshot(client,"21-item-shortcuts");
        Check(client.inventory.shortcutsVisible && client.inventory.shortcuts.size()==6,"six item shortcut slots");
        screen.Actions(client,"close-item-shortcuts");
        client.inventory.notice=true;
        client.inventory.status="The server declined to use the item.";
        screen.Snapshot(client,"21b-item-notice");
        screen.Actions(client,"dismiss-item-notice");
        Check(!client.inventory.notice && client.inventory.status.empty(),"textured item notice dismisses with Enter");
    });

    scenario.Run("npc","9 / NPC links, mask, cancellation and server close", [&] {
        open(GameNpcDialogue,201,0,"0/Welcome\\<Password/@@*3_enter_stgpasswd>{NOTCLOSE/1}");
        Check(client.npcDialogue.actor==201 && !client.npcDialogue.canClose,"server opens locked dialogue");
        screen.Snapshot(client,"22-npc");
        const auto option = std::ranges::find_if(client.npcDialogue.rows,
            [](const auto& row) { return row.command.starts_with("@@*"); });
        Check(option!=client.npcDialogue.rows.end(),"masked script option");
        client.ChooseNpcOption(*option);
        Check(client.NpcAnswerLimit()==3 && !client.npcDialogue.prompt.command.empty(),"password editor limit");
        screen.Snapshot(client,"23-npc-password");
        client.EndNpcPrompt(false);
        Check(client.npcDialogue.prompt.command.empty(),"cancel preserves NPC page");
        open(GameNpcClose,201);
        Check(!client.npcDialogue.actor,"server closes the locked page");
    });

    scenario.Run("shop","2 / shop catalogue, individual detail, bulk and purchase error", [&] {
        open(GameNpcDialogue,201,0,"0/Shop");
        open(GameShopGoods,201,2,"Potion/1/25/1/301/0/0/Bundle/2/8/65539/302/0/0/");
        Check(client.shop.goods.size()==2,"two catalogue rows");
        screen.Snapshot(client,"24-shop-catalogue");
        client.RequestShopOffers(0,0);
        game.Expect(client,Packet(ClientShopOffers,201,0,0,0,"Potion"));
        game.Game(client,GameShopOffers,201,1,0,0,Encode(Encode(item)+"/"),true);
        Check(client.shop.offers.size()==1,"individual offer record");
        screen.Snapshot(client,"25-shop-single");
        client.RequestShopOffers(1,0);
        Check(client.shop.bulk,"bulk mode opens an independent detail");
        screen.Snapshot(client,"26-shop-bulk");
        open(GameShopRejected,2);
        Check(client.shop.notice==1438,"ordinary insufficient gold notice");
        screen.Snapshot(client,"27-shop-failure");
        open(GameNpcClose,201);
    });

    scenario.Run("merchant-services","2 / crafting and merchant service variants", [&] {
        open(GameCraftGoods,201,0,"Recipe/2/50/0/301/0/0/");
        client.RequestShopOffers(0,0);
        screen.Snapshot(client,"28-craft");
        client.BuyShopItem();
        game.Expect(client,Packet(ClientCraft,201,0,0,0,"Recipe"));
        open(GameCraftFailed,4);
        Check(client.shop.notice==1444 && !client.purchase.deadline,"craft result clears pending request");
        screen.Snapshot(client,"29-craft-failure");
        open(GameCraftOk,88);
        Check(client.inventory.gold==88 && client.shop.notice==1440,"craft success");
        open(GameNpcClose,201);
        using Service=mir3_cui::MerchantService;
        for (const auto [opcode,kind,name] : {
            std::tuple{GameOpenSell,Service::Kind::Sell,"30-sell"},
            std::tuple{GameOpenRepair,Service::Kind::Repair,"31-repair"},
            std::tuple{GameStorageOpen,Service::Kind::Deposit,"32-deposit"}}) {
            open(opcode,201);
            Check(client.merchant.actor==201 && client.merchant.kind==kind && client.inventory.open,"merchant opens bag");
            screen.Snapshot(client,name);
            open(GameNpcClose,201);
        }
        game.Game(client,GameStorageList,201,0,0,1,Encode(item)+"/",true);
        Check(client.shop.storage && client.shop.offers.size()==1,"warehouse withdrawal list");
        screen.Snapshot(client,"33-withdrawal-list");
        open(GameNpcClose,201);
        open(GameStorageBagFull);
        Check(client.merchant.notice==1446,"warehouse bag-full result");
        screen.Snapshot(client,"34-warehouse-error");
        client.merchant={};
    });

    scenario.Run("trade","3 / trade rejection, paired bag, gold and cancellation", [&] {
        prepareBag();
        open(653,100);
        open(GameTradeFailed);
        Check(client.trade.prompt==mir3_cui::TradePrompt::Failed,"failed trade prompt");
        screen.Snapshot(client,"35-trade-failed");
        client.FinishTradePrompt(false);
        open(GameTradeOpen,0,0,"Trader");
        Check(client.trade.visible && client.inventory.open && client.trade.partner=="Trader","open two-party trade");
        screen.Snapshot(client,"36-trade");
        client.BeginTradeGold();
        Check(client.trade.prompt==mir3_cui::TradePrompt::Gold,"trade amount dialog");
        screen.Snapshot(client,"37-trade-gold");
        client.FinishTradePrompt(false);
        open(GameTradeCancelled);
        Check(!client.trade.visible,"cancel closes trade");
        client.CloseInventory();
    });

    scenario.Run("party","6 / party roster, member editor and rejection", [&] {
        screen.Actions(client,"open-party");
        open(667,0,0,"Leader/Bob/");
        Check(client.party.members.size()==2,"party roster");
        screen.Snapshot(client,"38-party");
        client.party.Begin(mir3_cui::PartyAction::Invite);
        screen.Snapshot(client,"39-party-invite");
        client.party.Begin(mir3_cui::PartyAction::None);
        open(664,static_cast<unsigned>(-5));
        Check(client.party.notice.id!=0,"invite failure");
        screen.Snapshot(client,"40-party-failure");
        client.party={};
    });

    scenario.Run("guild","4, 15 / guild pages, permissions and announcement editor", [&] {
        screen.Actions(client,"open-guild");
        game.Expect(client,Packet(1036));
        open(753,0,0,"Fixture\r\r0\r<Notice>\rWelcome\rHel");
        Check(client.guild.open && !client.guild.overview.manager,"guild overview for non-manager");
        screen.Snapshot(client,"41-guild-overview");
        client.BeginGuildAction(mir3_cui::GuildAction::Notice);
        Check(client.guild.notice==4000,"guild editor permission");
        client.guild.notice=0;
        client.RequestGuildPage(1);
        game.Expect(client,Packet(1037));
        open(756,0,0,"#1/Leader/*/Member/*/");
        screen.Snapshot(client,"42-guild-members");
        open(753,0,0,"Fixture\r\r1\r<Notice>\rWelcome\rHel");
        client.BeginGuildAction(mir3_cui::GuildAction::Notice);
        Check(client.guild.Multiline(),"manager announcement uses multiline editor");
        screen.Snapshot(client,"43-guild-announcement");
        client.guild.Begin(mir3_cui::GuildAction::None);
        client.RequestGuildPage(2);
        screen.Snapshot(client,"44-guild-history");
        client.guild={};
    });

    scenario.Run("quests","11 / quests empty and populated states", [&] {
        screen.Actions(client,"open-quests");
        game.Expect(client,Packet(1046));
        Check(client.quests.waiting,"quest list waits for gateway");
        screen.Snapshot(client,"45-quests-waiting");
        open(1301);
        Check(!client.quests.waiting && client.quests.entries.empty(),"empty reply clears waiting state");
        screen.Snapshot(client,"45b-quests-empty");
        open(1301,0,1,"30/Border patrol/2/0/Visit the gate/");
        Check(client.quests.entries.size()==1,"one quest loaded");
        client.quests.parent=30;
        screen.Snapshot(client,"46-quest-detail");
        client.quests.open=false;
    });

    scenario.Run("options-skills","12-14, 22 / settings, audio, riding, skills and portrait picker", [&] {
        open(1401,0x4a800000,4);
        screen.Actions(client,"open-settings");
        Check(client.serverOptions.confirmed==0x4a800000 && client.serverOptions.attackMode==4,"server settings applied");
        screen.Snapshot(client,"47-system-settings");
        client.serverOptions.open=false;
        client.audioOpen=true;
        screen.Snapshot(client,"48-audio");
        const bool reverse=client.audioOptions.reversePan;
        screen.Actions(client,"toggle-stereo");
        Check(client.audioOptions.reversePan!=reverse,"textured audio control changes stereo");
        screen.Actions(client,"close-audio");
        Check(!client.audioOpen,"textured audio panel closes");
        screen.Actions(client,"show-mount");
        Check(client.mount.visible,"mount shortcut opens panel");
        screen.Snapshot(client,"49-mount");
        open(1300,1);
        Check(client.mount.shape==1,"mount status reply");
        client.mount.visible=false;
        screen.Actions(client,"show-skills");
        Check(client.skills.visible && client.skills.learned.size()==8,"skill tree opens with eight tabs");
        screen.Snapshot(client,"50-skill-tree");
        client.skills.visible=false;
        screen.Actions(client,"show-skill-shortcuts");
        Check(client.skillShortcutsVisible,"separate skill shortcut bar");
        screen.Snapshot(client,"51-skill-shortcuts");
        client.skillShortcutsVisible=false;
        screen.Draw(client);
        const auto before=screen.Pixels();
        screen.Actions(client,"open-portrait");
        screen.Snapshot(client,"52-photo-picker");
        const auto after=screen.Pixels();
        const auto sample=(85*800+425)*4;
        Check(!std::equal(before.begin()+sample,before.begin()+sample+3,after.begin()+sample),"portrait picker paints its preview area");
        screen.Actions(client,"close-portrait");
    });

    scenario.Run("profile-orders","7, 18 / profile and server-enabled order map", [&] {
        std::string profile(1308,'\0');
        profile.replace(4,8,"Traveler");
        profile.replace(83,100,item);
        open(751,201,0,profile);
        Check(client.windows.profile.open && client.windows.profile.equipment[0].key.id==1234,"profile is a separate window");
        screen.Snapshot(client,"53-profile");
        screen.Actions(client,"close-profile");
        Check(!client.windows.profile.open,"close only the profile");
        open(1350,28|(45u<<16),1,"0\t201");
        const bool needsMap=!client.minimap.width;
        screen.Actions(client,"open-orders");
        Check(client.windows.orders.open,"order map requires server permission");
        if (needsMap) {
            game.Expect(client,Packet(1033));
            game.Game(client,710,0,1001);
        }
        screen.Snapshot(client,"54-orders");
        open(1353);
        Check(!client.windows.orders.open,"server revokes order map");
    });

    scenario.Run("consignment","19-20 / consignment list, search, and registration slot", [&] {
        prepareBag();
        std::string record(312,'\0');
        record.replace(0,5,"Local");
        Field(record,16,22);
        record.replace(20,6,"Seller");
        Field(record,72,25);
        record.replace(91,100,item);
        record.replace(191,5,"Notes");
        game.Game(client,1355,201,1,1,1,Encode(record)+"/",true);
        Check(client.windows.market.open && client.windows.market.listings.size()==1,"consignment list decoded");
        screen.Actions(client,"select-listing");
        Check(client.windows.market.selected==22,"click listing selects it");
        screen.Snapshot(client,"55-market");
        screen.Actions(client,"search-listings");
        game.Expect(client,Packet(1136,201,0,0,0,"*\rSword\r0"));
        open(1400,1);
        client.BeginMarketRegistration();
        client.equipment.held={1234,"Sword"};
        screen.Actions(client,"drop-consignment-item");
        Check(client.windows.market.item.id==1234 && !client.HeldItem(),"drop shared bag item into slot");
        Check(client.windows.positions[3].y==22,"registration begins at visible skin edge");
        screen.Actions(client,"type-consignment-price");
        screen.Snapshot(client,"56-registration");
        screen.Actions(client,"confirm-consignment");
        game.Expect(client,Packet(1130,201,0,0,0,"Sword\r1234\r30\r"));
        Check(client.inventory.Find({1234,"Sword"})!=nullptr,"registration waits for bag delta");
        client.windows.market={};
        client.CloseInventory();
    });

    scenario.Run("mail","21 / inbox, mail detail with gold, compose and outbox", [&] {
        open(1358);
        Check(client.windows.mail.unreadNotice,"new message flag");
        screen.Snapshot(client,"56b-new-mail");
        screen.Actions(client,"open-mailbox");
        game.Expect(client,Packet(1137));
        std::string letter(80,'\0');
        Field(letter,0,42);
        letter.replace(8,6,"Sender");
        letter.replace(64,7,"Subject");
        std::string inbox=Encode(letter)+"/";
        for (unsigned i=1;i<9;++i) {
            auto next=letter;
            Field(next,0,42+i);
            Field(next,6,1,2);
            next.replace(8,6,"Copy "+std::to_string(i));
            inbox+=Encode(next)+"/";
        }
        game.Game(client,1356,0,0,0,9,inbox,true);
        Check(client.windows.mail.folders[0].size()==9,"inbox list");
        screen.Snapshot(client,"57-inbox");
        screen.Actions(client,"scroll-inbox");
        Check(client.windows.mail.firstVisible[0]==2,"mail list scrolls beyond the first seven rows");
        screen.Snapshot(client,"57b-inbox-scrolled");
        screen.Actions(client,"read-inbox-letter");
        Check(client.windows.mail.firstVisible[0]==0 && client.windows.mail.selected==42,
            "first message selected after scrolling back");
        game.Expect(client,Packet(1138,42));
        std::string body(613,'\0');
        Field(body,0,42);
        Field(body,8,12);
        body.replace(112,10,"Hello\rMail");
        open(1357,0,0,body);
        Check(client.windows.mail.reading && client.windows.mail.gold==12,"read message and gold");
        screen.Snapshot(client,"58-letter");
        screen.Actions(client,"write-letter");
        screen.Snapshot(client,"59-compose");
        screen.Actions(client,"send-letter");
        std::string outgoing(596,'\0');
        outgoing.replace(0,6,"Sender");
        outgoing.replace(64,5,"Reply");
        outgoing.replace(95,5,"Reply");
        game.Expect(client,Packet(1141,0,0,0,0,outgoing));
        client.MailRequest(1137,1);
        game.Expect(client,Packet(1137,0,2));
        game.Game(client,1356,0,2,0,1,Encode(letter)+"/",true);
        Check(client.windows.mail.folders[1].size()==1,"outbox list");
        screen.Snapshot(client,"60-outbox");
        client.windows.mail={};
    });

    scenario.Run("sign-out","exit / confirmation, cancel and sign-out request", [&] {
        client.showSignOut=true;
        screen.Snapshot(client,"61-exit-confirmation");
        client.showSignOut=false;
        Check(client.stage==Stage::GameSession,"cancel stays in game");
        client.SignOutGame(true);
        game.Expect(client,Packet(1009));
        Check(client.stage==Stage::SigningOutGame,"return-to-character request");
        screen.Snapshot(client,"62-return-fade");
        client.returnToCharactersAt=GetTickCount64();
        characters.Accept(client);
        characters.Expect(client,Packet(100,0,0,0,0,"fixture/-123"));
        characters.Reply(client,Packet(520,1,0,0,0,"Hero/2/x/10/1"));
        Check(client.stage==Stage::Characters && client.characters.size()==1,"character list returns after fade");
        screen.Snapshot(client,"63-returned-characters");
    });
}
} // namespace

int wmain(int argc,wchar_t** argv)
{
    if (argc<4) {
        std::cerr << "Usage: Mir3CuiDialogSmoke RESOURCE_ROOT RUN_DIRECTORY PLAN_CSV"
                     " [--defaults ACTIONS_CSV] [--screenshots] [--preview]\n";
        return 2;
    }
    bool screenshots=false, preview=false;
    fs::path defaultsPath;
    for (int i=4;i<argc;++i) {
        const std::wstring_view option(argv[i]);
        if (option==L"--screenshots") screenshots=true;
        else if (option==L"--preview") preview=true;
        else if (option==L"--defaults" && i+1<argc) defaultsPath=fs::absolute(argv[++i]);
        else return 2;
    }
    WSADATA winsock{};
    if (WSAStartup(MAKEWORD(2,2),&winsock)!=0) return 1;
    const auto com = CoInitializeEx(nullptr,COINIT_APARTMENTTHREADED);
    int result=0;
    try {
        Check(SUCCEEDED(com),"initialize COM");
        const auto resources=fs::absolute(argv[1]);
        const auto run=fs::absolute(argv[2]);
        const auto plan=mir3_test::ReadPlan(fs::absolute(argv[3]));
        std::vector<mir3_test::Action> defaults;
        if (!plan.front().id.empty()) Check(!defaultsPath.empty(),"named scenarios require --defaults ACTIONS_CSV");
        if (!defaultsPath.empty()) {
            auto actionPlan=mir3_test::ReadPlan(defaultsPath);
            Check(actionPlan.front().id.empty(),"expected step,action,value for default actions");
            defaults=std::move(actionPlan.front().actions);
        }
        Preview window(preview);
        unsigned passed=0;
        for (const auto& selection : plan) {
            try {
                const auto state=selection.id.empty() ? run : run/"state"/selection.id;
                fs::create_directories(state);
                fs::current_path(state);
                LoginClient client;
                client.SetResourceRoot(resources);
                client.audioOptions.music=client.audioOptions.effects=false;
                client.audioOptionsPath=(state/"smoke-options.ini").wstring();
                Screen screen(resources,defaults,selection,screenshots ? run/"screenshots" : fs::path{},window);
                GatewayFixture characters,game;
                ScenarioRun scenario(selection.id,screen);
                StartSession(client,screen,characters,game,scenario);
                if (!scenario.Named() || !scenario.Ran()) {
                    if (scenario.Named() && selection.id!="hud") client.chat.composing=false;
                    InspectGameDialogs(client,screen,characters,game,scenario);
                }
                if (!scenario.Ran()) throw std::runtime_error("unknown scenario: "+selection.id);
                if (!scenario.Named()) screen.CheckActionsConsumed();
                ++passed;
            } catch (const std::exception& error) {
                std::cerr << "[FAIL] " << (selection.id.empty() ? "" : selection.id+": ") << error.what() << '\n';
                result=1;
            }
        }
        if (!plan.front().id.empty()) std::cout << "[SUMMARY] " << passed << '/' << plan.size() << " scenarios passed\n";
        if (!result) std::cout << "CUI_DIALOG_SMOKE_OK\n";
    } catch (const std::exception& error) {
        std::cerr << "[FAIL] " << error.what() << '\n';
        result=1;
    }
    if (SUCCEEDED(com)) CoUninitialize();
    WSACleanup();
    return result;
}
