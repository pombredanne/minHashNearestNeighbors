#include "typeDefintions.h"
#ifndef HASH_H
#define HASH_H
class Hash {
  private:
    const double A = sqrt(2) - 1;
    
    size_t _size_tHashSimple(size_t key, size_t aModulo) {
          // std::hash<size_t> hash;
          // return hash(key);
          key = ~key + (key << 15);
          key = key ^ (key >> 12);
          key = key + (key << 2);
          key = key ^ (key >> 4);
          key = key * 2057;
          key = key ^ (key >> 16);
          return key % aModulo;
    };
  public:      
    size_t hash(size_t pKey, size_t pModulo, size_t pSeed) {
        return _size_tHashSimple(pKey * pSeed * A, pModulo);
    }
};
#endif // HASH_H