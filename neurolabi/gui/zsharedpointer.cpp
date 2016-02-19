
template<typename T>
ZSharedPointer<T>::ZSharedPointer(T *p) : ztr1::shared_ptr<T>(p) {}

template<typename T>
ZSharedPointer<T>::ZSharedPointer(const ZSharedPointer<T> &p) : ztr1::shared_ptr<T>(p) {}

template<typename T>
ZUniquePointer<T>::ZUniquePointer(T *p) : ztr1::auto_ptr<T>(p) {}

template<typename T>
ZUniquePointer<T>::ZUniquePointer(const ZUniquePointer<T> &p) : ztr1::auto_ptr<T>(p) {}
