#include <websocketpp/config/asio.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "Util.hpp"
#define DEBUG
typedef websocketpp::server<websocketpp::config::asio> server;
//typedef websocketpp::server<websocketpp::config::asio_tls> server;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
typedef websocketpp::config::asio::message_type::ptr message_ptr;
typedef websocketpp::lib::shared_ptr<websocketpp::lib::asio::ssl::context> context_ptr;
class WebsocketServer;

class ServerHandler {
public:
    WebsocketServer* server=NULL;
    ServerHandler() {

    }
    std::vector<websocketpp::connection_hdl*> connections;
    virtual void onMessage(websocketpp::connection_hdl hdl, message_ptr msg) {

    }
    virtual void onOpen(websocketpp::connection_hdl hdl) {

    }
    virtual void onError() {

    }
    virtual void onClose(websocketpp::connection_hdl hdl) {

    }
    virtual void onhttp(websocketpp::connection_hdl hdl){}

};
void on_open(server* s, ServerHandler* handler, websocketpp::connection_hdl hdl) {
#ifdef DEBUG
    std::cout << "opened connection to:" << hdl.lock().get() << std::endl;
    
#endif
    //s->get_con_from_hdl(hdl).get()->get_uri().get()->get_query()
    //printf("uri %s\n", .c_str());
    handler->connections.push_back(&hdl);
    handler->onOpen(hdl);
    

}
void on_close(server* server, ServerHandler* handler,
    websocketpp::connection_hdl hdl) {
#ifdef DEBUG
    std::cout << "on_close" << std::endl;
#endif
    vectorremove(handler->connections, vectorgeti(handler->connections, &hdl));
    handler->onClose(hdl);
}
std::string htmlpage = "";//to keep pages in ram
string readfiletostring(const char* file) {
    std::ifstream ifs(file);
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    htmlpage = buffer.str();
    ifs.close();
    return htmlpage;
}
void on_http(server* s, ServerHandler* handler, websocketpp::connection_hdl hdl) {
    server::connection_ptr con = s->get_con_from_hdl(hdl);
    con->append_header("Location", "https://localhost");
    //con->add_subprotocol("html");
    printf("uri %s\n", con->get_uri().get()->str().c_str());
    printf("res %s\n", con->get_uri().get()->get_resource().c_str());
    printf("host %s\n", con->get_uri().get()->get_authority().c_str());
    printf("host %s\n", con->get_uri().get()->get_host().c_str());
    printf("port %s\n", con->get_uri().get()->get_port_str().c_str());
    printf("query %s\n", con->get_uri().get()->get_query().c_str());
    printf("scheme %s\n", con->get_uri().get()->get_scheme().c_str());
    con->append_header("Content-Type", "text/html;charset=UTF-8");
    if (con->get_uri().get()->get_resource().find("/peerjs/id")==0) {
        con->append_header("Access-Control-Allow-Origin","*");
        auto uuid = generateuuid();
        con->set_body(uuid);
        con->set_status(websocketpp::http::status_code::ok);
        return;
    }
    if (con->get_uri().get()->get_resource().find("/peerjs.min.js")==0  ) {
        con->append_header("Content-Type", "application/javascript");
        con->set_body(readfiletostring("peerjs.min.js"));
        con->set_status(websocketpp::http::status_code::ok);
        return;
    }
    if (con->get_uri().get()->get_resource().find("/s.css") == 0) {
        con->append_header("Content-Type", "text/css");

        con->set_body(readfiletostring("s.css"));
        con->set_status(websocketpp::http::status_code::ok);

        return;
    }
    
    con->set_body(readfiletostring("index2.html"));//reading every time from disk to make easier changes    
    con->set_status(websocketpp::http::status_code::ok);
}

std::string get_password() {
    return "test";
}
enum tls_mode {
    MOZILLA_INTERMEDIATE = 1,
    MOZILLA_MODERN = 2
};
context_ptr on_tls_init(tls_mode mode, websocketpp::connection_hdl hdl) {
    namespace asio = websocketpp::lib::asio;

    std::cout << "on_tls_init called with hdl: " << hdl.lock().get() << std::endl;
    std::cout << "using TLS mode: " << (mode == MOZILLA_MODERN ? "Mozilla Modern" : "Mozilla Intermediate") << std::endl;

    context_ptr ctx = websocketpp::lib::make_shared<asio::ssl::context>(asio::ssl::context::sslv23);

    try {
        if (mode == MOZILLA_MODERN) {
            // Modern disables TLSv1
            ctx->set_options(asio::ssl::context::default_workarounds |
                asio::ssl::context::no_sslv2 |
                asio::ssl::context::no_sslv3 |
                asio::ssl::context::no_tlsv1 |
                asio::ssl::context::single_dh_use);
        }
        else {
            ctx->set_options(asio::ssl::context::default_workarounds |
                asio::ssl::context::no_sslv2 |
                asio::ssl::context::no_sslv3 |
                asio::ssl::context::single_dh_use);
        }
        ctx->set_password_callback(bind(&get_password));
        
            ctx->use_certificate_chain_file("server.pem");
            ctx->use_private_key_file("server.pem", asio::ssl::context::pem);

        
        // Example method of generating this file:
        // `openssl dhparam -out dh.pem 2048`
        // Mozilla Intermediate suggests 1024 as the minimum size to use
        // Mozilla Modern suggests 2048 as the minimum size to use.
        ctx->use_tmp_dh_file("dh.pem");

        std::string ciphers;

        if (mode == MOZILLA_MODERN) {
            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!3DES:!MD5:!PSK";
        }
        else {
            ciphers = "ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-GCM-SHA384:DHE-RSA-AES128-GCM-SHA256:DHE-DSS-AES128-GCM-SHA256:kEDH+AESGCM:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA:ECDHE-ECDSA-AES256-SHA:DHE-RSA-AES128-SHA256:DHE-RSA-AES128-SHA:DHE-DSS-AES128-SHA256:DHE-RSA-AES256-SHA256:DHE-DSS-AES256-SHA:DHE-RSA-AES256-SHA:AES128-GCM-SHA256:AES256-GCM-SHA384:AES128-SHA256:AES256-SHA256:AES128-SHA:AES256-SHA:AES:CAMELLIA:DES-CBC3-SHA:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS-DES-CBC3-SHA:!EDH-RSA-DES-CBC3-SHA:!KRB5-DES-CBC3-SHA";
        }

        if (SSL_CTX_set_cipher_list(ctx->native_handle(), ciphers.c_str()) != 1) {
            std::cout << "Error setting cipher list" << std::endl;
        }
    }
    catch (std::exception& e) {
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return ctx;
}
void on_message(server* s, ServerHandler* handler, websocketpp::connection_hdl hdl, message_ptr msg) {
    //std::cout << "on_message called with hdl: " << hdl.lock().get()<< " and message: " << msg->get_payload()<< std::endl;

    try {
        handler->onMessage(hdl, msg);
    }
    catch (websocketpp::exception const& e) {
        std::cout << "Echo failed because: "
            << "(" << e.what() << ")" << std::endl;
    }
}
class WebsocketServer {
public:
    server echo_server;
    WebsocketServer() {

    }
    void send(websocketpp::connection_hdl hdl, string msg) {
        echo_server.send(hdl, msg, websocketpp::frame::opcode::text);
    }
    WebsocketServer(ServerHandler* handler, int port = 9002) {
        srand(time(0));
        

        if (handler == NULL)handler = new ServerHandler();
        handler->server = this;
        // Initialize ASIO
        echo_server.init_asio();

        // Register our message handler
        echo_server.set_open_handler(websocketpp::lib::bind(&on_open, &echo_server, handler, ::_1));

        echo_server.set_close_handler(
            websocketpp::lib::bind(&on_close, &echo_server, handler, ::_1));

        echo_server.set_message_handler(bind(&on_message, &echo_server, handler, ::_1, ::_2));
        echo_server.set_http_handler(bind(&on_http, &echo_server, handler, ::_1));
        //echo_server.set_tls_init_handler(bind(&on_tls_init, MOZILLA_INTERMEDIATE, ::_1));
        
        // Listen on port 9002
        echo_server.listen(port);

        // Start the server accept loop
        echo_server.start_accept();
        
        // Start the ASIO io_service run loop
        echo_server.run();
    }
};
