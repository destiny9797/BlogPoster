//
//  main.cpp
//  m146
//
//  Created by zhujiaying on 2021/4/4.
//

#include <iostream>
#include <unordered_map>
#include <list>
//#include <utility>

using namespace std;

class LRUCache1 {
public:
    LRUCache1(int capacity) {
        _capacity = capacity;
    }
    
    int get(int key) {
        if (_cache.find(key) != _cache.end()){
            pair<int,int> node = *_cache[key];
            _lru.erase(_cache[key]);
            _lru.push_front(node);
            _cache[key] = _lru.begin();
            return node.second;
        }
        else{
            return -1;
        }
    }
    
    void put(int key, int value) {
        if (_cache.find(key) != _cache.end()){
            _lru.erase(_cache[key]);
            _cache.erase(key);
        }
        if (_cache.size() >= _capacity){
            _cache.erase(_lru.back().first);
            _lru.pop_back();
        }
        _lru.push_front({key,value});
        _cache[key] = _lru.begin();
    }
    
private:
    int _capacity;
    unordered_map<int, list<pair<int,int>>::iterator> _cache;
    list<pair<int,int>> _lru;
};

struct Dlist{
    int key;
    int value;
    Dlist* last;
    Dlist* next;
    Dlist(int k, int v): key(k), value(v), last(nullptr), next(nullptr){}
};


class LRUCache {
public:
    LRUCache(int capacity) : _capacity(capacity), _size(0) {
        _dummyfront = new Dlist(-1,-1);
        _dummyend = new Dlist(-1,-1);
        _dummyfront->next = _dummyend;
        _dummyend->last = _dummyfront;
    }
    
    int get(int key) {
        if (_cache.find(key) == _cache.end())
            return -1;
        Dlist* node = _cache[key];
        moveToHead(node);
        return node->value;
    }
    
    void put(int key, int value) {
        if (_cache.find(key) != _cache.end()){
            _cache[key]->value = value;
            moveToHead(_cache[key]);
        }
        else{
            Dlist* node = new Dlist(key,value);
            addToHead(node);
            _cache[key] = node;
            _size++;
        }
        if (_size > _capacity){
            Dlist* node = _dummyend->last;
            removeNode(node);
            _cache.erase(node->key);
            delete node;
            _size--;
        }
    }
    
    void addToHead(Dlist* node){
        node->next = _dummyfront->next;
        _dummyfront->next = node;
        node->last = _dummyfront;
        node->next->last = node;
    }
    void removeNode(Dlist* node){
        node->last->next = node->next;
        node->next->last = node->last;
    }
    void moveToHead(Dlist* node){
        removeNode(node);
        addToHead(node);
    }
    
private:
    int _capacity;
    int _size;
    unordered_map<int, Dlist*> _cache;
    Dlist* _dummyfront;
    Dlist* _dummyend;
};

int main(int argc, const char * argv[]) {
    LRUCache lru(3);
    lru.put(1, 0);
    lru.put(2, -2);
    lru.get(1);
    lru.put(3, -3);
    lru.put(4, -4);
    cout << lru.get(2) << endl;
    cout << lru.get(1) << endl;
    
    return 0;
}
