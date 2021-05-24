#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <nlohmann/json.hpp>
#include "WS.hpp"
using json = nlohmann::json;
struct User{

    string name;
    websocketpp::connection_hdl hdl;
    User* other=NULL;
};
struct UUIDConn {
    websocketpp::connection_hdl hdl;
    string uuid;
};
struct P2PConn {
    string id;
    UUIDConn* a;
    UUIDConn* b;
};
class MyHandler : public ServerHandler {
public:
    vector<User> users;
    vector<P2PConn> p2pconns;
    vector<UUIDConn> uuidconnections;
    UUIDConn* getconnbyhdl(websocketpp::connection_hdl& hdl) {
        for (size_t i = 0; i < uuidconnections.size(); i++)
        {
            if (uuidconnections[i].hdl.lock() == hdl.lock()) {
                return &uuidconnections[i];
            }
        }
    }
    UUIDConn* getconnbyuuid(string uuid) {
        for (size_t i = 0; i < uuidconnections.size(); i++)
        {
            if (uuidconnections[i].uuid == uuid) {
                return &uuidconnections[i];
            }
        }
        return NULL;
    }
    websocketpp::connection_hdl gethdlbyuuid(string dst) {
        for (size_t i = 0; i < uuidconnections.size(); i++) {
            if (uuidconnections[i].uuid == dst) {
                return uuidconnections[i].hdl;
            }
        }
    }
    void onMessage(websocketpp::connection_hdl hdl, message_ptr msg) override {
        std::string str = msg->get_payload();
       // std::cout <<"'''"<< str.c_str()<<"'''"<<std::endl;

        auto j = json::parse(str);
        auto thisconn = getconnbyhdl(hdl);
        if (thisconn == NULL)return;
        auto thisuuid = thisconn->uuid;
        j["src"] = thisuuid;
        if (j.contains("type")) {
            string type = j["type"].get<std::string>();
            if (type == "CANDIDATE"|| type == "ANSWER") {
                if (j.contains("payload")) {
                    auto payload = j["payload"];
                    if (payload.contains("connectionId")) {
                        auto connid = payload["connectionId"].get<std::string>();
                        for (size_t i = 0; i < p2pconns.size(); i++)
                        {
                            if (p2pconns[i].id == connid) {
                                if (!j.contains("dst")) {
                                    if (p2pconns[i].a->uuid == thisuuid) {
                                        j["dst"] = p2pconns[i].b->uuid;
                                    }
                                    else
                                        if (p2pconns[i].b->uuid == thisuuid) {
                                            j["dst"] = p2pconns[i].a->uuid;
                                        }
                                }
                                
                                if (j.contains("dst")) {
                                    string dst = j["dst"].get<std::string>();
                                    server->send(gethdlbyuuid(dst), j.dump());
                                }
                                return;
                            }
                        }
                    }
                }
            }
            else if (type == "OFFER") {
                if (j.contains("dst")) {
                    string dst = j["dst"].get<std::string>();
                    server->send(gethdlbyuuid(dst), j.dump());
                    if (j.contains("payload")) {
                        auto payload = j["payload"];
                        if (payload.contains("connectionId")) {
                            auto connid = payload["connectionId"].get<std::string>();
                            auto a = getconnbyhdl(hdl);
                            auto b = getconnbyuuid(dst);
                            p2pconns.push_back({ connid,a,b });
                        }
                    }
                }
                else {
                    printf("unknown type 1 %s\n", type.c_str());
                }
            }
            else {
                if (type == "HEARTBEAT") {
                }
                else {
                    printf("unknown type 2 %s\n", type.c_str());
                }
            }
        }
        else {

        }
    }
    virtual void onOpen(websocketpp::connection_hdl hdl) override{
        server->send(hdl, "{\"type\":\"OPEN\"}");
        server::connection_ptr con = server->echo_server.get_con_from_hdl(hdl);
        
        printf("uri %s\n", con->get_uri().get()->str().c_str());
        printf("res %s\n", con->get_uri().get()->get_resource().c_str());
        printf("host %s\n", con->get_uri().get()->get_authority().c_str());
        printf("host %s\n", con->get_uri().get()->get_host().c_str());
        printf("port %s\n", con->get_uri().get()->get_port_str().c_str()); 
        string q = con->get_uri().get()->get_query();
        printf("query %s\n", q.c_str());
        printf("scheme %s\n", con->get_uri().get()->get_scheme().c_str());
        if (con->get_uri().get()->get_resource().find("/peerjs?") == 0) {
            //key=peerjs&id=0a8f174f-a449-e4d9-5000-7c12896eb661&token=l6tkdy2ffrk
            int iid = q.find("&id=");
            if (iid != -1) {
                int itoken = q.find("&token=");
                if (itoken != -1) {
                    int start = iid + strlen("&id=");
                    string uuid = q.substr(start, itoken - start);
                    printf("user uuid %s\n", uuid.c_str());
                    uuidconnections.push_back({ hdl,uuid });
                }
            }
        }
    }
    virtual void onError()override {

    }
    virtual void onClose(websocketpp::connection_hdl hdl)override {
        for (auto it = users.begin(); it != users.end(); it++)
        {
            User u = *it;
            if (u.hdl.lock() == hdl.lock()) {
                users.erase(it);
                return;
            }
        }
        
    }
};
int main()
{   
    srand(time(0));
    MyHandler h = MyHandler();
    WebsocketServer* ws = new WebsocketServer(&h);
    std::cout << "Hello World!\n";
}
