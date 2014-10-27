
template<typename T>
ZSharedPointer<T>::ZSharedPointer(T *p) : ztr1::shared_ptr<T>(p) {}

template<typename T>
ZSharedPointer<T>::ZSharedPointer(const ZSharedPointer<T> &p) : ztr1::shared_ptr<T>(p) {}
