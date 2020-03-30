#include "SimpleLRU.h"

namespace Afina {
namespace Backend {

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Put(const std::string &key, const std::string &value) { 
    std::size_t record_size = key.size() + value.size();
    if (_lru_index.count(key)) {
        auto& it = _lru_index.find(key)->second;
        auto item_ptr = it.get().prev->next;
        it.get().value = value;
        // отвязали
        item_ptr->prev->next = item_ptr->next;
        item_ptr->next->prev = item_ptr->prev;
        // привязали новые объекты к голове
        item_ptr->next = _lru_head;
        item_ptr->prev = _lru_head->prev;
        // привязали голову к новым объектам
        item_ptr->next->prev = item_ptr;
        item_ptr->prev->next = item_ptr;
        
        return true;
    }
    if (record_size > _max_size) {
        return false;
    }

    while (_cur_size + record_size > _max_size) {
        auto key2remove = _lru_head->key;
        _cur_size -= _lru_head->key.size() + _lru_head->value.size();
        if (_lru_index.size() == 1) {
            _lru_head->next = nullptr;
            _lru_head->prev = nullptr;
            _lru_head = nullptr;
        } else {
            auto node2remove = _lru_head;
            _lru_head->next->prev = _lru_head->prev;
            _lru_head->prev->next = _lru_head->next;
            _lru_head = node2remove->next;
            node2remove->next = nullptr;
            node2remove->prev = nullptr;
        }
        _lru_index.erase(key2remove);
    }

    auto new_node = std::make_shared<lru_node>();
    new_node->key = key;
    new_node->value = value;
    if (_lru_head == nullptr) {
        _lru_head = new_node;
        _lru_head->next = _lru_head;
        _lru_head->prev = _lru_head;
    } else {
        new_node->next = _lru_head;
        new_node->prev = _lru_head->prev;
        _lru_head->prev->next = new_node;
        _lru_head->prev = new_node;
    }
    _cur_size += record_size;
    _lru_index.insert({new_node->key, *new_node});

    return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::PutIfAbsent(const std::string &key, const std::string &value) {
    if (_lru_index.count(key)) {
        return false;
    }
    std::size_t record_size = key.size() + value.size();
    if (record_size > _max_size) {
        return false;
    }

    while (_cur_size + record_size > _max_size) {
        auto key2remove = _lru_head->key;
        _cur_size -= _lru_head->key.size() + _lru_head->value.size();
        if (_lru_index.size() == 1) {
            _lru_head->next = nullptr;
            _lru_head->prev = nullptr;
            _lru_head = nullptr;
        } else {
            auto node2remove = _lru_head;
            _lru_head->next->prev = _lru_head->prev;
            _lru_head->prev->next = _lru_head->next;
            _lru_head = node2remove->next;
            node2remove->next = nullptr;
            node2remove->prev = nullptr;
        }
        _lru_index.erase(key2remove);
    }

    auto new_node = std::make_shared<lru_node>();
    new_node->key = key;
    new_node->value = value;
    if (_lru_head == nullptr) {
        _lru_head = new_node;
        _lru_head->next = _lru_head;
        _lru_head->prev = _lru_head;
    } else {
        new_node->next = _lru_head;
        new_node->prev = _lru_head->prev;
        _lru_head->prev->next = new_node;
        _lru_head->prev = new_node;
    }
    _cur_size += record_size;
    _lru_index.insert({new_node->key, *new_node});


    return true; 
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Set(const std::string &key, const std::string &value) { 
    if (_lru_index.count(key)) {
        _lru_index.find(key)->second.get().value = value;
        auto& it = _lru_index.find(key)->second;
        auto item_ptr = it.get().prev->next;
        // отвязали
        item_ptr->prev->next = item_ptr->next;
        item_ptr->next->prev = item_ptr->prev;
        // привязали новые объекты к голове
        item_ptr->next = _lru_head;
        item_ptr->prev = _lru_head->prev;
        // привязали голову к новым объектам
        item_ptr->next->prev = item_ptr;
        item_ptr->prev->next = item_ptr;
        // поменяли голову
        // _lru_head = item_ptr;

        return true;
    }

    return false;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Delete(const std::string &key) { 
    if (!_lru_index.count(key)) {
        return false;
    }
    auto item_ptr = _lru_index.find(key)->second.get().prev->next;
    std::size_t record = item_ptr->key.size() + item_ptr->value.size();
    if (_lru_index.size() == 1) {
        item_ptr->next = nullptr;
        item_ptr->prev = nullptr;
        _lru_head = nullptr;
    } else {
        if (_lru_head == item_ptr) {
            _lru_head = item_ptr->next;
        }
        item_ptr->prev->next = item_ptr->next;
        item_ptr->next->prev = item_ptr->prev;
        item_ptr->next = nullptr;
        item_ptr->prev = nullptr;
    }
    _lru_index.erase(item_ptr->key);

    _cur_size -= record;
    return true;
}

// See MapBasedGlobalLockImpl.h
bool SimpleLRU::Get(const std::string &key, std::string &value) { 
        if (_lru_index.count(key)) {
            value = _lru_index.find(key)->second.get().value;
            auto& it = _lru_index.find(key)->second;
            auto item_ptr = it.get().prev->next;
            // отвязали
            item_ptr->prev->next = item_ptr->next;
            item_ptr->next->prev = item_ptr->prev;
            // привязали новые объекты к голове
            item_ptr->next = _lru_head;
            item_ptr->prev = _lru_head->prev;
            // привязали голову к новым объектам
            item_ptr->next->prev = item_ptr;
            item_ptr->prev->next = item_ptr;
            // поменяли голову
            // _lru_head = item_ptr;

            return true;
        }

        return false; 
    }

} // namespace Backend
} // namespace Afina
