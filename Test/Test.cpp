#include <windows.h>
#include "../Server/Def/picojson.h"
#include "../Server/Def/sqlite/sqlite3.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <memory>
#include <limits>
#include <regex>
#include <stdexcept>
using Json = picojson::value;
using Object = picojson::object;
using Array = picojson::array;
namespace fs = std::filesystem;

// RFC 4180 cells: quoted commas, doubled quotes and embedded newlines.
std::vector<std::vector<std::string>> ReadCsv(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open CSV");
    std::string data((std::istreambuf_iterator<char>(file)), {});
    if (data.compare(0,3,"\xef\xbb\xbf")==0) data.erase(0,3);
    std::vector<std::vector<std::string>> rows;
    std::vector<std::string> row;
    std::string cell;
    bool quoted=false, closed=false;
    for (size_t i=0;i<data.size();++i) {
        const char c=data[i];
        if (quoted) {
            if (c=='"') {
                if (i+1<data.size() && data[i+1]=='"') {cell+='"';++i;}
                else {quoted=false;closed=true;}
            } else cell+=c;
        } else if (c==',' || c=='\r' || c=='\n') {
            row.push_back(cell);cell.clear();closed=false;
            if (c!=',') {
                if (c=='\r' && i+1<data.size() && data[i+1]=='\n') ++i;
                if (row.size()!=1 || !row[0].empty()) rows.push_back(row);
                row.clear();
            }
        } else if (c=='"' && cell.empty() && !closed) quoted=true;
        else {
            if (closed || c=='"') throw std::runtime_error("Invalid CSV quoting");
            cell+=c;
        }
    }
    if (quoted) throw std::runtime_error("Unclosed CSV quote");
    if (!cell.empty() || !row.empty() || closed) {row.push_back(cell);rows.push_back(row);}
    return rows;
}
Json Value(const std::string& text) {
    Json result;std::string error;
    auto end=picojson::parse(result,text.begin(),text.end(),&error);
    if (error.empty() && end==text.end()) return result;
    return Json(text);
}
// Parameters and assertions: key=value;key=value. JSON quoted strings can contain ';'.
Object Fields(const std::string& text) {
    Object result;size_t start=0;bool quote=false,escape=false;
    for(size_t i=0;i<=text.size();++i) {
        if(i==text.size() || (text[i]==';' && !quote)) {
            const auto part=text.substr(start,i-start);start=i+1;
            if(part.empty()) continue;
            const auto equal=part.find('=');
            if(equal==std::string::npos || !equal) throw std::runtime_error("Expected key=value");
            if(!result.emplace(part.substr(0,equal),Value(part.substr(equal+1))).second)
                throw std::runtime_error("Duplicate field");
        } else if(escape) escape=false;
        else if(text[i]=='\\' && quote) escape=true;
        else if(text[i]=='"') quote=!quote;
    }
    if(quote) throw std::runtime_error("Unclosed parameter quote");
    return result;
}
const Json* At(const Json& value,const std::string& path) {
    const Json* current=&value;std::istringstream parts(path);std::string part;
    while(std::getline(parts,part,'.')) {
        if(current->is<Object>()) {
            const auto& object=current->get<Object>();const auto found=object.find(part);
            if(found==object.end()) return nullptr;current=&found->second;
        } else if(current->is<Array>() && !part.empty() && part.find_first_not_of("0123456789")==std::string::npos) {
            const auto index=std::stoull(part);const auto& array=current->get<Array>();
            if(index>=array.size()) return nullptr;current=&array[size_t(index)];
        } else return nullptr;
    }
    return current;
}
// PicoJSON's generic numeric equality converts int64 values to double in place.
// Compare recursively without losing integer precision or changing the response log.
bool Equal(const Json& left,const Json& right) {
    if(left.is<int64_t>() && right.is<int64_t>()) return left.get<int64_t>()==right.get<int64_t>();
    if(left.is<double>() && right.is<double>()) {
        if(!left.is<int64_t>() && !right.is<int64_t>()) return left.get<double>()==right.get<double>();
        const int64_t integer=left.is<int64_t>()?left.get<int64_t>():right.get<int64_t>();
        const double real=left.is<int64_t>()?right.get<double>():left.get<double>();
        const double upper=-double(std::numeric_limits<int64_t>::min());
        if(real<double(std::numeric_limits<int64_t>::min()) || real>=upper || std::floor(real)!=real) return false;
        return integer==int64_t(real);
    }
    if(left.is<Array>() && right.is<Array>()) {
        const auto& a=left.get<Array>();const auto& b=right.get<Array>();
        if(a.size()!=b.size()) return false;
        for(size_t i=0;i<a.size();++i) if(!Equal(a[i],b[i])) return false;
        return true;
    }
    if(left.is<Object>() && right.is<Object>()) {
        const auto& a=left.get<Object>();const auto& b=right.get<Object>();
        if(a.size()!=b.size()) return false;
        for(const auto& entry:a) {
            const auto found=b.find(entry.first);
            if(found==b.end() || !Equal(entry.second,found->second)) return false;
        }
        return true;
    }
    if(left.is<double>() || right.is<double>()) return false;
    return left==right;
}
std::string Difference(const Json& actual,const Object& expected) {
    for(const auto& item:expected) {
        const auto* value=At(actual,item.first);
        if(!value || !Equal(*value,item.second)) return item.first+": expected "+item.second.serialize()+", actual "+(value?value->serialize():"<missing>");
    }
    return {};
}
struct Handle {
    HANDLE value=INVALID_HANDLE_VALUE;
    ~Handle(){if(value!=INVALID_HANDLE_VALUE) CloseHandle(value);}
};
class Pipe {
    Handle handle;
public:
    Pipe(const std::string& role,DWORD pid) {
        const auto name="\\\\.\\pipe\\Mir3."+role+"."+std::to_string(pid);
        if(!WaitNamedPipeA(name.c_str(),10000)) throw std::runtime_error("RPC pipe unavailable: "+role);
        handle.value=CreateFileA(name.c_str(),GENERIC_READ|GENERIC_WRITE,0,nullptr,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,nullptr);
        if(handle.value==INVALID_HANDLE_VALUE) throw std::runtime_error("Cannot connect RPC pipe: "+role);
    }
    DWORD Transfer(bool write,char* data,DWORD length,ULONGLONG deadline) {
        Handle event;event.value=CreateEventW(nullptr,TRUE,FALSE,nullptr);
        OVERLAPPED operation{};operation.hEvent=event.value;DWORD count=0;
        const BOOL done=write?WriteFile(handle.value,data,length,&count,&operation):ReadFile(handle.value,data,length,&count,&operation);
        if(!done) {
            if(GetLastError()!=ERROR_IO_PENDING) throw std::runtime_error("RPC disconnected");
            const auto now=GetTickCount64();
            const DWORD remaining=now<deadline?DWORD(deadline-now):0;
            if(WaitForSingleObject(event.value,remaining)!=WAIT_OBJECT_0) {
                CancelIoEx(handle.value,&operation);
                GetOverlappedResult(handle.value,&operation,&count,TRUE);
                throw std::runtime_error("RPC timeout");
            }
            if(!GetOverlappedResult(handle.value,&operation,&count,FALSE)) throw std::runtime_error("RPC read/write failed");
        }
        if(!count) throw std::runtime_error("RPC closed");
        return count;
    }
    Json Exchange(std::string request,DWORD timeout) {
        const auto deadline=GetTickCount64()+timeout;request+='\n';
        size_t sent=0;
        while(sent<request.size()) sent+=Transfer(true,&request[sent],DWORD(request.size()-sent),deadline);
        std::string response;char buffer[4096];
        while(response.find('\n')==std::string::npos) {
            response.append(buffer,Transfer(false,buffer,sizeof(buffer),deadline));
            if(response.size()>4*1024*1024) throw std::runtime_error("RPC response too large");
        }
        Json value;const auto error=picojson::parse(value,response);
        if(!error.empty()) throw std::runtime_error("Invalid RPC response: "+error);
        return value;
    }
};
Json Query(const fs::path& path,const std::string& sql) {
    sqlite3* raw=nullptr;
    const auto name=path.u8string();
    const auto status=sqlite3_open_v2(name.c_str(),&raw,SQLITE_OPEN_READONLY,nullptr);
    std::unique_ptr<sqlite3,decltype(&sqlite3_close)> db(raw,sqlite3_close);
    if(status!=SQLITE_OK) throw std::runtime_error("Cannot open test database");
    sqlite3_busy_timeout(raw,5000);
    sqlite3_stmt* stmt=nullptr;
    if(sqlite3_prepare_v2(raw,sql.c_str(),-1,&stmt,nullptr)!=SQLITE_OK) throw std::runtime_error(sqlite3_errmsg(raw));
    std::unique_ptr<sqlite3_stmt,decltype(&sqlite3_finalize)> statement(stmt,sqlite3_finalize);
    Array rows;int rc;
    while((rc=sqlite3_step(stmt))==SQLITE_ROW) {
        Array row;
        for(int c=0;c<sqlite3_column_count(stmt);++c) {
            switch(sqlite3_column_type(stmt,c)) {
            case SQLITE_NULL:row.emplace_back();break;
            case SQLITE_INTEGER:row.emplace_back(int64_t(sqlite3_column_int64(stmt,c)));break;
            case SQLITE_FLOAT:row.emplace_back(sqlite3_column_double(stmt,c));break;
            default:row.emplace_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt,c))));
            }
        }
        rows.emplace_back(row);
    }
    if(rc!=SQLITE_DONE) throw std::runtime_error(sqlite3_errmsg(raw));
    return Json(Object{{"rows",Json(rows)}});
}
int Opcode(const fs::path& csv,const std::string& name) {
    auto repo=fs::absolute(csv).parent_path();
    while(!fs::exists(repo/"Server/Def/Protocol.h")) {
        if(repo==repo.root_path()) throw std::runtime_error("Cannot find protocol headers");
        repo=repo.parent_path();
    }
    const std::regex define(R"(^\s*#define\s+(CM_\w+|SM_\w+)\s+(0[xX][0-9a-fA-F]+|[0-9]+)\b)");
    for(const char* source:{"Client/Common/Protocol.h","Client/Define.h","Server/Def/Protocol.h"}) {
        std::ifstream input(repo/source);std::string line;std::smatch match;
        while(std::getline(input,line)) if(std::regex_search(line,match,define) && match[1]==name)
            return std::stoi(match[2],nullptr,0);
    }
    throw std::runtime_error("Unknown protocol: "+name);
}
unsigned Number(const std::wstring& text) {
    if(text.empty() || text.find_first_not_of(L"0123456789")!=std::wstring::npos) throw std::runtime_error("Expected unsigned number");
    const auto number=std::stoull(text);
    if(number>std::numeric_limits<unsigned>::max()) throw std::runtime_error("Number too large");
    return unsigned(number);
}
int wmain(int argc,wchar_t** argv) {
    std::ofstream log;unsigned passed=0,step=0;std::string label;
    const auto started=GetTickCount64();
    auto output=[&](const std::string& message) {
        SYSTEMTIME time;GetLocalTime(&time);char stamp[48];
        snprintf(stamp,sizeof(stamp),"%04d-%02d-%02d %02d:%02d:%02d.%03d ",time.wYear,time.wMonth,time.wDay,time.wHour,time.wMinute,time.wSecond,time.wMilliseconds);
        std::cout<<stamp<<message<<std::endl;if(log) log<<stamp<<message<<std::endl;
    };
    try {
        if(argc<2) throw std::runtime_error("Usage: Test.exe scenario.csv --client-pid N --server-pid N [--client2-pid N --from N --to N --log file --database file --timeout-ms N]");
        fs::path csv=argv[1],logPath=L"test.log",database;
        std::map<std::string,DWORD> pids;unsigned from=1,to=UINT_MAX,timeout=65000;
        for(int i=2;i<argc;i+=2) {
            if(i+1>=argc) throw std::runtime_error("Missing argument value");
            const std::wstring option=argv[i],value=argv[i+1];
            if(option==L"--log") logPath=value;
            else if(option==L"--database") database=value;
            else if(option==L"--client-pid") pids["Client"]=Number(value);
            else if(option==L"--client2-pid") pids["Client2"]=Number(value);
            else if(option==L"--server-pid") pids["Server"]=Number(value);
            else if(option==L"--from") from=Number(value);
            else if(option==L"--to") to=Number(value);
            else if(option==L"--timeout-ms") timeout=Number(value);
            else throw std::runtime_error("Unknown argument");
        }
        if(!logPath.parent_path().empty()) fs::create_directories(logPath.parent_path());
        log.open(logPath,std::ios::binary|std::ios::trunc);
        if(!log) throw std::runtime_error("Cannot open log file");
        if(!from || from>to || !timeout) throw std::runtime_error("Invalid execution range or timeout");
        auto rows=ReadCsv(csv);
        const std::vector<std::string> header={"step","name","target","action","params","expect"};
        if(rows.empty() || rows[0]!=header) throw std::runtime_error("Expected CSV header: step,name,target,action,params,expect");
        unsigned previous=0;size_t selected=0;
        for(size_t i=1;i<rows.size();++i) {
            const auto& row=rows[i];
            if(row.size()!=header.size()) throw std::runtime_error("Wrong CSV field count at record "+std::to_string(i+1));
            const unsigned id=Number(std::wstring(row[0].begin(),row[0].end()));
            if(!id || id<=previous) throw std::runtime_error("Step IDs must be positive and strictly increasing");
            previous=id;if(id>=from && id<=to) ++selected;
        }
        if(!selected) throw std::runtime_error("No steps in selected range");
        output("START "+csv.u8string()+" from="+std::to_string(from)+" to="+std::to_string(to));
        std::map<std::string,std::unique_ptr<Pipe>> pipes;
        for(size_t i=1;i<rows.size();++i) {
            const auto& row=rows[i];step=unsigned(std::stoul(row[0]));
            if(step<from || step>to) continue;
            label=row[1];const auto& target=row[2];const auto& action=row[3];
            const auto expected=Fields(row[5]);Json response;
            output("STEP "+std::to_string(step)+" "+label+" "+target+" "+action);
            if(target=="Database") {
                if(action!="query" || database.empty()) throw std::runtime_error("Database query requires --database");
                response=Query(database,row[4]);
            } else {
                if(target!="Client" && target!="Client2" && target!="Server") throw std::runtime_error("Target must be Client, Client2, Server or Database");
                if(!pipes.count(target)) {
                    if(!pids[target]) throw std::runtime_error("Missing PID for "+target);
                    pipes[target]=std::make_unique<Pipe>(target=="Client2"?"Client":target,pids[target]);
                }
                auto& pipe=*pipes[target];
                const auto fields=Fields(action=="raw"?"":row[4]);
                const auto request=action=="raw"?row[4]:Json(Object{
                    {"jsonrpc",Json(std::string("2.0"))},{"id",Json(int64_t(step))},
                    {"method",Json(action=="state.wait"?std::string("state.get"):action=="wire.expect"?std::string("protocol.get"):action)},
                    {"params",Json(action=="wire.expect"?Object{}:fields)}}).serialize();
                const auto deadline=GetTickCount64()+timeout;
                std::string previousWaitingState;
                do {
                    const auto now=GetTickCount64();
                    if(now>=deadline) throw std::runtime_error("State wait timeout: "+Difference(response,expected));
                    response=pipe.Exchange(request,DWORD(deadline-now));
                    if(action!="raw" && (!response.is<Object>() || !Equal(response.get("id"),Json(int64_t(step))) || response.get("jsonrpc")!=Json(std::string("2.0"))))
                        throw std::runtime_error("RPC response ID/version mismatch");
                    if(action!="state.wait" || Difference(response,expected).empty()) break;
                    const auto current=response.serialize();
                    if(current!=previousWaitingState) {output("WAIT "+std::to_string(step)+" response="+current);previousWaitingState=current;}
                    if(response.contains("error")) throw std::runtime_error("RPC error: "+response.get("error").serialize());
                    Sleep(20);
                } while(true);
                if(action=="wire.expect") {
                    const auto name=fields.at("message").get<std::string>();
                    const auto direction=fields.at("direction").get<std::string>();
                    const int opcode=Opcode(csv,name);int64_t count=0;
                    for(const auto& entry:response.get("result").get("observed").get<Array>())
                        if(entry.get("direction")==Json(direction) && Equal(entry.get("opcode"),Json(int64_t(opcode))))
                            count+=entry.get("count").get<int64_t>();
                    const auto minimum=fields.count("at_least")?fields.at("at_least").get<int64_t>():1;
                    if(count<minimum) throw std::runtime_error(name+": expected at least "+std::to_string(minimum)+", actual "+std::to_string(count));
                    response=Json(Object{{"message",Json(name)},{"count",Json(count)}});
                }
            }
            const auto difference=Difference(response,expected);
            if(!difference.empty()) throw std::runtime_error(difference);
            if(response.is<Object>() && response.contains("error") && !expected.count("error.code"))
                throw std::runtime_error("Unexpected RPC error: "+response.get("error").serialize());
            output("PASS "+std::to_string(step)+" response="+response.serialize());++passed;
        }
        output("RESULT passed checks="+std::to_string(passed)+" elapsed_ms="+std::to_string(GetTickCount64()-started));
        return 0;
    } catch(const std::exception& error) {
        output("FAIL step="+std::to_string(step)+" "+label+" "+error.what());
        output("RESULT failed checks="+std::to_string(passed)+" elapsed_ms="+std::to_string(GetTickCount64()-started));
        return 1;
    }
}
