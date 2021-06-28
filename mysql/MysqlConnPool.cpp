//
// Created by zhujiaying on 2021/6/28.
//

#include "MysqlConnPool.h"

#include <assert.h>


MysqlConnPool & MysqlConnPool::getInstance() {
    static MysqlConnPool mysql;
    return mysql;
}

MysqlConnPool::MysqlConnPool()
{

}

MysqlConnPool::~MysqlConnPool() {

}

void MysqlConnPool::Init(std::string &host, int port, std::string &username, std::string &password,
                         std::string &dbname, int connNum) {
    for (int i=0; i<connNum; ++i){
        spConnection conn = std::make_shared<mysqlx::Session>(host, port, username, password);
        _sqlconns.push_back(conn);
    }

}

MysqlConnPool::spConnection MysqlConnPool::getSqlconn() {
    std::unique_lock<std::mutex> lk(_mutex);
    _cond.wait(lk, [this](){return !_sqlconns.empty(); });

    spConnection conn = _sqlconns.back();
    assert(conn!=nullptr);
    _sqlconns.pop_back();
    return conn;
}

void MysqlConnPool::returnSqlconn(spConnection conn) {
    std::unique_lock<std::mutex> lk(_mutex);
    _sqlconns.push_back(conn);
    _cond.notify_one();
}