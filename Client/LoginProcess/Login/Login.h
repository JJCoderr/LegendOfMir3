#pragma once
#include "../LoginLayout.h"
#include <string>
class CLogin : public CBMWnd {
public:
    enum Action { None, Submit, Register, Password, Exit };
    void Create(CWHWilImageData* image);
    void ResetPassword();
    HRESULT OnKeyDown(WPARAM key, LPARAM) override;
    HRESULT OnButtonDown(WPARAM, LPARAM position) override;
    HRESULT OnButtonUp(WPARAM, LPARAM position) override;
    LRESULT OnMouseMove(WPARAM, LPARAM position) override;
    void Render(INT) override;
    void SetEditorVisible(bool visible);
private:
    friend struct LoginPreview;
    friend class CLoginProcess;
    void ChangeField(int target);
    void SyncEditor();
    Action SubmitEditor(bool mouse);
    int Hit(POINT point);
    CWHWilImageData* m_image=nullptr;
    std::string m_values[2];
    int m_field=0, m_pressed=-1;
    POINT m_pointer={};
    bool m_editorVisible=true;
};
