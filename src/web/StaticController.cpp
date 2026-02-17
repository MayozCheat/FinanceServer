#include "web/StaticController.h"
#include "core/FileUtil.h"

void StaticController::Register(HttpServer& http) {
    auto& s = http.Raw();

    s.Get("/app/", [this](const httplib::Request&, httplib::Response& res) {
        std::string path = webroot_ + "/app/index.html";
        std::string html = FileUtil::ReadAllText(path);
        if (html.empty()) {
            res.status = 404;
            res.set_content("index.html not found", "text/plain; charset=utf-8");
            return;
        }
        res.set_content(html, "text/html; charset=utf-8");
    });
}
