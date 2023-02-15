#include <exception>
#define NOT_DELETED 0
#define DELETED 1
typedef unsigned long int size_t;
size_t primes[18] = {337, 3371, 33713, 337153,
                                     3371539, 33715393, 337153933, 3371539337, 
                                     33715393373, 337153933733, 3371539337339, 33715393373399,
                                     33715393373419ULL, 337153933734203ULL, 3371539337342107ULL, 33715393373421077ULL,
                                     337153933734210809ULL, 3371539337342108117ULL};

class uninitializedArraySentinel{
    private:
        size_t* array1;
        size_t* array2;
        size_t fillCounter;
    public:
        uninitializedArraySentinel(size_t size);
        bool initialized(size_t index);
        void initialize(size_t index);
        ~uninitializedArraySentinel();
};

template <typename keyType, typename valueType>
class hashmap{
    public:
        keyType* keyArray;
        valueType* valArray;
        uninitializedArraySentinel* sentinel;
        bool* deleted;//checking deleted[i] flag makes sense only if isInitialized(i) = true
        size_t size;
        hashmap(size_t arraySize);
        void setItem(size_t index, keyType key, valueType value);
        void deleteItem(size_t index);
        bool isInitialized(size_t index);
        bool isDeleted(size_t index);
        keyType getKey(size_t index);
        valueType getValue(size_t index);
        ~hashmap();
};

template <typename keyType, typename valueType>
class dictionary{
    private:
        hashmap<keyType, valueType>* array;
        hashmap<keyType, valueType>* previousArray;
        size_t fillCounter;
        size_t rewriteCount;
        size_t* iterator;
        size_t (*hashFunction1)(keyType);
        size_t (*hashFunction2)(keyType);
        void resize();
        size_t searchForIndexToInsert(keyType key);
    public:
        dictionary(size_t (*hashF1)(keyType), size_t (*hashF2)(keyType));
        void setItem(keyType key, valueType val);
        valueType getItem(keyType key);
        void deleteItem(keyType key);
        ~dictionary();
};

template <typename keyType, typename valueType>
dictionary<keyType, valueType>::dictionary(size_t (*hashF1)(keyType), size_t (*hashF2)(keyType)){
    fillCounter = 0;
    rewriteCount = 0;
    iterator = &primes[0];
    array = new hashmap<keyType, valueType>(*iterator);
    previousArray = new hashmap<keyType, valueType>(1);//empty hashmap(size 1)
    hashFunction1 = hashF1;
    hashFunction2 = hashF2;
};

template <typename keyType, typename valueType>
void dictionary<keyType, valueType>::resize(){//a private method called when array fill ratio is 10%
    
    delete previousArray;
    previousArray = array;

    array = new hashmap<keyType, valueType>(*(++iterator));
    rewriteCount = 0;
}

template <typename keyType, typename valueType>
size_t dictionary<keyType, valueType>::searchForIndexToInsert(keyType key){//a private method that searches in dictonary(without rewriting)
    size_t hash1 = hashFunction1(key) % array->size;
    size_t hash2 = hashFunction2(key);
    size_t index = hash1;
    while(array->isInitialized(index) && !array->isDeleted(index))
        index = (index + hash2) % array->size;
    return index;
}

template <typename keyType, typename valueType>
void dictionary<keyType, valueType>::setItem(keyType key, valueType val){
    int rewriteOld = rewriteCount;
    for(; rewriteCount < rewriteOld + 20 && rewriteCount < previousArray->size; rewriteCount++){
        if(previousArray->isInitialized(rewriteCount)){//scans positionaly old structure and rewrites found elements to new array
            array->setItem(searchForIndexToInsert(previousArray->getKey(rewriteCount)), previousArray->getKey(rewriteCount), previousArray->getValue(rewriteCount));
        }
    }

    size_t index = searchForIndexToInsert(key);
    array->setItem(index, key, val);
    fillCounter++;
    
    if(array->size/10 < fillCounter)
        resize();
}

template <typename keyType, typename valueType>
valueType dictionary<keyType, valueType>::getItem(keyType key){
    int rewriteOld = rewriteCount;
    for(; rewriteCount < rewriteOld + 20 && rewriteCount < previousArray->size; rewriteCount++){
        if(previousArray->isInitialized(rewriteCount)){
            array->setItem(searchForIndexToInsert(previousArray->getKey(rewriteCount)), previousArray->getKey(rewriteCount), previousArray->getValue(rewriteCount));
        }
    }

    size_t hash1 = hashFunction1(key) % array->size;
    size_t hash2 = hashFunction2(key);
    size_t index = hash1;
    while(array->isInitialized(index) && (array->getKey(index) != key || array->isDeleted(index)))
        index = (index + hash2) % array->size;

    
    
    if(array->getKey(index) != key){//searches in second array if not found in first
        size_t hash1 = hashFunction1(key) % previousArray->size;
        size_t hash2 = hashFunction2(key);
        size_t index = hash1;
        while(previousArray->isInitialized(index) && (previousArray->getKey(index) != key || previousArray->isDeleted(index)))
            index = (index + hash2) % previousArray->size;
        if(previousArray->getKey(index) != key)
            throw std::out_of_range("index Error: element not found in array");
        return previousArray->getValue(index);
    }

    return array->getValue(index);
}

template <typename keyType, typename valueType>
void dictionary<keyType, valueType>::deleteItem(keyType key){
    int rewriteOld = rewriteCount;
    for(; rewriteCount < rewriteOld + 20 && rewriteCount < previousArray->size; rewriteCount++){
        if(previousArray->isInitialized(rewriteCount)){
            array->setItem(searchForIndexToInsert(previousArray->getKey(rewriteCount)), previousArray->getKey(rewriteCount), previousArray->getValue(rewriteCount));
        }
    }

    size_t hash1 = hashFunction1(key) % array->size;
    size_t hash2 = hashFunction2(key);
    size_t index = hash1;
    while(array->isInitialized(index) && array->getKey(index) != key)
        index = (index + hash2) % array->size;
    
    if(array->getKey(index) != key){
        size_t hash1 = hashFunction1(key) % previousArray->size;
        size_t hash2 = hashFunction2(key);
        size_t index = hash1;
        while(previousArray->isInitialized(index) && previousArray->getKey(index) != key)
            index = (index + hash2) % previousArray->size;
        if(previousArray->getKey(index) != key)
            throw std::out_of_range("index Error: element not found in array");
        previousArray->deleteItem(index);
    }
    else{
        array->deleteItem(index);
    }
        
    fillCounter--;
    
}

template <typename keyType, typename valueType>
dictionary<keyType, valueType>::~dictionary(){
    delete array;
    delete previousArray;
}

template <typename keyType, typename valueType>
hashmap<keyType, valueType>::hashmap(size_t arraySize){
    size = arraySize;
    keyArray = new keyType[size];
    valArray = new valueType[size];
    sentinel = new uninitializedArraySentinel(size);
    deleted = new bool[size];
}

template <typename keyType, typename valueType>
void hashmap<keyType, valueType>::setItem(size_t index, keyType key, valueType value){
    sentinel->initialize(index);
    keyArray[index] = key;
    valArray[index] = value;
    deleted[index] = NOT_DELETED;
}

template <typename keyType, typename valueType>
void hashmap<keyType, valueType>::deleteItem(size_t index){
    deleted[index] = DELETED;
}

template <typename keyType, typename valueType>
keyType hashmap<keyType, valueType>::getKey(size_t index){
    return keyArray[index];
}

template <typename keyType, typename valueType>
valueType hashmap<keyType, valueType>::getValue(size_t index){
    return valArray[index];
}

template <typename keyType, typename valueType>
bool hashmap<keyType, valueType>::isInitialized(size_t index){
    return sentinel->initialized(index);
}

template <typename keyType, typename valueType>
bool hashmap<keyType, valueType>::isDeleted(size_t index){
    return deleted[index] == DELETED;
}

template <typename keyType, typename valueType>
hashmap<keyType, valueType>::~hashmap(){
    delete[] deleted;
    delete[] keyArray;
    delete[] valArray;
    delete sentinel;
}

uninitializedArraySentinel::uninitializedArraySentinel(size_t size){
    fillCounter = 0;
    array1 = new size_t[size];
    array2 = new size_t[size];
}

bool uninitializedArraySentinel::initialized(size_t index){
    return array1[index] < fillCounter && array2[array1[index]] == index;
}

void uninitializedArraySentinel::initialize(size_t index){
    if(!initialized(index)){
        array1[index] = fillCounter;
        array2[fillCounter] = index;
        fillCounter++;
    }

}

uninitializedArraySentinel::~uninitializedArraySentinel(){
    delete[] array1;
    delete[] array2;
}