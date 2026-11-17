#pragma once
#include <string_view>
namespace SelectionDialog {
enum Message {Empty=219,QueryFailed=220,StartFailed=221,Disconnected=222,NameLong=223,NameExists=224,
    NameInvalid=225,Full=226,DeleteMissing=227,DeleteConfirmation=228,StartMissing=230,CreateFailed=232};
constexpr int NameBytes=13,ConfirmationBytes=90,CreationLimit=2;
inline int CreationFailure(unsigned reason) {return reason==2?NameExists:reason==3?Full:CreateFailed;}
inline int LegacyCreationFailure(unsigned reason) {
    enum { ExistingName=1,InvalidName=2,NoSpace=3 };
    return reason==ExistingName?NameExists:reason==InvalidName?NameInvalid:reason==NoSpace?Full:CreateFailed;
}
inline int NameError(std::string_view name) {
    if(name.size()>NameBytes) return NameLong;
    constexpr std::string_view forbidden=" /@?'\"\\.,:;`~!#$%^&*()-_+=|[{}]<>";
    for(size_t i=0;i<name.size();) {
        const unsigned char c=name[i];
        if(!c || forbidden.find(char(c))!=std::string_view::npos) return NameInvalid;
        if((c>='a' && c<='z') || (c>='A' && c<='Z') || (c>='0' && c<='9')) ++i;
        else {if(i+1>=name.size() || !name[i+1]) return NameInvalid;i+=2;}
    }
    return 0;
}
}
