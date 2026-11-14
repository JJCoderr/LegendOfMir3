#pragma once
#include "protocol/login_stream.hpp"
#include <godot_cpp/classes/stream_peer_tcp.hpp>
#include <godot_cpp/variant/array.hpp>
#include <deque>
#include <algorithm>
namespace mir3 {
// 连接只负责收发；页面回包、提示和重连决策由主界面处理。
class ClientConnection {
    godot::Ref<godot::StreamPeerTCP> peer;
    LoginStream stream;
    std::deque<godot::PackedByteArray> outgoing;
    std::int64_t outgoing_offset=0;
    bool connected=false,active=false;
public:
    struct Result {
        bool opened=false,closed=false,failed=false;
        std::vector<std::string> messages;
    };
    void close() {
        if (peer.is_valid()) peer->disconnect_from_host();
        peer.unref();stream.clear();outgoing.clear();outgoing_offset=0;connected=active=false;
    }
    bool open(const godot::String& host,int port) {
        close();peer.instantiate();
        active=peer->connect_to_host(host,port)==godot::OK;
        return active;
    }
    bool send(const godot::PackedByteArray& bytes) {
        if (!connected || bytes.is_empty()) return false;
        outgoing.push_back(bytes);return true;
    }
    Result poll() {
        Result result;if (!active || peer.is_null()) return result;
        peer->poll();const auto status=peer->get_status();
        if (status==godot::StreamPeerTCP::STATUS_ERROR) { close();result.failed=true;return result; }
        if (status==godot::StreamPeerTCP::STATUS_NONE) { result.closed=connected;result.failed=!connected;close();return result; }
        if (status!=godot::StreamPeerTCP::STATUS_CONNECTED) return result;
        if (!connected) { connected=true;result.opened=true; }
        // 每次recv最多8192；保留半包，每个接收块中的星号仅回一个心跳。
        for (int block=0;block<8 && peer->get_available_bytes()>0;++block) {
            const auto read=peer->get_partial_data(std::min(8192,peer->get_available_bytes()));
            if (read.size()!=2 || int(read[0])!=godot::OK) { close();result.failed=true;return result; }
            const godot::PackedByteArray bytes=read[1];if (bytes.is_empty()) break;
            auto decoded=stream.receive({bytes.ptr(),std::size_t(bytes.size())});
            if (decoded.overflow) { close();result.failed=true;return result; }
            if (decoded.heartbeat) { godot::PackedByteArray heartbeat;heartbeat.push_back('*');outgoing.push_back(heartbeat); }
            for (auto& message:decoded.messages) result.messages.push_back(std::move(message));
        }
        while (!outgoing.empty()) {
            const auto remaining=outgoing.front().slice(outgoing_offset);
            const auto written=peer->put_partial_data(remaining);
            if (written.size()!=2 || int(written[0])!=godot::OK) { close();result.failed=true;return result; }
            const std::int64_t count=written[1];
            if (count<=0) break;
            outgoing_offset+=count;
            if (outgoing_offset>=outgoing.front().size()) { outgoing.pop_front();outgoing_offset=0; }
        }
        return result;
    }
};
}
