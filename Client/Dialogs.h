#pragma once

#include <filesystem>
#include <memory>

struct cui_ctx;

namespace mir3_client {

class LoginClient;

class Dialogs {
public:
    Dialogs();
    ~Dialogs();

    bool Open(cui_ctx* ui, const std::filesystem::path& resourceRoot);
    void Close(cui_ctx* ui);
    bool DefersGameEntry() const;

private:
    struct Assets;
    std::unique_ptr<Assets> assets_;

    friend void DrawDialogs(cui_ctx*, LoginClient&, Dialogs*);
};

void DrawDialogs(cui_ctx* ui, LoginClient& client, Dialogs* dialogs = nullptr);

} // namespace mir3_client
