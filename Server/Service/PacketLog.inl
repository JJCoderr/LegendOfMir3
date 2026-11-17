// Included after the service's stdafx.h, inside its isolated namespace.
#ifdef MIR3_EISERVER
static void TracePacket(const wchar_t* direction, SOCKET socket, const char* packet)
{
    static const bool enabled = GetPrivateProfileIntW(L"Debug", L"MessageLog", 1, L".\\EIServer.ini") != 0;
    if (!enabled) return;
    const size_t length = strcspn(packet, "!");
    if (length < _DEFBLOCKSIZE) return;
    std::string frame(packet, length);
    _TDEFAULTMESSAGE header{};
    fnDecodeMessageA(&header, &frame[0]);
    wchar_t prefix[256];
    swprintf_s(prefix, L"MSG %ls socket=%llu id=%u recog=%d param=%u tag=%u series=%u ",
        direction, static_cast<unsigned long long>(socket), header.wIdent,
        header.nRecog, header.wParam, header.wTag, header.wSeries);
    std::wstring message(prefix);
    if (!wcscmp(direction, L"RX") && (header.wIdent == CM_IDPASSWORD ||
        header.wIdent == CM_DELCHR || header.wIdent == CM_ADDNEWUSER || header.wIdent == CM_CHANGEPASSWORD || header.wIdent == CM_UPDATEUSER)) {
        message += L"body=<credentials redacted>";
    } else {
        const std::string encoded = frame.substr(_DEFBLOCKSIZE);
        std::vector<char> decoded(encoded.size() + 1);
        const int count = fnDecode6BitBufA(const_cast<char*>(encoded.c_str()), decoded.data(), int(decoded.size()));
        message += L"body_bytes=" + std::to_wstring(count) + L" body=\"";
        constexpr int MaxBodyPreview = 512;
        bool binary = false;
        for (int i = 0; i < count; ++i)
            if ((static_cast<unsigned char>(decoded[i]) < 32 && !(i == count - 1 && decoded[i] == 0)) || decoded[i] == 127) binary = true;
        std::string escaped;
        for (int i = 0; i < count && i < MaxBodyPreview; ++i) {
            const unsigned char ch = decoded[i];
            if (ch < 32 || ch == 127 || (binary && ch >= 128) || ch == '\\' || ch == '"') {
                char hex[5]; sprintf_s(hex, "\\x%02X", ch); escaped += hex;
            } else escaped += char(ch);
        }
        int size = MultiByteToWideChar(CP_ACP, 0, escaped.data(), int(escaped.size()), nullptr, 0);
        std::wstring text(size, L'\0');
        if (size) MultiByteToWideChar(CP_ACP, 0, escaped.data(), int(escaped.size()), &text[0], size);
        message += text + L"\"";
        if (count > MaxBodyPreview) message += L" [truncated]";
        // Some messages contain several separately encoded records and delimiters.
        if (!encoded.empty() && !count) {
            message += L" wire=\"";
            for (size_t i = 0; i < encoded.size() && i < MaxBodyPreview; ++i) {
                wchar_t hex[5]; swprintf_s(hex, L"%02X ", static_cast<unsigned char>(encoded[i])); message += hex;
            }
            message += L"\"";
            if (encoded.size() > MaxBodyPreview) message += L" [truncated]";
        }
    }
    mir3::ConsoleLog(ServiceName, message.c_str());
}
#else
#define TracePacket(direction, socket, packet) ((void)0)
#endif
