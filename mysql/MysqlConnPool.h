//
// Created by zhujiaying on 2021/6/28.
//

#ifndef MYWEBSERVER_MYSQLPOOL_H
#define MYWEBSERVER_MYSQLPOOL_H

#include <string>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <mysqlx/xdevapi.h>

class MysqlConnPool{
public:
    typedef std::shared_ptr<mysqlx::Session> spConnection;

    static MysqlConnPool& getInstance();

    void Init(std::string& host, int port,
              std::string& username, std::string& password,
              std::string& dbname, int connNum);

    spConnection getSqlconn();

    void returnSqlconn(spConnection conn);

private:
    MysqlConnPool();

    ~MysqlConnPool();

    std::vector<spConnection> _sqlconns;

    std::mutex _mutex;

    std::condition_variable _cond;

//    std::shared_ptr<mysqlx::Schema> _db;
};

#endif //MYWEBSERVER_MYSQLPOOL_H
