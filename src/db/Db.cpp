#include "db/Db.h"
#include "core/Logger.h"

bool Db::Connect(const std::string& host, int port,
    const std::string& user,
    const std::string& password,
    const std::string& database) {
    try {
        sql::Driver* driver = get_driver_instance();
        std::string url = "tcp://" + host + ":" + std::to_string(port);

        conn_.reset(driver->connect(url, user, password));
        conn_->setSchema(database);

        Logger::Instance().Info("[数据库] 连接成功: " + database);
        return true;
    }
    catch (const std::exception& e) {
        Logger::Instance().Error(std::string("[数据库] 连接失败: ") + e.what());
        return false;
    }
}

bool Db::Query(const std::string& sqlText,
    std::vector<std::string>& outColNames,
    std::vector<Row>& outRows,
    std::string& outErr) {
    outColNames.clear();
    outRows.clear();
    outErr.clear();

    if (!conn_) {
        outErr = "db_not_connected";
        return false;
    }

    try {
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(sqlText));

        // ⚠ 关键修复：不能用 unique_ptr
        sql::ResultSetMetaData* meta = rs->getMetaData();

        int colCount = meta->getColumnCount();

        for (int i = 1; i <= colCount; i++) {
            outColNames.push_back(meta->getColumnLabel(i));
        }

        while (rs->next()) {
            Row r;
            r.cols.reserve(colCount);

            for (int i = 1; i <= colCount; i++) {
                r.cols.push_back(rs->getString(i));
            }

            outRows.push_back(std::move(r));
        }

        return true;
    }
    catch (const std::exception& e) {
        outErr = e.what();
        return false;
    }
}
